#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "tool_hub.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <algorithm>
#include <chrono>
#include <csignal>
#include <sstream>
#include <thread>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <utility>
#include <vector>

using json = nlohmann::json;

using HubClock = std::chrono::steady_clock;

static int remaining_ms(HubClock::time_point deadline) {
    const auto now = HubClock::now();
    if (now >= deadline) return 0;
    return std::max(1, static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count()));
}

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
    if (listenFd_ >= 0) shutdown();
}

bool ToolHub::init() {
    if (listenFd_ >= 0) return true;
    return setup_listen_socket();
}

void ToolHub::shutdown() {
    for (auto& [name, c] : conns_) {
        stop_tool(name);
        close_socket(c);
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
    if (!setCloexec(listenFd_)) {
        ::close(listenFd_);
        listenFd_ = -1;
        return false;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", hubSockPath_.c_str());

    if (::bind(listenFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        printf("ToolHub::setup_listen_socket(): bind() failed: %s\n", strerror(errno));
        ::close(listenFd_);
        listenFd_ = -1;
        return false;
    }

    ::chmod(hubSockPath_.c_str(), 0660);

    if (::listen(listenFd_, 16) != 0) {
        printf("ToolHub::setup_listen_socket(): listen() failed: %s\n", strerror(errno));
        ::close(listenFd_);
        listenFd_ = -1;
        ::unlink(hubSockPath_.c_str());
        return false;
    }
    return true;
}

bool ToolHub::start_tool(const std::string& toolName, const std::string& exePath, const std::string& arg, const std::string& workingDir, int startupTimeoutMs, std::string* error) {
    std::string failure;
    auto fail = [&](const std::string& message) {
        if (error) *error = message;
        fprintf(stderr, "[ToolHub] start %s: %s\n", toolName.c_str(), message.c_str());
        return false;
    };
    if (error) error->clear();
    if (toolName.empty() || exePath.empty() || startupTimeoutMs < -1)
        return fail("Invalid tool name, executable or startup timeout");
    if (listenFd_ < 0)
        return fail("ToolHub is not initialized");

    auto it = conns_.find(toolName);
    if (it != conns_.end()) {
        const auto state = poll_process(it->second);
        if (state == ProcessState::Error)
            return fail(it->second.processError);
        if (state == ProcessState::Running)
            return fail("Tool is already running; use restart");
    }

    auto args = getArgs(exePath, arg);
    Conn& c = conns_[toolName];
    c.exePath = exePath;
    c.workingDir = workingDir;
    c.args = std::move(args);
    c.startupTimeoutMs = startupTimeoutMs;
    if (!launch_tool(toolName, c, failure))
        return fail(failure);
    return true;
}

ToolHub::StopResult ToolHub::stop_tool(const std::string& toolName, int stopTimeoutMs, int killTimeoutMs) {
    auto it = conns_.find(toolName);
    if (it == conns_.end()) {
        StopResult result;
        result.error = "Unknown tool; no process was started by this server";
        return result;
    }
    auto result = stop_process(it->second, stopTimeoutMs, killTimeoutMs);
    printf("[ToolHub] stop %s: success=%d forced=%d already_stopped=%d\n",
        toolName.c_str(), result.success, result.forced, result.alreadyStopped);
    if (!result.success)
        fprintf(stderr, "[ToolHub] stop %s: %s\n", toolName.c_str(), result.error.c_str());
    return result;
}

ToolHub::RestartResult ToolHub::restart_tool(const std::string& toolName) {
    RestartResult result;
    auto it = conns_.find(toolName);
    if (it == conns_.end()) {
        result.error = "Unknown tool; use start with launch parameters first";
        return result;
    }
    if (listenFd_ < 0) {
        result.error = "ToolHub is not initialized";
        return result;
    }
    result.stopAttempted = true;
    result.stop = stop_tool(toolName);
    if (!result.stop.success) {
        result.error = result.stop.error;
        return result;
    }
    result.success = launch_tool(toolName, it->second, result.error);
    return result;
}

bool ToolHub::check_tool_process(const std::string& toolName, std::string& error) {
    error.clear();
    auto it = conns_.find(toolName);
    if (it == conns_.end()) {
        error = "Unknown tool";
        return false;
    }
    Conn& c = it->second;
    const auto state = poll_process(c);
    if (state == ProcessState::Error) {
        error = c.processError;
    } else if (state == ProcessState::Running) {
        if (!c.stopping) return true;
        error = "Tool is stopping";
    } else if (c.exitStatus && WIFEXITED(*c.exitStatus)) {
        const int code = WEXITSTATUS(*c.exitStatus);
        if (code == 0) return true;
        error = "Tool exited with code " + std::to_string(code);
    } else if (c.exitStatus && WIFSIGNALED(*c.exitStatus)) {
        error = "Tool terminated by signal " + std::to_string(WTERMSIG(*c.exitStatus));
    } else {
        error = "Tool has no confirmed successful process state";
    }
    return false;
}

void ToolHub::close_socket(Conn& c) {
    if (c.fd >= 0)
        ::close(c.fd);
    c.fd = -1;
    c.readBuf.clear();
}

ToolHub::ProcessState ToolHub::poll_process(Conn& c) {
    if (c.pid <= 0)
        return ProcessState::Exited;
    if (!c.processError.empty())
        return ProcessState::Error;

    int status = 0;
    pid_t waited;
    do {
        waited = ::waitpid(c.pid, &status, WNOHANG);
    } while (waited < 0 && errno == EINTR);
    if (waited == 0)
        return ProcessState::Running;
    if (waited == c.pid) {
        c.exitStatus = status;
        c.pid = -1;
        c.stopping = false;
        close_socket(c);
        return ProcessState::Exited;
    }

    c.processError = std::string("Cannot confirm owned child state: ") + strerror(errno);
    fprintf(stderr, "[ToolHub] %s\n", c.processError.c_str());
    return ProcessState::Error;
}

ToolHub::ProcessState ToolHub::wait_process(Conn& c, int timeoutMs) {
    const auto deadline = HubClock::now() + std::chrono::milliseconds(timeoutMs);
    while (true) {
        const auto state = poll_process(c);
        if (state != ProcessState::Running)
            return state;
        const int left = remaining_ms(deadline);
        if (left == 0)
            return ProcessState::Timeout;
        std::this_thread::sleep_for(std::chrono::milliseconds(std::min(left, 20)));
    }
}

void ToolHub::describe_exit(const Conn& c, StopResult& result) {
    if (!c.exitStatus)
        return;
    if (WIFEXITED(*c.exitStatus))
        result.exitCode = WEXITSTATUS(*c.exitStatus);
    else if (WIFSIGNALED(*c.exitStatus))
        result.termSignal = WTERMSIG(*c.exitStatus);
}

ToolHub::StopResult ToolHub::stop_process(Conn& c, int stopTimeoutMs, int killTimeoutMs) {
    StopResult result;
    if (stopTimeoutMs < 0 || killTimeoutMs < 0) {
        result.error = "Stop timeouts must be non-negative";
        return result;
    }
    auto state = poll_process(c);
    if (state == ProcessState::Error) {
        result.error = c.processError;
        return result;
    }
    if (state == ProcessState::Exited) {
        close_socket(c);
        result.success = true;
        result.alreadyStopped = true;
        describe_exit(c, result);
        return result;
    }

    c.stopping = true;
    if (::kill(c.pid, SIGTERM) < 0 && errno != ESRCH) {
        result.error = std::string("SIGTERM failed: ") + strerror(errno);
        return result;
    }
    state = wait_process(c, stopTimeoutMs);
    if (state == ProcessState::Timeout) {
        if (::kill(c.pid, SIGKILL) == 0)
            result.forced = true;
        else if (errno != ESRCH) {
            result.error = std::string("SIGKILL failed: ") + strerror(errno);
            return result;
        }
        state = wait_process(c, killTimeoutMs);
    }
    if (state == ProcessState::Exited) {
        result.success = true;
        describe_exit(c, result);
    } else if (state == ProcessState::Error) {
        result.error = c.processError;
    } else {
        result.error = "Process exit was not confirmed after termination timeout";
    }
    return result;
}

bool ToolHub::launch_tool(const std::string& toolName, Conn& c, std::string& error) {
    if (!spawn_tool_process(c, error)
        || (c.startupTimeoutMs >= 0 && !wait_tool_hello(toolName, c.startupTimeoutMs, error))) {
        const auto stopped = stop_process(c, StopTimeoutMs, KillTimeoutMs);
        if (!stopped.success)
            error += "; startup cleanup failed: " + stopped.error;
        else if (stopped.forced)
            error += "; startup cleanup required SIGKILL";
        return false;
    }
    return true;
}

bool ToolHub::spawn_tool_process(Conn& c, std::string& error) {
    if (poll_process(c) != ProcessState::Exited) {
        error = c.processError.empty() ? "Previous process has not exited" : c.processError;
        return false;
    }
    close_socket(c);
    auto argv = makeArgv(c.args);
    const char* exe = c.exePath.c_str();
    const char* cwd = c.workingDir.empty() ? nullptr : c.workingDir.c_str();
    char* const* argvData = argv.data();
    int execPipe[2];
    if (::pipe(execPipe) < 0) {
        error = std::string("exec error pipe failed: ") + strerror(errno);
        return false;
    }
    if (!setCloexec(execPipe[0]) || !setCloexec(execPipe[1])
        || fcntl(execPipe[0], F_SETFL, O_NONBLOCK) < 0) {
        error = std::string("exec error pipe setup failed: ") + strerror(errno);
        ::close(execPipe[0]);
        ::close(execPipe[1]);
        return false;
    }
    const pid_t pid = ::fork();
    if (pid < 0) {
        error = std::string("fork failed: ") + strerror(errno);
        ::close(execPipe[0]);
        ::close(execPipe[1]);
        return false;
    }
    if (pid == 0) {
        ::close(execPipe[0]);
        if (!cwd || ::chdir(cwd) == 0)
            ::execv(exe, argvData);
        const int failure = errno;
        const char* data = reinterpret_cast<const char*>(&failure);
        size_t left = sizeof(failure);
        while (left > 0) {
            const ssize_t n = ::write(execPipe[1], data, left);
            if (n < 0 && errno == EINTR)
                continue;
            if (n <= 0)
                break;
            data += n;
            left -= static_cast<size_t>(n);
        }
        ::_exit(127);
    }

    ::close(execPipe[1]);
    c.pid = pid;
    c.exitStatus.reset();
    c.processError.clear();
    c.stopping = false;
    const auto deadline = HubClock::now() + std::chrono::milliseconds(2000);
    int execError = 0;
    size_t received = 0;
    bool executed = false;
    while (true) {
        const ssize_t n = ::read(execPipe[0], reinterpret_cast<char*>(&execError) + received,
            sizeof(execError) - received);
        if (n > 0) {
            received += static_cast<size_t>(n);
            if (received == sizeof(execError)) {
                error = std::string("chdir/exec failed: ") + strerror(execError);
                break;
            }
        } else if (n == 0) {
            executed = received == 0;
            if (!executed) error = "Incomplete exec error";
            break;
        } else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
            error = std::string("exec error pipe read failed: ") + strerror(errno);
            break;
        }
        const int left = remaining_ms(deadline);
        if (left == 0) {
            error = "Timed out waiting for exec";
            break;
        }
        pollfd pfd { execPipe[0], POLLIN, 0 };
        if (::poll(&pfd, 1, left) < 0 && errno != EINTR) {
            error = std::string("exec error pipe poll failed: ") + strerror(errno);
            break;
        }
    }
    ::close(execPipe[0]);
    if (executed) {
        const auto state = poll_process(c);
        if (state == ProcessState::Error) {
            error = c.processError;
            return false;
        }
        if (state == ProcessState::Exited
            && (!c.exitStatus || !WIFEXITED(*c.exitStatus) || WEXITSTATUS(*c.exitStatus) != 0)) {
            error = "Tool exited unsuccessfully immediately after exec";
            return false;
        }
    }
    return executed;
}

bool ToolHub::wait_tool_hello(const std::string& toolName, int timeoutMs, std::string& error) {
    Conn& c = conns_.at(toolName);
    const auto deadline = HubClock::now() + std::chrono::milliseconds(timeoutMs);
    while (true) {
        const auto state = poll_process(c);
        if (state != ProcessState::Running) {
            error = state == ProcessState::Error ? c.processError : "Tool exited before hello";
            return false;
        }
        if (is_connected(toolName))
            return true;
        const int left = remaining_ms(deadline);
        if (left == 0) {
            error = "Tool hello timeout";
            return false;
        }
        accept_and_attach_one(std::min(left, 500));
    }
}

bool ToolHub::is_connected(const std::string& toolName) const {
    auto it = conns_.find(toolName);
    return it != conns_.end() && it->second.fd >= 0
        && it->second.pid > 0 && !it->second.stopping && it->second.processError.empty();
}

void ToolHub::accept_and_attach_one(int timeoutMs) {
    const auto deadline = HubClock::now() + std::chrono::milliseconds(timeoutMs);
    pollfd pfd{};
    pfd.fd = listenFd_;
    pfd.events = POLLIN;

    int pr = ::poll(&pfd, 1, timeoutMs);
    if (pr <= 0)
        return;

    int fd = ::accept(listenFd_, nullptr, nullptr);
    if (fd < 0)
        return;
    if (!setCloexec(fd)) {
        ::close(fd);
        return;
    }

    // ждём hello на этом fd
    std::string buf;
    buf.reserve(2048);

    auto recvLineOnce = [&]() -> std::optional<std::string> {
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
            const int left = remaining_ms(deadline);
            if (left == 0) return std::nullopt;
            int rpr = ::poll(&rp, 1, left);
            if (rpr < 0 && errno == EINTR) continue;
            if (rpr <= 0)
                return std::nullopt;
            char tmp[1024];
            ssize_t n = ::recv(fd, tmp, sizeof(tmp), 0);
            if (n <= 0)
                return std::nullopt;
            buf.append(tmp, tmp + n);
            if (buf.size() > 65536) return std::nullopt;
        }
    };

    auto lineOpt = recvLineOnce();
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

    if (!hello.is_object() || !hello.contains("type") || !hello["type"].is_string()
        || hello["type"] != "hello" || !hello.contains("tool") || !hello["tool"].is_string()) {
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
    ucred peer {};
    socklen_t peerSize = sizeof(peer);
    if (poll_process(c) != ProcessState::Running || c.stopping
        || ::getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &peer, &peerSize) != 0
        || peerSize != sizeof(peer) || peer.pid != c.pid) {
        ::close(fd);
        return;
    }
    close_socket(c);
    c.fd = fd;
    c.readBuf = std::move(buf);
}

std::optional<uint64_t> ToolHub::send(const std::string& toolName, const std::string& cmd, const json& args) {
    auto it = conns_.find(toolName);
    if (it == conns_.end()) {
        printf("[ToolHub] send(): toolName error\n");
        return std::nullopt;
    }

    Conn& c = it->second;
    if (poll_process(c) != ProcessState::Running || c.stopping || c.fd < 0) {
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
        close_socket(c);
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
    if (poll_process(c) != ProcessState::Running || c.stopping || c.fd < 0) {
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

bool ToolHub::send_json_line(int fd, const json& j) {
    std::string s = j.dump();
    s.push_back('\n');

    const char* p = s.data();
    size_t left = s.size();
    const auto deadline = HubClock::now() + std::chrono::milliseconds(5000);
    while (left > 0) {
        ssize_t n = ::send(fd, p, left, MSG_NOSIGNAL | MSG_DONTWAIT);
        if (n < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                const int timeout = remaining_ms(deadline);
                if (timeout == 0) return false;
                pollfd pfd { fd, POLLOUT, 0 };
                if (::poll(&pfd, 1, timeout) < 0 && errno != EINTR) return false;
                continue;
            }
            return false;
        }
        if (n == 0 || remaining_ms(deadline) == 0) return false;
        p += n;
        left -= (size_t)n;
    }
    return true;
}

std::optional<json> ToolHub::recv_json_line(Conn& c, int timeoutMs) {
    const auto deadline = HubClock::now() + std::chrono::milliseconds(std::max(timeoutMs, 0));
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

        int pr = ::poll(&pfd, 1, timeoutMs < 0 ? -1 : remaining_ms(deadline));
        if (pr == 0)
            return std::nullopt; // timeout
        if (pr < 0) {
            if (errno == EINTR)
                continue;
            return std::nullopt;
        }

        char tmp[1024];
        ssize_t n = ::recv(c.fd, tmp, sizeof(tmp), 0);
        if (n == 0) {
            close_socket(c);
            return std::nullopt;
        }
        if (n < 0) {
            if (errno == EINTR)
                continue;
            close_socket(c);
            return std::nullopt;
        }
        c.readBuf.append(tmp, tmp + n);
        if (c.readBuf.size() > 65536) {
            close_socket(c);
            return std::nullopt;
        }
        if (timeoutMs >= 0 && remaining_ms(deadline) == 0
            && c.readBuf.find('\n') == std::string::npos)
            return std::nullopt;
    }
}
