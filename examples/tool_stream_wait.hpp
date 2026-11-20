#pragma once

#include "brd.h"
#include "tool_stop.hpp"
#include <chrono>

namespace tool_stop {

inline S32 cleanup_control(BRD_Handle handle, U32 command, void* args, const char* phase)
{
    const bool trace = requested();
    if (trace) begin(phase);
    const S32 status = BRD_ctrl(handle, 0, command, args);
    if (trace || requested()) end(phase, status);
    return status;
}

// Общий тайм-аут сохраняется; короткие ожидания позволяют проверять запрос остановки.
// При отмене возвращаем WAIT_TIMEOUT; остановку DMA и освобождение буферов
// выполняет вызывающий код.
inline S32 wait_stream(BRD_Handle handle, U32 command, ULONG timeoutMs)
{
    if (requested()) return BRDerr_WAIT_TIMEOUT;
    if (timeoutMs == 0) return BRD_ctrl(handle, 0, command, &timeoutMs);

    const bool infinite = timeoutMs == 0xFFFFFFFFUL;
    const auto deadline = std::chrono::steady_clock::now()
        + std::chrono::milliseconds(timeoutMs);
    while (!requested()) {
        const auto now = std::chrono::steady_clock::now();
        if (!infinite && now >= deadline) return BRDerr_WAIT_TIMEOUT;
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
        ULONG sliceMs = infinite || remaining > 100 ? 100 : (remaining > 0 ? ULONG(remaining) : 1);
        const S32 status = BRD_ctrl(handle, 0, command, &sliceMs);
        if (!BRD_errcmp(status, BRDerr_WAIT_TIMEOUT)) return status;
    }
    return BRDerr_WAIT_TIMEOUT;
}

}
