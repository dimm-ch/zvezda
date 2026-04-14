#pragma once

#include "exec_cmd.hpp"

class DacControl final : public CommandExecutor {
public:
    DacControl();
    ~DacControl() = default;

    static std::string command();
    size_t findTetrad();

private:
    bool dispatch(const json& request, json& response);
    virtual std::string get_ini(const std::string& ini) override;
    
    bool ping(const json& request, json& response);
    bool nco(const json& request, json& response);

    using cmd_handler = bool (DacControl::*)(const json&, json&);
    static const std::unordered_map<std::string, cmd_handler> cmd_map;
};
