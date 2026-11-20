#include "exec_cmd.hpp"
#include "tool_hub.hpp"
#include "server.hpp"
#include "bardy_session.hpp"
#include "../../log_level.hpp"

bool CommandExecutor::check_lid(json& response) {
    if ((lid < 1 || lid > 2)) {
        ZVEZDA_DEBUG_LOG(printf("<ERR> Bad LID number!\n"));
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

    if (baseName != "srv" && request[baseName] != "stop"
        && !BardySession::inst().require_open(response))
        return response;

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
    response["tool"] = toolName;

    std::string dir = std::string(lid == 2 ? "2" : "") + "examples";
    std::string path = (getExePath().parent_path().parent_path() / dir).lexically_normal().string();
    std::string exeTool = path + "/" + baseName + "_mngr";
    
    std::string ini = get_ini(request.value("ini", ""));

    std::string error;
    bool ok = ToolHub::inst().start_tool(toolName, exeTool, ini, path, startTimeoutToolMs, &error);
    if (!ok) {
        response["error"] = error;
        ZVEZDA_DEBUG_LOG(BRDC_printf("%s start(): %s\n", printDebugName.c_str(), error.c_str()));
        return false;
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(waitTimeoutToolS));
    if (!ToolHub::inst().check_tool_process(toolName, error)) {
        response["error"] = error;
        return false;
    }
    return true;
}

bool CommandExecutor::stop(const json& request, json& response) {
    if (!check_lid(response)) return false;
    response["tool"] = toolName;
    const auto stopped = ToolHub::inst().stop_tool(toolName);
    if (!stopped.success) {
        response["error"] = stopped.error;
        return false;
    }
    // Выход после SIGKILL не подтверждает штатное освобождение ресурсов.
    if (stopped.forced) {
        response["error"] = "Tool did not stop within the timeout and was forcibly terminated; normal cleanup was not confirmed";
        return false;
    }
    return true;
}
