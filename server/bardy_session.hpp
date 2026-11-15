#pragma once

#include "json.hpp"

// Used only by the server's command-processing thread. Construction does not
// initialize BARDY or access hardware.
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
