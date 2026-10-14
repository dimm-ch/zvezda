#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <sys/types.h>

#include "json.hpp"

class ToolHub {
public:
    static ToolHub& inst();

    ToolHub(const ToolHub&) = delete;
    ToolHub& operator=(const ToolHub&) = delete;
    ToolHub(ToolHub&&) = delete;
    ToolHub& operator=(ToolHub&&) = delete;

    bool init();
    void shutdown();
    struct StopResult {
        bool success = false;
        bool forced = false;
        bool alreadyStopped = false;
        std::optional<int> exitCode;
        std::optional<int> termSignal;
        std::string error;
    };
    struct RestartResult {
        bool success = false;
        bool stopAttempted = false;
        StopResult stop;
        std::string error;
    };
    static constexpr int StopTimeoutMs = 5000;
    static constexpr int KillTimeoutMs = 2000;

    bool start_tool(const std::string& toolName, const std::string& exePath, const std::string& arg, const std::string& workingDir = std::string(), int startupTimeoutMs = -1, std::string* error = nullptr);
    StopResult stop_tool(const std::string& toolName, int stopTimeoutMs = StopTimeoutMs, int killTimeoutMs = KillTimeoutMs);
    RestartResult restart_tool(const std::string& toolName);
    bool check_tool_process(const std::string& toolName, std::string& error);
    bool is_connected(const std::string& toolName) const;
    std::optional<uint64_t> send(const std::string& toolName, const std::string& cmd, const nlohmann::json& args);
    std::optional<nlohmann::json> wait(const std::string& toolName, uint64_t id, int timeoutMs);
    std::optional<nlohmann::json> call(const std::string& toolName, const std::string& cmd, const nlohmann::json& args, int timeoutMs);
    
private:
    ToolHub();
    ~ToolHub();
    
    struct Conn {
        std::string exePath;
        std::string workingDir; 
        std::vector<std::string> args;
        int fd = -1;
        pid_t pid = -1;
        std::string readBuf;
        int startupTimeoutMs = -1;
        bool stopping = false;
        std::optional<int> exitStatus;
        std::string processError;
    };

    enum class ProcessState { Running, Exited, Error, Timeout };

    bool setup_listen_socket();
    bool spawn_tool_process(Conn& c, std::string& error);
    bool launch_tool(const std::string& toolName, Conn& c, std::string& error);
    bool wait_tool_hello(const std::string& toolName, int timeoutMs, std::string& error);
    void accept_and_attach_one(int timeoutMs);
    void close_socket(Conn& c);
    ProcessState poll_process(Conn& c);
    ProcessState wait_process(Conn& c, int timeoutMs);
    StopResult stop_process(Conn& c, int stopTimeoutMs, int killTimeoutMs);
    static void describe_exit(const Conn& c, StopResult& result);
    bool send_json_line(int fd, const nlohmann::json& j);
    std::optional<nlohmann::json> recv_json_line(Conn& c, int timeoutMs);

    std::string hubSockPath_ = "/tmp/toolhub.sock";
    int listenFd_ = -1;

    std::unordered_map<std::string, Conn> conns_;
    uint64_t nextId_ = 1;
};
