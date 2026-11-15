#pragma once

#include "../log_level.hpp"
#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <stdexcept>
#if defined(__linux__)
#include <time.h>
#include <unistd.h>
#endif

namespace tool_stop {

// ADC already has worker threads. The signal flag must also be safe for them,
// and must never use a hidden lock from inside a signal handler.
static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "Tool stop flag must be lock-free");
static_assert(ATOMIC_INT_LOCK_FREE == 2, "Tool stop signal must be lock-free");
static_assert(ATOMIC_LONG_LOCK_FREE == 2, "Tool stop timestamp must be lock-free");

struct State {
    std::atomic<bool> stop { false };
    std::atomic<int> signal { 0 };
    std::atomic<long> seconds { -1 };
    std::atomic<long> nanoseconds { 0 };
    std::atomic<bool> observed { false };
    const char* name = "tool";
};

inline State& state() noexcept
{
    static State value;
    return value;
}

inline std::atomic<bool>& flag() noexcept
{
    return state().stop;
}

inline void handle_signal(int signal) noexcept
{
    const int savedErrno = errno;
    auto& value = state(); // Initialized by install(), before handlers exist.
    int expected = 0;
    if (value.signal.compare_exchange_strong(expected, signal, std::memory_order_relaxed)) {
#if defined(__linux__)
        // POSIX clock_gettime is async-signal-safe. Never print or call BARDY here.
        timespec received {};
        if (::clock_gettime(CLOCK_MONOTONIC, &received) == 0) {
            value.seconds.store(static_cast<long>(received.tv_sec), std::memory_order_relaxed);
            value.nanoseconds.store(static_cast<long>(received.tv_nsec), std::memory_order_relaxed);
        }
#endif
        // Publish the first signal and its timestamp together to all workers.
        value.stop.store(true, std::memory_order_release);
    }
    errno = savedErrno;
}

inline long long signal_ms() noexcept
{
    auto& value = state();
    if (!value.stop.load(std::memory_order_acquire)) return -1;
    const long seconds = value.seconds.load(std::memory_order_relaxed);
    if (seconds < 0) return -1;
    return static_cast<long long>(seconds) * 1000
        + value.nanoseconds.load(std::memory_order_relaxed) / 1000000;
}

inline long long elapsed_ms() noexcept
{
#if defined(__linux__)
    const long long received = signal_ms();
    timespec now {};
    if (received >= 0 && ::clock_gettime(CLOCK_MONOTONIC, &now) == 0)
        return static_cast<long long>(now.tv_sec) * 1000 + now.tv_nsec / 1000000 - received;
#endif
    return -1;
}

inline void log(const char* event, const char* phase, bool hasStatus = false, long status = 0, int level = 2) noexcept
{
#if LOG_LEVEL > 0
    if (LOG_LEVEL < level) return;
    const int savedErrno = errno;
    long pid = 0;
#if defined(__linux__)
    pid = static_cast<long>(::getpid());
#endif
    const auto& value = state();
    // A single stdio call per record keeps worker/main records from interleaving.
    if (hasStatus)
        std::fprintf(stderr, "[tool-stop] tool=%s pid=%ld event=%s phase=%s signal=%d signal_mono_ms=%lld since_signal_ms=%lld status=%ld status_hex=0x%08lX\n",
            value.name, pid, event, phase, value.signal.load(std::memory_order_relaxed),
            signal_ms(), elapsed_ms(), status, static_cast<unsigned long>(static_cast<unsigned int>(status)));
    else
        std::fprintf(stderr, "[tool-stop] tool=%s pid=%ld event=%s phase=%s signal=%d signal_mono_ms=%lld since_signal_ms=%lld\n",
            value.name, pid, event, phase, value.signal.load(std::memory_order_relaxed), signal_ms(), elapsed_ms());
    std::fflush(stderr);
    errno = savedErrno;
#else
    (void)event;
    (void)phase;
    (void)hasStatus;
    (void)status;
    (void)level;
#endif
}

inline bool requested() noexcept
{
    auto& value = state();
    const bool stop = value.stop.load(std::memory_order_acquire);
    if (stop && !value.observed.exchange(true, std::memory_order_relaxed))
        log("signal-observed", "stop-request", false, 0, 1);
    return stop;
}

inline void begin(const char* phase) noexcept
{
    requested();
    log("begin", phase);
}

inline void end(const char* phase, long status) noexcept
{
    requested();
    // Preserve noteworthy statuses in basic mode without interpreting BRD codes here.
    log("end", phase, true, status, status != 0 ? 1 : 2);
}

inline void end(const char* phase) noexcept
{
    requested();
    log("end", phase);
}

inline int finish(int exitCode) noexcept
{
    requested();
    // Main's explicit cleanup is complete; the server separately confirms process exit.
    log("main-finished", "explicit-cleanup-complete", true, exitCode, 1);
    return exitCode;
}

// Call once, before initialization of hardware or creation of worker threads.
inline void install(const char* name)
{
    auto& value = state();
    value.name = name;
    value.signal.store(0, std::memory_order_relaxed);
    value.seconds.store(-1, std::memory_order_relaxed);
    value.nanoseconds.store(0, std::memory_order_relaxed);
    value.observed.store(false, std::memory_order_relaxed);
    flag().store(false, std::memory_order_relaxed);
    if (std::signal(SIGINT, handle_signal) == SIG_ERR
        || std::signal(SIGTERM, handle_signal) == SIG_ERR) {
        throw std::runtime_error("Cannot install tool stop signal handlers");
    }
    log("handlers-installed", "startup", false, 0, 1);
}

} // namespace tool_stop
