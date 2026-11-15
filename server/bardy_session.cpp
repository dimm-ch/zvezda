#include "bardy_session.hpp"
#include "server.hpp"
#include "tool_hub.hpp"
#include "../log_level.hpp"

#include <cstdio>
#include <string>

BardySession& BardySession::inst() {
    static BardySession session;
    return session;
}

const char* BardySession::state_name() const {
    switch (state_) {
    case State::Closed: return "stopped";
    case State::Open: return "started";
    case State::Error: return "error";
    }
    return "error";
}

bool BardySession::require_open(json& response) const {
    if (state_ == State::Open) return true;
    response["error"] = state_ == State::Closed
        ? "Server is stopped; use srv:start first"
        : "Server cleanup is incomplete; retry srv:stop before srv:start";
    return false;
}

bool BardySession::open(json& response) {
    if (state_ == State::Open) return true;
    if (state_ == State::Error) return require_open(response);

    const auto ini = (getExePath().parent_path().parent_path() / "brd/brd.ini")
        .lexically_normal().string();
    S32 devices = 0;
    const S32 status = BRD_init(ini.c_str(), &devices);
    if (!BRD_errcmp(status, BRDerr_OK)) {
        x_DevNum = 0;
#if LOG_LEVEL >= 1
        std::fprintf(stderr, "[BardySession] BRD_init failed: status=0x%X\n",
            static_cast<unsigned int>(status));
#endif
        response["error"] = "Server initialization failed";
        return false;
    }

    x_DevNum = devices;
    state_ = State::Open;
    BRD_displayMode(BRDdm_VISIBLE | BRDdm_CONSOLE);
    return true;
}

bool BardySession::close(json& response) {
    // Reap exited children as well as checking live ones. A remembered tool or
    // a closed hello socket does not establish whether its process has exited.
    const auto activity = ToolHub::inst().inspect_processes();
    if (!activity.running.empty() || !activity.errors.empty()) {
        response["running_tools"] = activity.running;
        if (activity.errors.empty()) {
            response["error"] = "Stop running tools before srv:stop";
        } else {
            std::string error = "Cannot confirm tool exit: ";
            bool first = true;
            for (const auto& entry : activity.errors) {
                if (!first) error += ", ";
                error += entry.first;
                first = false;
#if LOG_LEVEL >= 1
                std::fprintf(stderr, "[BardySession] Cannot confirm exit of %s: %s\n",
                    entry.first.c_str(), entry.second.c_str());
#endif
            }
            response["error"] = error + "; server stop was not performed";
        }
        return false;
    }
    if (state_ == State::Closed) return true;

    // A partial release must not enable more hardware work or claim closed.
    // Retain failed handles so that a later srv:stop can retry them.
    state_ = State::Error;
    bool resourcesReleased = true;
    for (int lid = 0; lid < MAX_LID_DEVICES; ++lid) {
        auto& device = DevicesLid[lid];
        bool servicesReleased = true;
        auto release = [&](TRIadSrv& service) {
            if (!service.isCaptured()) return;
            S32 status = BRDerr_OK;
            if (!service.release_checked(status)) {
                servicesReleased = false;
                resourcesReleased = false;
#if LOG_LEVEL >= 1
                std::fprintf(stderr, "[BardySession] BRD_release failed: lid=%d service=%s status=0x%X\n",
                    lid, service.name().c_str(), static_cast<unsigned int>(status));
#endif
            }
        };
        release(device.servRegs);
        release(device.adc);
        for (auto& dac : device.dac) release(dac.servDac);
        if (!servicesReleased) continue;
        device.dac.clear();
        if (device.device.isOpen()) {
            S32 status = BRDerr_OK;
            if (!device.device.close_checked(status)) {
                resourcesReleased = false;
#if LOG_LEVEL >= 1
                std::fprintf(stderr, "[BardySession] BRD_close failed: lid=%d status=0x%X\n",
                    lid, static_cast<unsigned int>(status));
#endif
            }
        }
    }
    x_handleDevice = 0;
    if (resourcesReleased) {
        const S32 status = BRD_cleanup();
        if (BRD_errcmp(status, BRDerr_OK)) {
            state_ = State::Closed;
            x_DevNum = 0;
        } else {
            resourcesReleased = false;
#if LOG_LEVEL >= 1
            std::fprintf(stderr, "[BardySession] BRD_cleanup failed: status=0x%X\n",
                static_cast<unsigned int>(status));
#endif
        }
    }
    if (!resourcesReleased) {
        response["error"] = "Server resources were not fully released; retry srv:stop";
        return false;
    }
    return true;
}
