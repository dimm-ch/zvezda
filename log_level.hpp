#pragma once

// Смена уровня требует пересборки сервера и утилит.
// 0 — без диагностики обвязки, 1 — прежний вывод, 2 — вся диагностика.
#ifndef LOG_LEVEL
#define LOG_LEVEL 2
#endif

static_assert(LOG_LEVEL == 0 || LOG_LEVEL == 1 || LOG_LEVEL == 2,
    "LOG_LEVEL must be 0, 1 or 2");

// Только печать: действия управления и обмена должны выполняться вне макроса.
#if LOG_LEVEL >= 2
#define ZVEZDA_DEBUG_LOG(...) do { __VA_ARGS__; } while (false)
#else
#define ZVEZDA_DEBUG_LOG(...) do {} while (false)
#endif
