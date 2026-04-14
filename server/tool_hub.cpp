#include "tool_hub.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <filesystem>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>

using json = nlohmann::json;

static bool setCloexec(int fd) {
    int flags = fcntl(fd, F_GETFD);
    if (flags < 0) return false;
    return (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == 0);
}

std::vector<std::string> getArgs(const std::string& exePath, const std::string& arg) {
    std::vector<std::string> tokens;
    tokens.push_back(exePath);

    std::istringstream iss(arg);
    std::string t;
    while (iss >> t)
        tokens.push_back(t);
    
    return tokens;
}

static std::vector<char*> makeArgv(std::vector<std::string>& argsStr) {
    std::vector<char*> argv;
    argv.reserve(argsStr.size() + 1);
    for (auto& s : argsStr)
        argv.push_back(s.data());
    argv.push_back(nullptr);
    return argv;
}

ToolHub& ToolHub::inst() {
    static ToolHub inst;
    return inst;
}

ToolHub::ToolHub() {

}

ToolHub::~ToolHub() {
    shutdown();
}

bool ToolHub::init() {
    return setup_listen_socket();
}

void ToolHub::shutdown() {
    for (auto& [name, c] : conns_) {
        close_conn(c);
    }
    if (listenFd_ >= 0) {
        ::close(listenFd_);
        listenFd_ = -1;
    }
    ::unlink(hubSockPath_.c_str());
}

bool ToolHub::setup_listen_socket() {
    ::unlink(hubSockPath_.c_str());

    listenFd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        printf("ToolHub::setup_listen_socket(): socket() failed: %s\n", strerror(errno));
        return false;
    }
    setCloexec(listenFd_);

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", hubSockPath_.c_str());

    if (::bind(listenFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        printf("ToolHub::setup_listen_socket(): bind() failed: %s\n", strerror(errno));
        return false;
    }

    ::chmod(hubSockPath_.c_str(), 0660);

    if (::listen(listenFd_, 16) != 0) {
        printf("ToolHub::setup_listen_socket(): listen() failed: %s\n", strerror(errno));
        return false;
    }
    return true;
}

bool ToolHub::start_tool(const std::string& toolName, const std::string& exePath, const std::string& arg, const std::string& workingDir, int startupTimeoutMs) {
    if (!register_tool(toolName, exePath, arg, workingDir)) {
        printf("[ToolHub] register_tool error!\n");
        return false;
    }
    printf("[ToolHub] register_tool success!\n");

    if (!spawn_tool_process(toolName)) {
        printf("[ToolHub] spawn_tool_process error!\n");
        return false;
    }
    printf("[ToolHub] spawn_tool_process success!\n");

    if (startupTimeoutMs == -1)
        return true;

    if (!wait_tool_hello(toolName, startupTimeoutMs)) {
        printf("[ToolHub] wait_tool_hello error!\n");
        return false;
    }
    printf("[ToolHub] wait_tool_hello success!\n");

    return true;
}

bool ToolHub::spawn_tool_process(const std::string& toolName) {
    auto it = conns_.find(toolName);
    if (it == conns_.end() || it->second.exePath.empty()) {
        printf("ToolHub::spawn_tool_process(): unknown tool or no exePath for %s\n", toolName.c_str());
        return false;
    }

    Conn& c = it->second;

    close_conn(c);
    auto argv = makeArgv(c.args);

    pid_t pid = ::fork();
    if (pid < 0) {
        printf("ToolHub::spawn_tool_process(): fork() failed for %s: %s\n", toolName.c_str(), strerror(errno));
        return false;
    }

    if (pid == 0) {
        printf("ToolHub::workingDir: %s\n", c.workingDir.c_str());
        if (!c.workingDir.empty())
            std::filesystem::current_path(std::filesystem::path(c.workingDir));

        ::execv(c.exePath.c_str(), argv.data());
        printf("ToolHub::spawn_tool_process(): execl() failed for %s: %s\n", toolName.c_str(), strerror(errno));
        _exit(127);
    }

    c.pid = pid;
    return true;
}

