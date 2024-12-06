///
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief Файл команд, структур и констант всех служб модуля FM414x3G
///
#pragma once
#include <ctrladmpro/ctrl_jesd.h>
#include <ctrladmpro/ctrladc.h>
#include <ctrladmpro/ctrlddc.h>

// Общие и уникальные команды служб модуля
// -----------------------------------------------------------------------------
enum BRD_Command_FM414x3G : uint32_t {
    BRDctrl_SETDEBUGINFO = BRDctrl_DAQ, // установка отладочной информации

    BRDctrl_ADC_SETMODE,
    BRDctrl_ADC_SETJESD,
    BRDctrl_ADC_SETINVERTCHANNELA,
    BRDctrl_ADC_SETINPUTFSVOLTAGE,
    BRDctrl_ADC_SETINPUTBUFFERCONTROL,
    BRDctrl_ADC_SETSTARTLEVEL,
    BRDctrl_ADC_SETSYSREF,
    BRDctrl_ADC_SETCLOCKDELAYCONTROL,
    BRDctrl_ADC_SETCLOCKDELAYCHANNEL,
    BRDctrl_DDC_SETFCPHASE,
    BRDctrl_ADC_SETFASTDETECTCHANNEL,
    BRDctrl_ADC_SETPRESYNC, // установка режима предварительной синхронизации JESD'ов
    BRDctrl_ADC_SETTIMESTAMP, // установка режима временных меток TimeStamp в потоке
    BRDctrl_ADC_SETTESTSEQUENCE, // замена потока ADC на тестовую последовательность
    BRDctrl_ADC_SETEVENTCNT, // установка/сброс счётчика внешнего события

    // установка цифровых компараторов для проекта "Криотрейд"
    BRDctrl_ADC_SETCOMPENABLE,
    BRDctrl_ADC_GETCOMPENABLE,
    BRDctrl_ADC_SETCOMPLEVEL,
    BRDctrl_ADC_GETCOMPLEVEL,
    BRDctrl_ADC_SETCOMPINVERSE,
    BRDctrl_ADC_GETCOMPINVERSE,
};

// Структуры служб субмодуля
// -----------------------------------------------------------------------------
#pragma pack(push, 1)
typedef struct _BRD_ClkMode_FM414x3G {
    uint32_t src; // IN - clock source
    double Fout; // Частота работы АЦП
    double Fosc; // Частота осцилятора на субмодуле
    uint32_t PwrA; // LMX Мощность сигнала канала A
    uint32_t PwrB; // LMX Мощность сигнала канала B
    bool OutA; // LMX Включение канала A
    bool OutB; // LMX Включение канала B
} BRD_ClkMode_FM414x3G, *PBRD_ClkMode_FM414x3G;

typedef struct _BRD_AdcMode_FM414x3G {
    size_t app_mode; // Номер режима приложения. (Chip Application Mode)
    size_t channel_mask; // Маска каналов ADC: 0x01, 0x03 или DDC: 0x01, 0x03, 0x0F, 0x33, 0xFF
    size_t ddc_decimate; // Значение децимации для DDC.
    // bool   ddc_complex;  // Признак комплексного сигнала IQ для DDC.
    // bool   ddc_enable;
    // size_t ddc_decimation;   // Коэффициент децимации DDC: 2 - 48
    // size_t ddc_mixer_select; // Тип сигнала от АЦП: 0 - действительный, 1 - комплексный
} BRD_AdcMode_FM414x3G, *PBRD_AdcMode_FM414x3G;

typedef struct _BRD_DdcInputIQ_FM414x3G {
    size_t channel; // номер канала DDC
    size_t src_i; // номер входного АЦП для канала I
    size_t src_q; // номер входного АЦП для канала Q
} BRD_DdcInputIQ_FM414x3G, *PBRD_DdcInputIQ_FM414x3G;

typedef struct _BRD_StartLevel_FM414x3G {
    double level; // уровень внешнего старта
    uint32_t resistance; // входное сопротивление внешнего старта
} BRD_StartLevel_FM414x3G, *PBRD_StartLevel_FM414x3G;

typedef struct _BRD_SysRefConfig_FM414x3G {
    double level; // уровень внешнего сигнала SysRef
    uint32_t select; // выбор источника сигнала SysRef
} BRD_SysRefConfig_FM414x3G, * PBRD_SysRefConfig_FM414x3G;

typedef struct _BRD_ClockDelayChannel_FM414x3G {
    size_t channel; // номер канала АЦП
    uint8_t divider_phase; // 0x109 Clock divider phase
    uint8_t superfine_delay; // 0x111 Clock superfine delay
    uint8_t fine_delay; // 0x112 Clock fine delay
} BRD_ClockDelayChannel_FM414x3G, *PBRD_ClockDelayChannel_FM414x3G;

typedef struct _BRD_FastDetect_FM414x3G {
    size_t channel; // номер канала АЦП
    bool enable; // включить/выключить Fast Detect
    double threshold_upper; // верхний порог [-78..0] dBFS, по умолчанию 0
    double threshold_lower; // нижний порог [-78..0] dBFS, по умолчанию 0
    uint16_t dwell_time; // время срабатывания [1..65535] в сэмплах, по умолчанию 1
} BRD_FastDetect_FM414x3G, *PBRD_FastDetect_FM414x3G;

enum class BRD_PRESYNC_TYPE_FM414x3G : size_t {
    SYNC = 0,
    ASYNC = 1
};

typedef struct _BRD_PreSync_FM414x3G {
    bool enable; // включить режим PreSync
    BRD_PRESYNC_TYPE_FM414x3G type; // тип режима
} BRD_PreSync_FM414x3G, *PBRD_PreSync_FM414x3G;

// Режимы тестовой последовательности
// -----------------------------------------------------------------------------
enum class PRBS_TYPE : size_t {
    NONE, // отключён
    PN9, // 9-битный полином
    PN23 // 23-битный полином
};

typedef struct _BRD_TimeStamp_FM414x3G {
    bool enable; // включить режим TimeStamp
    bool polarity_negative; // признак отрицательной полярность внешнего сигнала SYSREF
} BRD_TimeStamp_FM414x3G, *PBRD_TimeStamp_FM414x3G;

#pragma pack(pop)
