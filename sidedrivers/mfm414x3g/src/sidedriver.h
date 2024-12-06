///
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief
///

#pragma once
#include <algorithm>
#include <cstdio>

#define SIDE_CLASS Cmfm414x3g
#define VER_MAJOR 0x0001'0000L
#define VER_MINOR 0x0000'0000L
#define DEBUG_REGCTRL // вывод расшифровки команд `DEVScmd_*`
#define DEBUG_SRVCTRL // вывод расшифровки команд `SERVcmd_*` и `BRDctrl_*`

#include "adm2if.h"
#include "basemodule.h"
#include "icr.h"
#include "module.h"

#include "LMX2594.h"
#include "LTC2991.h"
#include "SrvADC.h"

class Cmfm414x3g : public CModule {
    static constexpr auto info = "SideDriver for module FM414x3G v2.0 (InSys 2020 | Alexander U.Jurankov)";
    static constexpr auto name_class = "SDRV"; // имя класса для отладочных сообщений

    friend class SrvADC;

    size_t m_type = 0xFF; // тип субмодуля
    size_t m_version = 0xFF; // версия субмодуля

    // параметры субмодуля со значениями по умолчанию
    static constexpr uint16_t SUBMODULE_CFG_TAG = 0x00AA;
    size_t m_adc_count = 4; // количество АЦП
    double m_fosc = 200.0; // частота осцилятора на субмодуле в МГц
    double m_start_level = 5.0; // шкала преобразования ЦАП-ИПН в вольтах (порог внешнего старта)
    double m_adc_min_bitrate = 1'687.0; // АЦП минимальный битрейт в Mbps
    double m_adc_max_bitrate = 16'000.0; // АЦП максимальный битрейт в Mbps
    bool m_pll_present = true; // признак присутствия микросхемы PLL
    bool m_adc_present = true; // признак присутствия микросхемы АЦП

    // информация о базовой/несущей плате
    CBaseModule* base_dev = nullptr; // объект базового модуля (несущей платы)
    DEVS_API_entry* base_entry = nullptr; // DEVS API Entry Point (точка входа для отправления команд модулю)
    BRDCHAR base_name[BOARDNAME_SIZE]; // название базового модуля

    // порядковые номера, ID нужных тетрад, микросхемы этих тетрад
    int m_sync_tetr_num = -1;
    static constexpr size_t SYNC_ID = 0x11A; // SYNC тетрада

    // общие микросхемы поддержки
    LMX2594 m_lmx; // Синтезатор LMX2594
    LTC2991 m_monitor; // Монитор напряжений и температуры LTC2991

    // сервисы/службы субмодуля
    SrvADC m_srv_adc { this }; // сервис ADC

    // приватные функции
    auto dump_icr(uint8_t* mem, size_t len) -> void; // дамп на консоль байтов из памяти EEPROM
    auto analysis_eeprom(uint8_t const* eeprom) -> void; // анализирование данных EEPROM

public:
    Cmfm414x3g() = default;
    Cmfm414x3g(PBRD_InitData pBrdInitData, long sizeInitData);
    ~Cmfm414x3g() = default;

    auto SideDriverInit() -> int; // инициализация модуля
    auto FindID(size_t id, size_t id_num) -> int; // поиск тетрады по ID

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
    auto getClassName(void) { return _BRDC("Cmfm414x3g"); }

    // работа с прямыми/косвенными регистрами и с устройствами на шине SPD
    auto IndRegRead(size_t tetr_num, size_t reg_num) -> size_t;
    auto IndRegWrite(size_t tetr_num, size_t reg_num, size_t val) -> void;
    auto DirRegRead(size_t tetr_num, size_t reg_num) -> size_t;
    auto DirRegWrite(size_t tetr_num, size_t reg_num, size_t val) -> void;
    auto DirRegWrites(size_t tetr_num, size_t reg_num, void* buf, size_t size) -> void;
    auto SpdRead(size_t tetr_num, size_t dev, size_t numb, size_t addr) -> size_t;
    auto SpdWrite(size_t tetr_num, size_t dev, size_t numb, size_t addr, size_t val) -> void;

    // общие микросхемы тетрады SYNC
    auto SetClkMode(size_t src, double Fout, double Fosc,
        size_t PwrA = 63, size_t PwrB = 63, bool OutA = true, bool OutB = true) -> bool;
    auto SetStartLevel(double level, size_t resistance) -> void;
    auto SetSysRef(double level, size_t select) -> void;
    auto setup_jesd_cpll(double lane_rate) -> void;
    auto setup_jesd_qpll0(double lane_rate) -> void;

    using CONTROL1_0x17 = union {
        uint16_t all;
        struct {
            uint16_t : 4, SYSREF_ADC_CNF : 2, : 4, START_INPR : 1, EXT_JESD_CORECLK : 1;
        };
    };

    // CPLL DRP Registers for Kintex UltraScale
    using DRP_0x63 = union {
        size_t all;
        struct {
            size_t
                RXOUT_DIV : 3;
        };
    };
    using DRP_0x28 = union {
        size_t all;
        struct {
            size_t : 8,
                CPLL_FBDIV : 8,
                CPLL_FBDIV_45 : 1;
        };
    };
    using DRP_0x2A = union {
        size_t all;
        struct {
            size_t : 11,
                CPLL_REFCLK_DIV : 5;
        };
    };
};
