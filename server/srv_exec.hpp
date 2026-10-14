#pragma once

#include <map>
#include <string>
#include <cstdint>
#include <exception>
#include <atomic>
#include <mutex>
#include <thread>

#include "cmd/exec_cmd.hpp"

class ServerExecutor {
    using CommandMap = std::map<std::string, CommandExecutor::UniquePtr>;
    CommandMap commandMap {};
    int _server = -1;
    int _socket = -1;
    std::exception_ptr threadError {};
    std::atomic<bool> stopping_ { false };
    std::thread worker_;
    std::mutex socketMutex_;
    std::mutex errorMutex_;

public:
    ServerExecutor(std::string ip = {}, uint16_t port = 7777);
    ~ServerExecutor();

    template <typename CommandExecutorType>
    void add_command() {
        commandMap.emplace(CommandExecutorType::command(), std::make_unique<CommandExecutorType>());
    }

    void start();
    void stop();
    std::exception_ptr error();
};
