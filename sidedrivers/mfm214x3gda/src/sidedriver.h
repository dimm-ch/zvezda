///
/// \file sidedriver.h
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief
/// \version 0.1
/// \date 20.05.2019
///
/// \copyright InSys Copyright (c) 2019
///

#pragma once
#include <algorithm>
#include <cstdio>
#include <string>
#include <thread>

#include <boost/interprocess/sync/file_lock.hpp>

#define SIDE_CLASS Cmfm214x3gda
#define VER_MAJOR 0x0001'0000L
#define VER_MINOR 0x0000'0000L
#define DEBUG_REGCTRL // вывод расшифровки команд `DEVScmd_*`
#define DEBUG_SRVCTRL // вывод расшифровки команд `SERVcmd_*` и `BRDctrl_*`

#include "adm2if.h"
#include "basemodule.h"
#include "icr.h"
#include "module.h"

#include "LMX2594.h"
#include "SrvADC.h"
#include "SrvDAC.h"

class Cmfm214x3gda : public CModule {
    static constexpr auto info = "SideDriver for module FM214x3GDA v2.0 (InSys 2020 | Alexander U.Jurankov)";
    static constexpr auto name_class = "SDRV"; // имя класса для отладочных сообщений

    friend class SrvADC;
    friend class SrvDAC;

    size_t m_type = 0xFF; // тип субмодуля
    size_t m_version = 0xFF; // версия субмодуля

    // параметры субмодуля со значениями по умолчанию
    static constexpr uint16_t SUBMODULE_CFG_TAG = 0x00A9;
    size_t m_adc_count = 2; // количество АЦП
    size_t m_dac_count = 2; // количество ЦАП
    double m_fosc = 200.0; // частота осцилятора на субмодуле в МГц
    double m_start_level = 5.0; // шкала преобразования ЦАП-ИПН в вольтах (порог внешнего старта)
    double m_adc_min_bitrate = 1'687.0; // АЦП минимальный битрейт в Mbps
    double m_adc_max_bitrate = 16'000.0; // АЦП максимальный битрейт в Mbps
    double m_dac_min_bitrate = 3'000.0; // ЦАП минимальный битрейт в Mbps
    double m_dac_max_bitrate = 15'500.0; // ЦАП максимальный битрейт в Mbps
    bool m_pll_present = true; // признак присутствия микросхемы PLL
    bool m_adc_present = true; // признак присутствия микросхемы АЦП
    bool m_dac_present = true; // признак присутствия микросхемы ЦАП

    // информация о базовой/несущей плате
    CBaseModule* base_dev = nullptr; // объект базового модуля (несущей платы)
    DEVS_API_entry* base_entry = nullptr; // DEVS API Entry Point (точка входа для отправления команд модулю)
    BRDCHAR base_name[BOARDNAME_SIZE]; // название базового модуля

    // порядковые номера, ID нужных тетрад, микросхемы этих тетрад
    ssize_t m_sync_tetr_num = -1;
    static constexpr size_t SYNC_ID = 0x117; // SYNC тетрада

    // механизм `SYNC SHARED` для совместой работы ЦАП и АЦП
    std::string m_sync_fname_lock {}; // `./CAMBPEX_1314_lock.tmp` путь к файлу блокировки
    std::string m_sync_fname_data {}; // `./CAMBPEX_1314_data.tmp` путь к файлу с информацией
    boost::interprocess::file_lock m_sync_lock {};
    bool m_sync_slave = false; // признак работы текущего процесса в режиме "ведомого"

    // общие микросхемы поддержки
    LMX2594 m_lmx; // Синтезатор LMX2594
    double m_lmx_Fout; // Выходная частота LMX
    double m_jesd_coreclk; // Тактовая частота узла JESD в ПЛИС
    double m_jesd_refclk; // Опорная частота узла JESD в ПЛИС

    // сервисы/службы субмодуля
    SrvDAC m_srv_dac { this }; // сервис DAC
    SrvADC m_srv_adc { this }; // сервис ADC

    // приватные функции
    auto dump_icr(uint8_t* mem, size_t len) -> void; // дамп на консоль байтов из памяти EEPROM
    auto analysis_eeprom(uint8_t const* eeprom) -> void; // анализирование данных EEPROM

public:
    Cmfm214x3gda() = default;
    Cmfm214x3gda(PBRD_InitData pBrdInitData, long sizeInitData);
    ~Cmfm214x3gda()
    {
        if (!m_sync_slave)
            m_sync_lock.unlock();
    }

    auto SideDriverInit() -> int; // инициализация модуля
    auto FindID(size_t id, size_t id_num) -> ssize_t; // поиск тетрады по ID

    // вызывается из `SIDE_initAuto`
    auto Init(uint8_t* pSubmodEEPROM, BRDCHAR* pIniString = nullptr) -> int32_t;

    auto BaseInfoInit(void* pDev, DEVS_API_entry* pEntry, BRDCHAR* pName) -> void; // <-- CMD_init
    auto GetServ(SERV_ListItem srv_list[]) -> void; // <-- CMD_getServList
    auto GetDeviceInfo(BRD_Info* pInfo) -> void; // <-- CMD_getinfo
    auto SetSrvList(SERV_ListItem srv_list[]) -> void; // <-- CMD_setServList

    // заглушки для процессорных команд
    auto GetPuList(BRD_PuList* list) -> void { } // <-- CMD_puList
    auto PuFileLoad(uint32_t id, BRDCHAR* fileName, uint32_t* pState) { return SUBERR(BRDerr_OK); } // <-- CMD_puLoad
    auto GetPuState(uint32_t id, uint32_t* pState) { return SUBERR(BRDerr_OK); } // <-- CMD_puState

    // команды модуля и служб
    auto RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam) -> LONG;
    auto SrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, void* pContext) -> ULONG;

    // ??? не уверен, что кто-то вызывает эту функцию
    auto getClassName(void) { return _BRDC("Cmfm214x3gda"); }

    // работа с прямыми/косвенными регистрами и с устройствами на шине SPD
    auto IndRegRead(size_t tetr_num, size_t reg_num) -> size_t;
    auto IndRegWrite(size_t tetr_num, size_t reg_num, size_t val) -> void;
    auto DirRegRead(size_t tetr_num, size_t reg_num) -> size_t;
    auto DirRegWrite(size_t tetr_num, size_t reg_num, size_t val) -> void;
    auto DirRegWrites(size_t tetr_num, size_t reg_num, void* buf, size_t size) -> void;
    auto SpdRead(size_t tetr_num, size_t dev, size_t numb, size_t addr) -> size_t;
    auto SpdWrite(size_t tetr_num, size_t dev, size_t numb, size_t addr, size_t val) -> void;

    // общие микросхемы тетрады SYNC
    auto SetClkMode(size_t src, double Fout, double Fosc, size_t PwrA = 63, size_t PwrB = 63, bool OutA = true, bool OutB = true) -> bool;
    auto SetJesdCoreAndRefCLK(size_t cmd, double lane_rate, bool ext_core_clk = false) -> bool;
    auto SetJesdQPLL(size_t cmd) -> bool;
    auto SetJesdCPLL(size_t cmd) -> bool;
    auto SetStartLevel(double level, size_t resistance) -> void;
    auto SetSysRefDAC(double level, size_t select) -> void;
    auto SetSysRefADC(double level, size_t select) -> void;

    using CONTROL1_0x17 = union {
        uint16_t all;
        struct {
            uint16_t : 4, SYSREF_ADC_CNF : 2, SYSREF_DAC_CNF : 2, : 2, START_INPR : 1, EXT_JESD_CORECLK : 1;
        };
    };
};
