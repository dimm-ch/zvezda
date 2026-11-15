#pragma once

#include "brd.h"
#include "tool_stop.hpp"
#include <chrono>

namespace tool_stop {

// Diagnostic wrapper only: preserve the command, arguments and return status.
// Normal acquisition/output remains quiet; log cleanup calls after a stop request.
inline S32 cleanup_control(BRD_Handle handle, U32 command, void* args, const char* phase)
{
    const bool trace = requested();
    if (trace) begin(phase);
    const S32 status = BRD_ctrl(handle, 0, command, args);
    if (trace || requested()) end(phase, status);
    return status;
}

// Keep the original total timeout, but return to the utility every 100 ms.
// Cancellation returns WAIT_TIMEOUT; the caller must stop DMA and release its
// buffers as usual. Intermediate timeouts do not stop or restart the stream.
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

} // namespace tool_stop
