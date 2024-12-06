///
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief
///

#pragma once
#include "AD9208.h"
#include "mfm414x3g.h"
#include "service.h"

class Cmfm414x3g; // класс родителя

// Служба АЦП
class SrvADC : public CService {
    friend class Cmfm414x3g;
    static constexpr auto name_class = "ADC"; // имя класса для отладочных сообщений

    // порядковый номер и ID DAC тетрады
    int m_tetr_num = -1;
    static constexpr size_t ADC_ID = 0x119;
    size_t m_fifo_size = 0; // размер буфера FIFO
    size_t m_fifo_type = 0; // размер шины FIFO в битах
    size_t m_jesd_phy_id = 0; // идентификатор JESD PHY
    bool m_global_error = false; // глобальный признак ошибки
    bool m_cryo = false; // признак доп.функций для проекта "Криотрейд Инжиниринг"
    size_t m_timeout_msec = 5000;
    AD9208 m_adc0; // АЦП0 AD9208
    AD9208 m_adc1; // АЦП1 AD9208

    // параметры АЦП со значениями по умолчанию
    size_t m_bits = 14; // разрядность АЦП (14 бит) или DDC (16 бит)
    size_t m_encoding = 1; // тип кодировки (1 - двоично-дополнительный)
    double m_min_rate = 2.5; // минимальная частота дискретизации (2.5 МГц)
    double m_max_rate = 3.1; // максимальная частота дискретизации (3.1 МГц)
    double m_inp_range = 0.5; // выходной диапазон (0.5 В)

public:
    SrvADC(Cmfm414x3g* parent);

    // общие команды для всех служб/сервисов
    auto CtrlIsAvailable(void* pDev, void* pServData, ULONG cmd, void* args) -> int override;
    auto CtrlCapture(void* pDev, void* pServData, ULONG cmd, void* args) -> int override;
    auto CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args) -> int override;
    auto CtrlGetAddrData(void* pDev, void* pServData, ULONG cmd, void* args) -> int override;

    // уникальные команды службы ADC только для этого модуля
    auto CtrlSetDebugInfo(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetJesd(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetDDCInputSource(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetInvertChannelA(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetInputFSVoltage(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetInputBufferControl(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetStartLevel(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetClockDelayControl(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetClockDelayChannel(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetDDCFcPhase(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetFastDetectChannel(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetLMFCCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetLMFCCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetSysRef(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetPresync(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetTimeStamp(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetTestSequence(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetBlkCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetEventCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetJESDLinkErrorStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetJESDSyncStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetJESDDebugStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetJESDPrbs(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetJESDPrbsStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int;

    // установка цифровых компараторов для проекта "Криотрейд"
    auto CtrlSetCompEnable(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetCompEnable(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetCompLevel(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetCompLevel(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetCompInverse(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetCompInverse(void* pDev, void* pServData, ULONG cmd, void* args) -> int;

    // общие команды службы ADC
    auto CtrlGetSrcStream(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlEnable(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlFifoReset(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlFifoStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetChanMask(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetChanMask(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetClkMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetDDCFc(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetDDCFc(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetStartMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetStartDelay(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetAcqCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetSkipCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetTitleMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetTitleData(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetPretrigMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetPretrigMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetDelay(void* pDev, void* pServData, ULONG cmd, void* args) -> int;

    // команды службы ADC, необходимые только для `exam_edac`
    auto CtrlGetCfg(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlReadIniFile(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetSyncMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetStartMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlReadDelay(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetFormat(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetInpRange(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetBias(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlPrepareStart(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlIsComplex(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    // auto CtrlGetInterpFactor(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetMaster(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetTraceText(void* pDev, void* pServData, ULONG cmd, void* args) -> int;

    // Регистр управления субмодулем
    using CONTROL1_0x17 = union {
        uint16_t all;
        struct {
            uint16_t
                RX_RESET : 1, // 0
                RX_LANES : 2, // 1,2
                : 3, // 3,4,5
                PWUP_ADC0 : 1, // 6
                PWUP_ADC1 : 1, // 7
                : 4, // 8,9,10,11
                TIMESTAMP_EN : 1, // 12
                PRBS_ENABLE : 1, // 13
                PRBS_MODE : 1; // 14
        };
    };

    // Регистр управления режимом предварительной синхронизации JESD'ов
    using PRESYNC_0x18 = union {
        uint16_t all;
        struct {
            uint16_t
                START : 1, // Старт предварительной синхронизации
                MODE : 1; // 1 – включен режим предварительной синхронизации
        };
    };

    // Регистры управления цифровыми компараторами
    using COMP_0x224 = union {
        uint16_t all;
        struct {
            uint16_t
                COMP0_EN : 1, // 0 - выкл., 1 - вкл. компаратора на канале 0
                COMP1_EN : 1,
                COMP2_EN : 1,
                COMP3_EN : 1, : 4,
                COMP0_INV : 1, // инверсия порога старта, fasle - возрастание, true - спад, на канале 0
                COMP1_INV : 1,
                COMP2_INV : 1,
                COMP3_INV : 1;
        };
    };
};
