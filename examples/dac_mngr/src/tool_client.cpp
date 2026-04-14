#include "tool_client.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>

using json = nlohmann::json;

static int connect_unix(const std::string& path) {
    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path.c_str());

    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        ::close(fd);
        return -1;
    }
    return fd;
}

ToolClient& ToolClient::inst() {
    static ToolClient inst;
    return inst;
}

ToolClient::~ToolClient() {
    close();
}

bool ToolClient::connect_and_hello(const std::string& hubSockPath, const std::string& toolName) {
    printf("[ToolClient] connect_and_hello()\n");
    // если уже подключены — считаем успехом
    if (fd_ >= 0)
        return true;

    int fd = connect_unix(hubSockPath);
    if (fd < 0) {
        printf("[ToolClient] connect_and_hello(): connect failed: %s\n", strerror(errno));
        return false;
    }

    fd_ = fd;
    toolName_ = toolName;
    readBuf_.clear();

    // hello
    json hello = {{"type","hello"}, {"tool", toolName_}};
    if (!send_json_line(hello)) {
        printf("[ToolClient] connect_and_hello(): send hello failed\n");
        close();
        return false;
    }

    return true;
}

void ToolClient::close()
{
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
    readBuf_.clear();
    toolName_.clear();
    currentCmdId_ = 0;
}

bool ToolClient::is_connected() const {
    return fd_ >= 0;
}
bool ToolClient::send_json_line(const json& j)
{
    if (fd_ < 0)
        return false;

    std::string s = j.dump();
    s.push_back('\n');

    const char* p = s.data();
    size_t left = s.size();

    while (left > 0) {
        ssize_t n = ::send(fd_, p, left, MSG_NOSIGNAL);
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

std::optional<json> ToolClient::recv_json_line(int timeoutMs)
{
    if (fd_ < 0)
        return std::nullopt;

    auto wait_readable = [&](int ms) -> bool {
        pollfd pfd{};
        pfd.fd = fd_;
        pfd.events = POLLIN;

        int pr;
        do {
            pr = ::poll(&pfd, 1, ms);
        } while (pr < 0 && errno == EINTR);

        if (pr <= 0)
            return false; // timeout or error
        return (pfd.revents & POLLIN) != 0;
    };

    while (true) {
        auto pos = readBuf_.find('\n');
        if (pos != std::string::npos) {
            std::string line = readBuf_.substr(0, pos);
            readBuf_.erase(0, pos + 1);
            if (line.empty()) continue;

            try { return json::parse(line); }
            catch (...) { return std::nullopt; }
        }

        // ждать данные
        int pollMs = timeoutMs;
        if (timeoutMs < 0)
            pollMs = -1; // бесконечно
        if (!wait_readable(pollMs))
            return std::nullopt;

        char tmp[1024];
        ssize_t n;
        do {
            n = ::recv(fd_, tmp, sizeof(tmp), 0);
        } while (n < 0 && errno == EINTR);

        if (n == 0) {
            // сервер закрыл соединение
            close();
            return std::nullopt;
        }
        if (n < 0) {
            close();
            return std::nullopt;
        }

        readBuf_.append(tmp, tmp + n);

        // важно: если timeoutMs == 0, мы не должны блокироваться “вечно”
        // после одного чтения просто пойдём наверх и попробуем извлечь строку
        if (timeoutMs == 0) {
            // если строки ещё нет — команды нет (пока)
            auto p = readBuf_.find('\n');
            if (p == std::string::npos)
                return std::nullopt;
        }
    }
}

std::optional<ToolClient::Command> ToolClient::poll_command(int timeoutMs)
{
    // timeoutMs:
    // 0  -> не ждать
    // >0 -> ждать до timeoutMs
    // <0 -> ждать бесконечно
    auto msgOpt = recv_json_line(timeoutMs);
    if (!msgOpt) return std::nullopt;

    const json& msg = *msgOpt;
    
    if (!msg.contains("id") || !msg["id"].is_number_unsigned())
        return std::nullopt;
    if (!msg.contains("cmd") || !msg["cmd"].is_string())
        return std::nullopt;

    Command c;
    c.id = msg.value("id", 0ull);
    c.cmd = msg.value("cmd", "");
    c.args = msg.value("args", json::object());

    currentCmdId_ = c.id;
    return c;
}

bool ToolClient::send_ok(uint64_t id, const std::string& result) {
    json resp = {{"id", id}, {"result", true}, {"value", result }};
    printf("[ToolClient] send_ok(): %s\n", resp.dump().c_str());
    return send_json_line(resp);
}

bool ToolClient::send_error(uint64_t id, const std::string& err) {
    json resp = {{"id", id}, {"result", false}, {"error", err }};
    printf("[ToolClient] send_error(): %s\n", resp.dump().c_str());
    return send_json_line(resp);
}