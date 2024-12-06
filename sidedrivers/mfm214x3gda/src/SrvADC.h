///
/// \file AD9176.h
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief
/// \version 0.1
/// \date 24.06.2019
///
/// \copyright InSys Copyright (c) 2019
///

#pragma once
#include "AD9208.h"
#include "mfm214x3gda.h"
#include "service.h"

class Cmfm214x3gda; // класс родителя

// Служба АЦП
class SrvADC : public CService {
    friend class Cmfm214x3gda;
    static constexpr auto name_class = "ADC"; // имя класса для отладочных сообщений

    // порядковый номер и ID DAC тетрады
    ssize_t m_tetr_num = -1;
    static constexpr size_t ADC_ID = 0x118;
    size_t m_fifo_size = 0; // размер буфера FIFO
    size_t m_fifo_type = 0; // размер шины FIFO в битах
    size_t m_jesd_phy_id = 0; // идентификатор JESD PHY
    size_t m_timeout_msec = 5000;
    AD9208 m_adc; // АЦП0 AD9208

    // параметры АЦП со значениями по умолчанию
    size_t m_bits = 14; // разрядность АЦП (14 бит) или DDC (16 бит)
    size_t m_encoding = 1; // тип кодировки (1 - двоично-дополнительный)
    double m_min_rate = 2.5; // минимальная частота дискретизации (2.5 МГц)
    double m_max_rate = 3.1; // максимальная частота дискретизации (3.1 МГц)
    double m_inp_range = 0.5; // выходной диапазон (0.5 В)

public:
    SrvADC(Cmfm214x3gda* parent);

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
    auto CtrlSetPreSync(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetTimeStamp(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetJESDLinkErrorStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetJESDSyncStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetJESDDebugStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetJESDPrbs(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetJESDPrbsStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int;

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
    auto CtrlGetStartMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
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
    auto CtrlReadDelay(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetFormat(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetInpRange(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetBias(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlIsComplex(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    // auto CtrlGetInterpFactor(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetMaster(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetTraceText(void* pDev, void* pServData, ULONG cmd, void* args) -> int;

    using CONTROL1_0x17 = union {
        uint16_t all;
        struct {
            uint16_t
                RX_RESET : 1,
                RX_LANES : 2, : 3,
                PWUP_ADC : 1, : 5,
                TIMESTAMP_EN : 1,
                PRBS_ENABLE : 1,
                PRBS_MODE : 1;
        };
    };

    // Предварительная синхронизация JESD интерфейсов
    using PRESYNC_0x18 = union {
        uint16_t all;
        struct {
            uint16_t
                START : 1,
                MODE : 1;
        };
    };
};
