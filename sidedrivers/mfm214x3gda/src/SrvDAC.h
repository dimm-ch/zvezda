///
/// \file AD9176.h
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief
/// \version 0.1
/// \date 20.05.2019
///
/// \copyright InSys Copyright (c) 2019
///

#pragma once
#include "AD9176.h"
#include "mfm214x3gda.h"
#include "service.h"

class Cmfm214x3gda; // класс родителя

// Служба ЦАП
class SrvDAC : public CService {
    friend class Cmfm214x3gda;
    static constexpr auto name_class = "DAC"; // имя класса для отладочных сообщений

    // порядковый номер и ID DAC тетрады !!! 0x113 Бинго :-) !!!
    ssize_t m_tetr_num = -1;
    static constexpr size_t DAC_ID = 0x113;
    size_t m_fifo_size = 0; // размер буфера FIFO
    size_t m_fifo_type = 0; // размер шины FIFO в битах
    size_t m_jesd_phy_id = 0; // идентификатор JESD PHY
    size_t m_timeout_msec = 5000;
    AD9176 m_dac; // ЦАП AD9176

    // параметры ЦАП со значениями по умолчанию
    size_t m_bits = 16; // разрядность ЦАП (16 бит)
    size_t m_encoding = 1; // тип кодировки (1 - двоично-дополнительный)
    double m_min_rate = 2.91; // минимальная частота дискретизации (2.91 МГц)
    double m_max_rate = 12.6; // максимальная частота дискретизации (12.6 МГц)
    double m_out_range = 0.35; // выходной диапазон (0.35 В)

    std::pair<std::array<uint8_t, 4>, std::array<uint8_t, 4>> jesd_status {}; // статистика по JESD'ам в ЦАП
    std::pair<uint32_t, uint32_t> jesd_status_fpga {}; // статистика по JESD'ам в ПЛИС

public:
    SrvDAC(Cmfm214x3gda* parent);

    // общие команды для всех служб/сервисов
    auto CtrlIsAvailable(void* pDev, void* pServData, ULONG cmd, void* args) -> int override;
    auto CtrlCapture(void* pDev, void* pServData, ULONG cmd, void* args) -> int override;
    auto CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args) -> int override;
    auto CtrlGetAddrData(void* pDev, void* pServData, ULONG cmd, void* args) -> int override;

    // уникальные команды службы DAC только для этого модуля
    auto CtrlSetDebugInfo(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetMainNCO(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlDDSEnable(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetDDSAmpl(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetChannelNCO(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetJESDMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlCalibration(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetLMFC(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetLMFC(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetStartLevel(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetSysRef(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetLMFCCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetLMFCCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetPreSync(void* pDev, void* pServData, ULONG cmd, void* args) -> int;

    // общие команды службы DAC
    auto CtrlGetSrcStream(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetSource(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetCyclMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlEnable(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlFifoReset(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlPutData(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlFifoStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetClkMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetChanMask(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetChanMask(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetStartMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetStartMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetStartDelay(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetCyclingMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetCyclingMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetDelay(void* pDev, void* pServData, ULONG cmd, void* args) -> int;

    // команды службы DAC, необходимые только для `exam_edac`
    auto CtrlGetCfg(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlReadIniFile(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetFormat(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetInterpFactor(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetSyncMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlSetMaster(void* pDev, void* pServData, ULONG cmd, void* args) -> int;
    auto CtrlGetTraceText(void* pDev, void* pServData, ULONG cmd, void* args) -> int;

    // -------------------------------------------------------------------------
    // Регистры
    // -------------------------------------------------------------------------
    // #define DAC_REG(name, addr, ...) static constexpr auto name = addr; using name##_VALUE = union { uint16_t all; struct { uint16_t __VA_ARGS__ ;};}

    // Control1
    // DAC_REG(CONTROL1, 0x17, TX_RESET:1, JESD_MODE:5, DUAL_LINK:1, :1, TX0_PD:1, TX1_PD:1, DAC_RESET:1);

    using CONTROL1_0x17 = union {
        uint16_t all;
        struct {
            uint16_t
                TX_RESET : 1,
                JESD_MODE : 5, DUAL_LINK : 1, : 1, TX0_PD : 1, TX1_PD : 1, DAC_RESET : 1;
        };
    };

    using PRESYNC_0x18 = union {
        uint16_t all;
        struct {
            uint16_t
                START : 1,
                MODE : 1;
        };
    };
};
