#pragma once

#include "exec_cmd.hpp"

class AdcControl final : public CommandExecutor {
public:
    AdcControl();
    ~AdcControl() = default;

    static std::string command();
    size_t findTetrad();
    
private:
    bool dispatch(const json& request, json& response);
    virtual std::string get_ini(const std::string& ini) override;
    
    using cmd_handler = bool (AdcControl::*)(const json&, json&);
    static const std::unordered_map<std::string, cmd_handler> cmd_map;
};