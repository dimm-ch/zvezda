#pragma once

#include <map>
#include <string>
#include <cstdint>
#include <exception>

#include "cmd/exec_cmd.hpp"

class ServerExecutor {
    using CommandMap = std::map<std::string, CommandExecutor::UniquePtr>;
    CommandMap commandMap {};
    int _server {};
    int _socket = -1;
    std::exception_ptr threadError {};

public:
    ServerExecutor(std::string ip = {}, uint16_t port = 7777);
    ~ServerExecutor();

    template <typename CommandExecutorType>
    void add_command() {
        commandMap.emplace(CommandExecutorType::command(), std::make_unique<CommandExecutorType>());
    }

    void start();
    std::exception_ptr error();
};