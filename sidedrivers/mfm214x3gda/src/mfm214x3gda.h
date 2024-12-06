///
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief Файл команд, структур и констант всех служб модуля FM214x3GDA
///
#pragma once
#include <ctrladmpro/ctrl_jesd.h>
#include <ctrladmpro/ctrladc.h>
#include <ctrladmpro/ctrldac.h>
#include <ctrladmpro/ctrlddc.h>

// Общие и уникальные команды служб модуля
// -----------------------------------------------------------------------------
enum BRD_Command_FM214x3GDA : uint32_t {
    BRDctrl_SETDEBUGINFO = BRDctrl_DAQ, // Установка отладочной информации

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

    BRDctrl_DAC_SETMAINNCO,
    BRDctrl_DAC_DDSENABLE,
    BRDctrl_DAC_SETDDSAMPL,
    BRDctrl_DAC_SETCHANNELNCO,
    BRDctrl_DAC_SETJESDMODE, // установка режима JESD
    BRDctrl_DAC_CALIBRATION, // запуск калибровки выходных каналов
    BRDctrl_DAC_SETSTARTLEVEL,
    BRDctrl_DAC_SETSYSREF,
    BRDctrl_DAC_SETPRESYNC, // установка режима PreSync
};

// Структуры служб субмодуля
// -----------------------------------------------------------------------------
#pragma pack(push, 1)

typedef struct _BRD_ClkMode_FM214x3GDA {
    uint32_t src; // IN - clock source
    double Fout; // Частота работы АЦП
    double Fosc; // Частота осцилятора на субмодуле
    uint32_t PwrA; // LMX Мощность сигнала канала A
    uint32_t PwrB; // LMX Мощность сигнала канала B
    bool OutA; // LMX Включение канала A
    bool OutB; // LMX Включение канала B
    uint32_t Mult; // ЦАП коэффициент умножения входной частоты
} BRD_ClkMode_FM214x3GDA, *PBRD_ClkMode_FM214x3GDA;

typedef struct _BRD_AdcMode_FM214x3GDA {
    size_t app_mode; // Номер режима приложения. (Chip Application Mode)
    size_t channel_mask; // Маска каналов ADC: 0x01, 0x03 или DDC: 0x01, 0x03, 0x0F, 0x33, 0xFF
    size_t ddc_decimate; // Значение децимации для DDC.
    // bool   ddc_complex;  // Признак комплексного сигнала IQ для DDC.
    // bool   ddc_enable;
    // size_t ddc_decimation;   // Коэффициент децимации DDC: 2 - 48
    // size_t ddc_mixer_select; // Тип сигнала от АЦП: 0 - действительный, 1 - комплексный
} BRD_AdcMode_FM214x3GDA, *PBRD_AdcMode_FM214x3GDA;

typedef struct _BRD_DdcInputIQ_FM214x3GDA {
    size_t channel; // номер канала DDC
    size_t src_i; // номер входного АЦП для канала I
    size_t src_q; // номер входного АЦП для канала Q
} BRD_DdcInputIQ_FM214x3GDA, *PBRD_DdcInputIQ_FM214x3GDA;

typedef struct _BRD_StartLevel_FM214x3GDA {
    double level; // уровень внешнего старта
    uint32_t resistance; // входное сопротивление внешнего старта
} BRD_StartLevel_FM214x3GDA, *PBRD_StartLevel_FM214x3GDA;

typedef struct _BRD_ClockDelayChannel_FM214x3GDA {
    size_t channel; // номер канала АЦП
    uint8_t divider_phase; // 0x109 Clock divider phase
    uint8_t superfine_delay; // 0x111 Clock superfine delay
    uint8_t fine_delay; // 0x112 Clock fine delay
} BRD_ClockDelayChannel_FM214x3GDA, *PBRD_ClockDelayChannel_FM214x3GDA;

typedef struct _BRD_FastDetect_FM214x3GDA {
    size_t channel; // номер канала АЦП
    bool enable; // включить/выключить Fast Detect
    double threshold_upper; // верхний порог [-78..0] dBFS, по умолчанию 0
    double threshold_lower; // нижний порог [-78..0] dBFS, по умолчанию 0
    uint16_t dwell_time; // время срабатывания [1..65535] в сэмплах, по умолчанию 1
} BRD_FastDetect_FM214x3GDA, *PBRD_FastDetect_FM214x3GDA;

typedef struct _BRD_JESDMode_FM214x3GDA {
    size_t mode; // режим JESD'а
    bool dual; // признак Dual линка (Single -- false)
    size_t ch_mode; // режим интерполяции каналов
    size_t dp_mode; // режим интерполяции выходных каналов
    bool ext_core_clk; // признак использования внешней тактовой частоты для CoreClk

} BRD_JESDMode_FM214x3GDA, *PBRD_JESDMode_FM214x3GDA;

enum class BRD_PRESYNC_TYPE_FM214x3GDA : size_t {
    SYNC = 0,
    ASYNC = 1
};

typedef struct _BRD_PreSync_FM214x3GDA {
    bool enable; // включить режим PreSync
    BRD_PRESYNC_TYPE_FM214x3GDA type; // тип режима
} BRD_PreSync_FM214x3GDA, *PBRD_PreSync_FM214x3GDA;

typedef struct _BRD_AMPL_FM214x3GDA {
    size_t num_chan; // номер канала
    uint16_t ampl; // значение амплитуды тестового сигнала
} BRD_AMPL_FM214x3GDA, *PBRD_AMPL_FM214x3GDA;

typedef struct _BRD_NCO_FM214x3GDA {
    size_t num_chan; // номер канала
    double freq; // значение выходной частоты
    double phase; // значение начальной фазы
    double gain; // значение усиления
} BRD_NCO_FM214x3GDA, *PBRD_NCO_FM214x3GDA;

typedef struct _BRD_SysRefConfig_FM214x3GDA {
    double level; // уровень внешнего сигнала SysRef
    uint32_t select; // выбор источника сигнала SysRef
} BRD_SysRefConfig_FM214x3GDA, *PBRD_SysRefConfig_FM214x3GDA;

// Режимы тестовой последовательности
// -----------------------------------------------------------------------------
enum class PRBS_TYPE : size_t {
    NONE, // отключён
    PN9, // 9-битный полином
    PN23 // 23-битный полином
};

typedef struct _BRD_TimeStamp_FM214x3GDA {
    bool enable; // включить режим TimeStamp
    bool polarity_negative; // признак отрицательной полярность внешнего сигнала SYSREF
} BRD_TimeStamp_FM214x3GDA, *PBRD_TimeStamp_FM214x3GDA;

#pragma pack(pop)
