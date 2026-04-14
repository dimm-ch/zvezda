#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

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
    bool start_tool(const std::string& toolName, const std::string& exePath, const std::string& arg, const std::string& workingDir = std::string(), int startupTimeoutMs = -1);
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
    };

    bool setup_listen_socket();
    bool spawn_tool_process(const std::string& toolName);
    bool register_tool(const std::string& toolName, const std::string& exePath, const std::string& arg, const std::string& workingDir);
    bool wait_tool_hello(const std::string& toolName, int timeoutMs);
    void accept_and_attach_one(int timeoutMs);
    void close_conn(Conn& c);
    bool send_json_line(int fd, const nlohmann::json& j);
    std::optional<nlohmann::json> recv_json_line(Conn& c, int timeoutMs);

    std::string hubSockPath_ = "/tmp/toolhub.sock";
    int listenFd_ = -1;

    std::unordered_map<std::string, Conn> conns_;
    uint64_t nextId_ = 1;
};
