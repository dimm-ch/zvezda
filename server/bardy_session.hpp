#pragma once

#include "json.hpp"

// Используется только потоком обработки команд сервера.
class BardySession {
public:
    static BardySession& inst();

    bool open(nlohmann::json& response);
    bool close(nlohmann::json& response);
    bool require_open(nlohmann::json& response) const;
    const char* state_name() const;

private:
    BardySession() = default;
    BardySession(const BardySession&) = delete;
    BardySession& operator=(const BardySession&) = delete;

    enum class State { Closed, Open, Error };
    State state_ = State::Closed;
};
