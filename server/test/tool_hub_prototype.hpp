#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

#include "json.hpp"

class ToolHubLite {
public:
    explicit ToolHubLite(std::string hubSockPath);
    ~ToolHubLite();

    // Поднять listen-сокет (один раз при старте сервера)
    bool init();
    void shutdown();

    // Зарегистрировать утилиту (имя -> путь бинарника).
    // Можно вызывать хоть 5 раз при старте сервера.
    void register_tool(const std::string& toolName, const std::string& exePath);

    // Запустить утилиту, если ещё не запущена, и ДОЖДАТЬСЯ готовности.
    // Если already connected -> сразу true.
    bool start_tool(const std::string& toolName, int startupTimeoutMs, bool doPing = true);

    // Синхронный вызов команды к утилите (предполагаем: один запрос за раз).
    std::optional<nlohmann::json> call(const std::string& toolName,
                                       const std::string& cmd,
                                       const nlohmann::json& args,
                                       int timeoutMs);

    bool is_connected(const std::string& toolName) const;

private:
    struct Conn {
        std::string exePath;
        int fd = -1;
        pid_t pid = -1;
        std::string readBuf;
    };

private:
    bool setup_listen_socket();
    bool spawn_tool_process(const std::string& toolName);

    // ждём подключение и hello от НУЖНОГО toolName
    bool wait_tool_hello(const std::string& toolName, int timeoutMs);

    // IO helpers (JSONL)
    bool send_json_line(int fd, const nlohmann::json& j);
    std::optional<nlohmann::json> recv_json_line(Conn& c, int timeoutMs);

    // принять новое соединение и разобрать hello, прикрепить к conns_
    void accept_and_attach_one(int timeoutMs);

    // безопасно закрыть соединение/процесс (если надо перезапускать)
    void close_conn(Conn& c);

private:
    std::string hubSockPath_;
    int listenFd_ = -1;

    std::unordered_map<std::string, Conn> conns_;
    uint64_t nextId_ = 1;
};