bool ToolHub::register_tool(const std::string& toolName, const std::string& exePath, const std::string& arg, const std::string& workingDir) {
    if (conns_.find(toolName) != conns_.end()) {
        fprintf(stderr, "[Error][ToolHub] register_tool() tool is already registered: %s\n", toolName.c_str());
        return false;
    }
    conns_[toolName].exePath = exePath;
    conns_[toolName].workingDir = workingDir;
    conns_[toolName].args = getArgs(exePath, arg);
    return true;
}

void ToolHub::close_conn(Conn& c) {
    if (c.fd >= 0) {
        ::close(c.fd);
        c.fd = -1;
        c.readBuf.clear();
    }
    if (c.pid > 0) {
        ::kill(c.pid, SIGTERM);
        int st = 0;
        ::waitpid(c.pid, &st, 0);
        c.pid = -1;
    }
}

bool ToolHub::wait_tool_hello(const std::string& toolName, int timeoutMs) {
    const int step = 100;
    int left = timeoutMs;

    while (left > 0) {
        if (is_connected(toolName))
            return true;
        accept_and_attach_one(step);
        left -= step;
    }
    return is_connected(toolName);
}

bool ToolHub::is_connected(const std::string& toolName) const {
    auto it = conns_.find(toolName);
    if (it == conns_.end())
        return false;
    return it->second.fd >= 0;
}

void ToolHub::accept_and_attach_one(int timeoutMs) {
    pollfd pfd{};
    pfd.fd = listenFd_;
    pfd.events = POLLIN;

    int pr = ::poll(&pfd, 1, timeoutMs);
    if (pr <= 0)
        return;

    int fd = ::accept(listenFd_, nullptr, nullptr);
    if (fd < 0)
        return;
    setCloexec(fd);

    // ждём hello на этом fd
    std::string buf;
    buf.reserve(2048);

    auto recvLineOnce = [&](int ms) -> std::optional<std::string> {
        while (true) {
            auto pos = buf.find('\n');
            if (pos != std::string::npos) {
                std::string line = buf.substr(0, pos);
                buf.erase(0, pos + 1);
                return line;
            }
            pollfd rp{};
            rp.fd = fd;
            rp.events = POLLIN;
            int rpr = ::poll(&rp, 1, ms);
            if (rpr <= 0)
                return std::nullopt;
            char tmp[1024];
            ssize_t n = ::recv(fd, tmp, sizeof(tmp), 0);
            if (n <= 0)
                return std::nullopt;
            buf.append(tmp, tmp + n);
        }
    };

    auto lineOpt = recvLineOnce(500);
    if (!lineOpt) {
        ::close(fd);
        return;
    }

    json hello;
    try {
        hello = json::parse(*lineOpt);
    }
    catch (...) {
        ::close(fd);
        return;
    }

    if (hello.value("type", "") != "hello") {
        ::close(fd);
        return;
    }
    std::string tool = hello.value("tool", "");

    auto it = conns_.find(tool);
    if (it == conns_.end()) {
        ::close(fd);
        return;
    }

    // прикрепляем fd к нужной утилите
    Conn& c = it->second;
    if (c.fd >= 0)
        ::close(c.fd);
    c.fd = fd;
    c.readBuf.clear();
}

std::optional<uint64_t> ToolHub::send(const std::string& toolName, const std::string& cmd, const json& args) {
    auto it = conns_.find(toolName);
    if (it == conns_.end()) {
        printf("[ToolHub] send(): toolName error\n");
        return std::nullopt;
    }

    Conn& c = it->second;
    if (c.fd < 0) {
        printf("[ToolHub] send(): fd error\n");
        return std::nullopt;
    }

    uint64_t id = nextId_++;
    json req = {
        {"id", id},
        {"cmd", cmd},
        {"args", args}
    };

    if (!send_json_line(c.fd, req)) {
        printf("[ToolHub] send(): send_json_line error: %s\n", strerror(errno));
        return std::nullopt;
    }

    return id;
}

std::optional<json> ToolHub::wait(const std::string& toolName, uint64_t id, int timeoutMs) {
    auto it = conns_.find(toolName);
    if (it == conns_.end()) {
        printf("[ToolHub] wait(): toolName error\n");
        return std::nullopt;
    }

    Conn& c = it->second;
    if (c.fd < 0) {
        printf("[ToolHub] wait(): fd error\n");
        return std::nullopt;
    }

    auto respOpt = recv_json_line(c, timeoutMs);
    if (!respOpt) {
        printf("[ToolHub] wait(): recv_json_line error: %s\n", strerror(errno));
        return std::nullopt;
    }

    if ((*respOpt).value("id", 0ull) != id) {
        printf("[ToolHub] wait(): id error\n");
        return std::nullopt;
    }

    if (!(*respOpt).contains("result") || !(*respOpt)["result"].is_boolean()) {
        printf("[ToolHub] wait(): result field error\n");
        return std::nullopt;
    }

    return respOpt;
}

std::optional<json> ToolHub::call(const std::string& toolName, const std::string& cmd, const json& args, int timeoutMs) {
    auto idOpt = send(toolName, cmd, args);
    if (!idOpt)
        return std::nullopt;

    return wait(toolName, *idOpt, timeoutMs);
}

// std::optional<json> ToolHub::call(const std::string& toolName, const std::string& cmd, const json& args, int timeoutMs) {
//     auto it = conns_.find(toolName);
//     if (it == conns_.end()) {
//         printf("[ToolHub] call(): toolName error\n");
//         return std::nullopt;
//     }

//     Conn& c = it->second;
//     if (c.fd < 0) {
//         printf("[ToolHub] call(): fd error\n");
//         return std::nullopt;
//     }

//     uint64_t id = nextId_++;
//     json req = {{"id", id},{"cmd", cmd},{"args", args}};

//     if (!send_json_line(c.fd, req)) {
//         printf("[ToolHub] call(): send_json_line error: %s\n", strerror(errno));
//         return std::nullopt;
//     }

//     auto respOpt = recv_json_line(c, timeoutMs);
//     if (!respOpt) {
//         printf("[ToolHub] call(): recv_json_line error: %s\n", strerror(errno));
//         return std::nullopt;
//     }

//     if ((*respOpt).value("id", 0ull) != id) {
//         printf("[ToolHub] call(): id error\n");
//         return std::nullopt;
//     }
//     if (!(*respOpt).contains("result") || !(*respOpt)["result"].is_boolean()) {
//         printf("[ToolHub] call(): id error\n");
//         return std::nullopt;
//     }

//     return respOpt;
// }

bool ToolHub::send_json_line(int fd, const json& j) {
    std::string s = j.dump();
    s.push_back('\n');

    const char* p = s.data();
    size_t left = s.size();
    while (left > 0) {
        ssize_t n = ::send(fd, p, left, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return false;
        }
        p += n;
        left -= (size_t)n;
    }
    return true;
}

std::optional<json> ToolHub::recv_json_line(Conn& c, int timeoutMs) {
    while (true) {
        auto pos = c.readBuf.find('\n');
        if (pos != std::string::npos) {
            std::string line = c.readBuf.substr(0, pos);
            c.readBuf.erase(0, pos + 1);
            if (line.empty())
                continue;

            try {
                return json::parse(line);
            }
            catch (...) {
                return std::nullopt;
            }
        }

        pollfd pfd{};
        pfd.fd = c.fd;
        pfd.events = POLLIN;

        int pr = ::poll(&pfd, 1, timeoutMs);
        if (pr == 0)
            return std::nullopt; // timeout
        if (pr < 0) {
            if (errno == EINTR)
                continue;
            return std::nullopt;
        }

        char tmp[1024];
        ssize_t n = ::recv(c.fd, tmp, sizeof(tmp), 0);
        if (n == 0)
            return std::nullopt; // disconnected
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return std::nullopt;
        }
        c.readBuf.append(tmp, tmp + n);
    }
}
