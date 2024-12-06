///
/// \author Alexander U.Jurankov (h2e@insys.ru)
/// \brief Файл команд, структур и констант JESD функций
///

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "ctrladmpro.h"

/// Команды управления
typedef enum _BRDctrl_JESD : uint32_t {
    BRDctrl_JESD_SETLMFCCOUNT = BRDctrl_JESD_SECTION, /// Установка значений LMFC счётчиков в ПЛИС
    BRDctrl_JESD_GETLMFCCOUNT, /// Получение значений LMFC счётчиков в ПЛИС
    BRDctrl_JESD_SETLMFC, // установка калибровочных значений для JESD линков (Delay и Variable Delay Buffer)
    BRDctrl_JESD_GETLMFC, // чтение значений латентности JESD линков
    BRDctrl_JESD_GETLINKERRORSTATUS, // чтение регистра 0x01C
    BRDctrl_JESD_GETSYNCSTATUS, // чтение регистра STAT_STATUS (0x038, 0x060)
    BRDctrl_JESD_GETDEBUGSTATUS, // чтение регистра STAT_RX_DEBUG (0x03C, 0x05C)
    BRDctrl_JESD_SETPRBS, // установка режима тестовой последовательности PRBS
    BRDctrl_JESD_GETPRBSSTATUS, // получение значений статусов и ошибок в режиме PRBS
    BRDctrl_SYNC_ILLEGAL /// Недопустимая команда
} BRDctrl_JESD;

#pragma pack(push, 1)

/// Значения латентности LMFC Count в тактах CoreCLK (в ПЛИС)
typedef struct _BRD_JesdLmfcCount {
    size_t count; // значение латентности LMFC Count в тактах CoreCLK
    size_t F; // количество октетов в фрейме на линию
    size_t K; // количество фреймов в мултифрейме
} BRD_JesdLmfcCount, *PBRD_JesdLmfcCount;

/// Калибровочные значения Delay и Variable Delay Buffer для JESD линков (в ЦАП)
typedef struct _BRD_JesdLmfc {
    size_t delay; // описание на стр.44 руководства AD9176
    size_t variable;
} BRD_JesdLmfc, *PBRD_JesdLmfc;

/// Получаемые значения Latency Delay для JESD линков (в ЦАП)
typedef struct _BRD_JesdLmfcLatency {
    size_t delay_link0; // значение латентности линка 0 (описание на стр.44 руководства AD9176)
    size_t delay_link1; // значение латентности линка 1
    size_t L; // количество используемых линий
    size_t F; // количество октетов в фрейме на линию
    size_t K; // количество фреймов в мултифрейме
} BRD_JesdLmfcLatency, *PBRD_JesdLmfcLatency;

typedef struct _BRD_PrbsStatus_FM214x3GDA {
    uint16_t status; // статусы каналов 0 и 1
    uint16_t count_ch0; // счётчик ошибок канала 0
    uint16_t count_ch1; // счётчик ошибок канала 1
} BRD_PrbsStatus_FM214x3GDA, *PBRD_PrbsStatus_FM214x3GDA;

typedef struct _BRD_PrbsStatus_FM414x3G {
    uint16_t status; // статусы каналов 0 и 1
    uint16_t count_ch0; // счётчик ошибок канала 0
    uint16_t count_ch1; // счётчик ошибок канала 1
    uint16_t count_ch2; // счётчик ошибок канала 2
    uint16_t count_ch3; // счётчик ошибок канала 3
} BRD_PrbsStatus_FM414x3G, *PBRD_PrbsStatus_FM414x3G;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif
