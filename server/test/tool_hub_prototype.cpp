#include "tool_hubb_prototype.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

#include <cerrno>
#include <cstring>
#include <iostream>

using json = nlohmann::json;

static bool setCloexec(int fd) {
    int flags = fcntl(fd, F_GETFD);
    if (flags < 0) return false;
    return (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == 0);
}

ToolHubLite::ToolHubLite(std::string hubSockPath) : hubSockPath_(std::move(hubSockPath)) {}

ToolHubLite::~ToolHubLite() {
    shutdown();
}

bool ToolHubLite::init() {
    return setup_listen_socket();
}

void ToolHubLite::shutdown() {
    for (auto& [name, c] : conns_) {
        close_conn(c);
    }
    if (listenFd_ >= 0) {
        ::close(listenFd_);
        listenFd_ = -1;
    }
    ::unlink(hubSockPath_.c_str());
}

void ToolHubLite::register_tool(const std::string& toolName, const std::string& exePath) {
    conns_[toolName].exePath = exePath;
}

bool ToolHubLite::is_connected(const std::string& toolName) const {
    auto it = conns_.find(toolName);
    if (it == conns_.end())
        return false;
    return it->second.fd >= 0;
}

bool ToolHubLite::setup_listen_socket() {
    ::unlink(hubSockPath_.c_str());

    listenFd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        std::cerr << "hub socket() failed: " << strerror(errno) << "\n";
        return false;
    }
    setCloexec(listenFd_);

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", hubSockPath_.c_str());

    if (::bind(listenFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        std::cerr << "hub bind() failed: " << strerror(errno) << "\n";
        return false;
    }

    ::chmod(hubSockPath_.c_str(), 0660);

    if (::listen(listenFd_, 16) != 0) {
        std::cerr << "hub listen() failed: " << strerror(errno) << "\n";
        return false;
    }
    return true;
}

void ToolHubLite::close_conn(Conn& c) {
    if (c.fd >= 0) {
        ::close(c.fd);
        c.fd = -1;
        c.readBuf.clear();
    }
    if (c.pid > 0) {
        // мягко
        ::kill(c.pid, SIGTERM);
        int st = 0;
        ::waitpid(c.pid, &st, 0);
        c.pid = -1;
    }
}

bool ToolHubLite::spawn_tool_process(const std::string& toolName) {
    auto it = conns_.find(toolName);
    if (it == conns_.end() || it->second.exePath.empty()) {
        std::cerr << "ToolHubLite: unknown tool or no exePath for '" << toolName << "'\n";
        return false;
    }

    Conn& c = it->second;

    // если был старый pid/fd — закроем
    close_conn(c);

    pid_t pid = ::fork();
    if (pid < 0) {
        std::cerr << "fork() failed for " << toolName << ": " << strerror(errno) << "\n";
        return false;
    }

    if (pid == 0) {
        // child: запускаем утилиту: <exe> --hub <sock> --tool <name>
        execl(c.exePath.c_str(),
              c.exePath.c_str(),
              "--hub", hubSockPath_.c_str(),
              "--tool", toolName.c_str(),
              (char*)nullptr);
        std::cerr << "execl() failed for " << toolName << ": " << strerror(errno) << "\n";
        _exit(127);
    }

    c.pid = pid;
    return true;
}

bool ToolHubLite::send_json_line(int fd, const json& j) {
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

std::optional<json> ToolHubLite::recv_json_line(Conn& c, int timeoutMs) {
    // читаем до '\n' с poll-таймаутом
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

void ToolHubLite::accept_and_attach_one(int timeoutMs) {
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
            if (rpr <= 0) return std::nullopt;
            char tmp[1024];
            ssize_t n = ::recv(fd, tmp, sizeof(tmp), 0);
            if (n <= 0) return std::nullopt;
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
        ::close(fd); return;
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

bool ToolHubLite::wait_tool_hello(const std::string& toolName, int timeoutMs) {
    // ждём, пока toolName появится как connected
    const int step = 100; // мс
    int left = timeoutMs;

    while (left > 0) {
        if (is_connected(toolName))
            return true;
        accept_and_attach_one(step);
        left -= step;
    }
    return is_connected(toolName);
}

bool ToolHubLite::start_tool(const std::string& toolName, int startupTimeoutMs, bool doPing) {
    if (is_connected(toolName))
        return true;

    if (!spawn_tool_process(toolName))
        return false;
    if (!wait_tool_hello(toolName, startupTimeoutMs))
        return false;

    if (!doPing)
        return true;

    // optional ping to ensure tool is ready
    auto resp = call(toolName, "ping", json::object(), startupTimeoutMs);
    return resp.has_value() && (*resp).value("ok", false) == true;
}

std::optional<json> ToolHubLite::call(const std::string& toolName,
                                     const std::string& cmd,
                                     const json& args,
                                     int timeoutMs) {
    auto it = conns_.find(toolName);
    if (it == conns_.end())
        return std::nullopt;
    Conn& c = it->second;
    if (c.fd < 0)
        return std::nullopt;

    uint64_t id = nextId_++; // однопоточно
    json req = {{"type","req"},{"id",id},{"cmd",cmd},{"args",args}};

    if (!send_json_line(c.fd, req))
        return std::nullopt;

    auto respOpt = recv_json_line(c, timeoutMs);
    if (!respOpt)
        return std::nullopt;

    // минимальная проверка
    if ((*respOpt).value("type","") != "resp")
        return std::nullopt;
    if ((*respOpt).value("id", 0ull) != id)
        return std::nullopt;

    return respOpt;
}
