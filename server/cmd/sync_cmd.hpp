#pragma once
#include "exec_cmd.hpp"

class FmcSync final : public CommandExecutor {
public:
    FmcSync();
    ~FmcSync() = default;

    static std::string command();
    
private:
    bool check_lid(json& response) override;
    bool dispatch(const json& request, json& response);
    virtual std::string get_ini(const std::string& ini) override;
    
    using cmd_handler = bool (FmcSync::*)(const json&, json&);
    static const std::unordered_map<std::string, cmd_handler> cmd_map;
};