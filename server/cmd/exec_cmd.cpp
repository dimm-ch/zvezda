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

    std::string error;
    bool ok = ToolHub::inst().start_tool(toolName, exeTool, ini, path, startTimeoutToolMs, &error);
    if (!ok) {
        response["error"] = error;
        BRDC_printf("%s start(): %s\n", printDebugName.c_str(), error.c_str());
        return false;
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(waitTimeoutToolS));
    response["tool"] = toolName;
    if (!ToolHub::inst().check_tool_process(toolName, error)) {
        response["error"] = error;
        return false;
    }
    return true;
}

static json stop_response(const ToolHub::StopResult& stopped) {
    json response = {
        {"result", stopped.success ? "Success" : "Error"},
        {"forced", stopped.forced},
        {"already_stopped", stopped.alreadyStopped}
    };
    if (stopped.exitCode) response["exit_code"] = *stopped.exitCode;
    if (stopped.termSignal) response["signal"] = *stopped.termSignal;
    if (!stopped.error.empty()) response["error"] = stopped.error;
    return response;
}

bool CommandExecutor::stop(const json& request, json& response) {
    if (!check_lid(response)) return false;
    const auto stopped = ToolHub::inst().stop_tool(toolName);
    response = stop_response(stopped);
    response["tool"] = toolName;
    return stopped.success;
}

bool CommandExecutor::restart(const json& request, json& response) {
    if (!check_lid(response)) return false;
    if (request.contains("ini")) {
        response["error"] = "Restart reuses saved launch parameters; use stop/start to change ini";
        return false;
    }
    const auto restarted = ToolHub::inst().restart_tool(toolName);
    response["tool"] = toolName;
    if (restarted.stopAttempted)
        response["stop"] = stop_response(restarted.stop);
    if (!restarted.success) {
        response["error"] = restarted.error;
        return false;
    }
    std::this_thread::sleep_for(std::chrono::seconds(waitTimeoutToolS));
    std::string error;
    if (!ToolHub::inst().check_tool_process(toolName, error)) {
        response["error"] = error;
        return false;
    }
    return true;
}
