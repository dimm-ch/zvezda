#pragma once

// New shutdown diagnostics only. Change at build time and rebuild all four targets.
// 0: off, 1: basic events and nonzero statuses, 2: detailed cleanup stages.
#ifndef LOG_LEVEL
#define LOG_LEVEL 2
#endif

static_assert(LOG_LEVEL == 0 || LOG_LEVEL == 1 || LOG_LEVEL == 2,
    "LOG_LEVEL must be 0, 1 or 2");
