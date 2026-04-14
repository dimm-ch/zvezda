#include "exec_cmd.hpp"
#include "tool_hub.hpp"
#include "server.hpp"

bool CommandExecutor::check_lid(json& response) {
    if ((lid < 1 || lid > 2)) {
        printf("<ERR> Bad LID number!\n");
        response["error"] = "Bad LID!";
        return false;
    }

    toolName = baseName + std::to_string(lid);
    return true;
}

json CommandExecutor::execute(const json& request) {
    json response;
    response["result"] = "Error";
    
    lid = get_json_value_or<int>(request, "lid", 0);

    if (!request.contains(baseName)) {
        response["error"] = "Parameter is missing: " + baseName;
        return response;
    }

    if (dispatch(request, response))
        response["result"] = "Success";
        
    return response;
}

std::string CommandExecutor::get_ini(const std::string& ini) {
    return ini;
}

bool CommandExecutor::start(const json& request, json& response) {
    if (!check_lid(response)) {
        return false;
    }

    std::string dir = std::string(lid == 2 ? "2" : "") + "examples";
    std::string path = (getExePath().parent_path().parent_path() / dir).lexically_normal().string();
    std::string exeTool = path + "/" + baseName + "_mngr";
    
    std::string ini = get_ini(request.value("ini", ""));

    bool ok = ToolHub::inst().start_tool(toolName, exeTool, ini, path, startTimeoutToolMs);
    if (!ok) {
        response["error"] = "Tool start timeout or failed: " + exeTool;
        BRDC_printf("%s start() Tool start timeout or failed: %s\n", printDebugName.c_str(), exeTool.c_str());
        return false;
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(waitTimeoutToolS));
    return true;
}
