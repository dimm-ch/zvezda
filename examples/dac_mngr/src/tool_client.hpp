#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "../../../server/json.hpp"

class ToolClient
{
public:
    struct Command {
        uint64_t id = 0;
        std::string cmd;
        nlohmann::json args;
    };
    std::string toolName_;

    static ToolClient& inst();

    bool connect_and_hello(const std::string& hubSockPath, const std::string& toolName);

    // Проверить/подождать команду:
    // timeoutMs = 0  -> не ждать, просто проверить
    // timeoutMs > 0  -> подождать до timeoutMs
    // timeoutMs < 0  -> ждать бесконечно
    std::optional<Command> poll_command(int timeoutMs);

    // Отправить ответ на команду.
    bool send_ok(uint64_t id, const std::string& result = std::string());
    bool send_error(uint64_t id, const std::string& err = std::string());

    // Для аккуратного завершения (не обязательно)
    void close();

    bool is_connected() const;

private:
    ToolClient() = default;
    ~ToolClient();

    ToolClient(const ToolClient&) = delete;
    ToolClient& operator=(const ToolClient&) = delete;

private:
    bool send_json_line(const nlohmann::json& j);
    std::optional<nlohmann::json> recv_json_line(int timeoutMs);

private:
    int fd_ = -1;
    std::string readBuf_;
    uint64_t currentCmdId_ = 0; // для отладки (не обязательно)
};