#include <cmath>

#include "INIReader.h"
#include "SrvDAC.h"
#include "sidedriver.h"

#ifndef NDEBUG
#define NDEBUG // отключение любой отладочной информации для этого класса
#endif
#include "log.h"

///
/// \brief      Получение конфигурационных параметров (только для `exam_edac`)
///
/// \param      pDev       pointer to CModule or BaseModule or SubModule
/// \param      pServData  Specific Service Data
/// \param      cmd        pointer to Command Code (from BRD_ctrl())
/// \param      args       Command Arguments (from BRD_ctrl())
///
/// \return     result
///
auto SrvDAC::CtrlGetCfg(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) param = ArgumentCast<BRD_DacCfg>(args);

    // получим размер буфера FIFO
    auto fifo_type = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_FTYPE); // размер шины в битах
    auto fifo_size = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_FSIZE); // размер FIFO в FTYPE'ах
    m_fifo_size = (fifo_type >> 3) * ((fifo_size == 0) ? 1 : fifo_size);

    LOG("BRDctrl_DAC_GETCFG | FifoType: %zd, FifoSize: %zd (Fifo: %zd)",
        size_t(fifo_type), size_t(fifo_size), m_fifo_size);

    // параметры ЦАП
    param.Bits = m_bits; // разрядность
    param.Encoding = m_encoding; // тип кодировки (0 - прямой, 1 - двоично-дополнительный)
    param.MinRate = m_min_rate * 1'000'000; // минимальная частота дискретизации в Гц
    param.MaxRate = m_max_rate * 1'000'000; // максимальная частота дискретизации в Гц
    param.OutRange = m_out_range * 1'000; // выходной диапазон в мВ
    param.FifoSize = m_fifo_size; // размер FIFO АЦП (в байтах)
    param.MaxChan = 12; // максимальное число каналов

    return BRDerr_OK;
}

///
/// \brief      Чтение параметров из `ini` файла и инициализация модуля (только
///             для `exam_edac`). Инициализация модуля необходима только для
///             программ, типа `exam_edac`, которые не отправляют специфические
///             команды для установки каких-либо значений, а используют для
///             этого INI-файлы.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvDAC::CtrlReadIniFile(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) ini = ArgumentCast<BRD_IniFile>(args);
    auto status = uint32_t {};
    // auto ini_filename = BARDY_STR(ini.fileName);
    // auto ini_sectionname = BARDY_STR(ini.sectionName);

    LOG("BRDctrl_DAC_READINIFILE (fileName: %s, sectionName: %s)",
        BARDY_STR(ini.fileName), BARDY_STR(ini.sectionName));

    INIReader reader(BARDY_STR(ini.fileName));
    if (reader.ParseError() < 0) {
        LOG(" - ERROR: Can't load '%s'", BARDY_STR(ini.fileName));
        return BRDerr_ERROR;
    }

    // получаем параметры из INI-файла
    // и производим настройку оборудования
    // без INI-файла необходимо отправлять команды сайд-драйверу
    // -------------------------------------------------------------------------
    CtrlSetDebugInfo(pDev, pServData, BRDctrl_SETDEBUGINFO, (void*)(reader.GetBoolean(BARDY_STR(ini.sectionName), "DebugInfo", false)));

    m_timeout_msec = reader.GetInteger(BARDY_STR(_BRDC("Option")), "TimeoutSec", 5) * 1000;

    // установка параметров стартовой синхронизации
    auto start_mode = BRD_StartMode {};
    start_mode.startSrc = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StartBaseSource", 0)); // Start source
    start_mode.startInv = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StartBaseInverting", 0)); // Start inversion
    start_mode.trigOn = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StartMode", 1)); // Trigger start mode on
    start_mode.trigStopSrc = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StopSource", 0)); // Stop source for trigger start
    start_mode.stopInv = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StopInverting", 0)); // Stop inversion
    start_mode.reStartMode = uint32_t(reader.GetBoolean(BARDY_STR(ini.sectionName), "ReStart", false)); // Restart mode
    CtrlSetStartMode(pDev, pServData, BRDctrl_DAC_SETSTARTMODE, &start_mode);    

    // настройка источника тактовой частоты для АЦП
    auto clk_mode = BRD_ClkMode_FM214x3GDA {};
    clk_mode.src = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "ClockSource", 0x02));
    clk_mode.Fout = reader.GetReal(BARDY_STR(ini.sectionName), "ClockValue", 2'900.0);
    clk_mode.Fosc = reader.GetReal(BARDY_STR(ini.sectionName), "BaseClockValue", sub_module.m_fosc);
    clk_mode.PwrA = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "LMXPowerA", 63)) & 0b11'1111;
    clk_mode.PwrB = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "LMXPowerB", 63)) & 0b11'1111;
    clk_mode.OutA = reader.GetBoolean(BARDY_STR(ini.sectionName), "LMXOutA", true);
    clk_mode.OutB = reader.GetBoolean(BARDY_STR(ini.sectionName), "LMXOutB", true);
    clk_mode.Mult = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "ClockMult", 1));
    status = CtrlSetClkMode(pDev, pServData, BRDctrl_DAC_SETCLKMODE, &clk_mode);
    if (status != BRDerr_OK)
        return BRDerr_ERROR;

    // установка уровня и входного сопротивления внешнего старта
    auto start_level = BRD_StartLevel_FM214x3GDA{};
    start_level.level = reader.GetReal(BARDY_STR(ini.sectionName), "StartLevel", sub_module.m_start_level);
    start_level.resistance = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StartResistance", 0));
    CtrlSetStartLevel(pDev, pServData, BRDctrl_DAC_SETSTARTLEVEL, &start_level);

    // установка уровня и выбор источника сигнала SysRef
    auto sysref = BRD_SysRefConfig_FM214x3GDA{};
    sysref.level = reader.GetReal(BARDY_STR(ini.sectionName), "SysRefLevel", 0.0);
    sysref.select = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "SysRefConfig", 0x00));
    bool ext_sysref = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "SysRefExt", 0x00));
    if( ext_sysref )
        sysref.select = 2;
    CtrlSetSysRef(pDev, pServData, BRDctrl_DAC_SETSYSREF, &sysref);

    // установка маски каналов
    auto chan_mask = reader.GetInteger(BARDY_STR(ini.sectionName), "ChannelMask", 0);
    CtrlSetChanMask(pDev, pServData, BRDctrl_DAC_SETCHANMASK, &chan_mask);

    // Режима работы JESD
    auto jesd_mode = BRD_JESDMode_FM214x3GDA {};
    jesd_mode.mode = size_t(reader.GetInteger(BARDY_STR(ini.sectionName), "JESD_Mode", 0));
    jesd_mode.dual = reader.GetBoolean(BARDY_STR(ini.sectionName), "JESD_Mode_Dual", false);
    jesd_mode.ch_mode = size_t(reader.GetInteger(BARDY_STR(ini.sectionName), "CH_Mode", 1));
    jesd_mode.dp_mode = size_t(reader.GetInteger(BARDY_STR(ini.sectionName), "DP_Mode", 1));
    jesd_mode.ext_core_clk = reader.GetBoolean(BARDY_STR(ini.sectionName), "JESD_CoreClockExt", false); // Признак использования внешней тактовой частоты для CoreClk
    status = CtrlSetJESDMode(pDev, pServData, BRDctrl_DAC_SETJESDMODE, &jesd_mode);
    if (status != BRDerr_OK)
        return BRDerr_ERROR;

    // Калибровочные значение для JESD линков (Delay и Variable Delay Buffer)
    auto jesd_lmfc = BRD_JesdLmfc {};
    jesd_lmfc.delay = size_t(reader.GetInteger(BARDY_STR(ini.sectionName), "LMFCDel", 0));
    jesd_lmfc.variable = size_t(reader.GetInteger(BARDY_STR(ini.sectionName), "LMFCVar", 63));
    CtrlSetLMFC(pDev, pServData, BRDctrl_JESD_SETLMFC, &jesd_lmfc);

    // Значение латентности LMFC Count в тактах CoreCLK
    auto jesd_lmfc_count = BRD_JesdLmfcCount {};
    jesd_lmfc_count.count = size_t(reader.GetInteger(BARDY_STR(ini.sectionName), "LMFCCount", 0));
    CtrlSetLMFCCount(pDev, pServData, BRDctrl_JESD_SETLMFCCOUNT, &jesd_lmfc_count);

    // установка значений кадрового режима
    auto start_delay = BRD_EnVal {};
    start_delay.enable = reader.GetBoolean(BARDY_STR(ini.sectionName), "StartDelayEnable", false);
    start_delay.value = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StartDelayCounter", 0));
    CtrlSetStartDelay(pDev, pServData, BRDctrl_DAC_SETSTDELAY, &start_delay);

    // Значения несущей частоты для DAC0 и DAC1 в Гц
    for (auto i = 0u; i < 2; i++) {
        auto nco = BRD_NCO_FM214x3GDA { i,
            reader.GetReal(BARDY_STR(ini.sectionName), "MainNCO" + std::to_string(i), 0),
            reader.GetReal(BARDY_STR(ini.sectionName), "MainNCO" + std::to_string(i) + "_Phase", 0),
            0 };
        CtrlSetMainNCO(pDev, pServData, BRDctrl_DAC_SETMAINNCO, &nco);
    }

    // Значения несущих частот, фазы и усиления для Channel's[0-5]
    for (auto i = 0u; i < 6; i++) {
        auto nco = BRD_NCO_FM214x3GDA {
            i,
            reader.GetReal(BARDY_STR(ini.sectionName), "ChannelNCO" + std::to_string(i), 0), // в Гц
            reader.GetReal(BARDY_STR(ini.sectionName), "ChannelNCO" + std::to_string(i) + "_Phase", 0), // в градусах
            reader.GetReal(BARDY_STR(ini.sectionName), "ChannelGain" + std::to_string(i), 0), // в дБ
        };
        CtrlSetChannelNCO(pDev, pServData, BRDctrl_DAC_SETCHANNELNCO, &nco);
    }

    // Включение калибровки выходных каналов
    auto calibration = reader.GetBoolean(BARDY_STR(ini.sectionName), "Calibration", false);
    if (calibration == true)
        CtrlCalibration(pDev, pServData, BRDctrl_DAC_CALIBRATION, nullptr);

    // Включение/Отключение синтезатора (DDS)
    auto dds_enable = reader.GetBoolean(BARDY_STR(ini.sectionName), "DDSEnable", false);
    if (dds_enable) {
        // Значения амплитуд для каналов
        for (auto i = 0u; i < 6; i++) {
            auto ampl = BRD_AMPL_FM214x3GDA { i, uint16_t(reader.GetInteger(BARDY_STR(ini.sectionName), "DDSAmpl" + std::to_string(i), 0x50FF)) };
            CtrlSetDDSAmpl(pDev, pServData, BRDctrl_DAC_SETDDSAMPL, &ampl);
        }
        // Включим DDS
        CtrlDDSEnable(pDev, pServData, BRDctrl_DAC_DDSENABLE, &dds_enable);
    }

    // Включение/Отключение режима PreSync
    auto presync = BRD_PreSync_FM214x3GDA {};
    presync.enable = reader.GetBoolean(BARDY_STR(ini.sectionName), "PreSync", false);
    presync.type = (reader.Get(BARDY_STR(ini.sectionName), "PreSyncType", "sync") == "sync") ? BRD_PRESYNC_TYPE_FM214x3GDA::SYNC : BRD_PRESYNC_TYPE_FM214x3GDA::ASYNC;
    CtrlSetPreSync(pDev, pServData, BRDctrl_DAC_SETPRESYNC, &presync);

    // установка стандартной задержки для IDELAYE3 (UltraScale)
    auto std_delay = BRD_StdDelay {};
    for (auto i = 0U; i < 16; i++)
        if (reader.HasValue(BARDY_STR(ini.sectionName), "StdDelay" + std::to_string(i))) {
            std_delay.nodeID = i;
            std_delay.value = reader.GetInteger(BARDY_STR(ini.sectionName), "StdDelay" + std::to_string(i), 0);
            CtrlSetDelay(pDev, pServData, BRDctrl_DAC_WRDELAY, &std_delay);
        }

    // !!! только для DEV-Board ... для нашей платы удалить !!!
    // auto spi = reader.GetBoolean(BARDY_STR(ini.sectionName), "SPI_FMC", true);
    // auto ctrl = CONTROL1_VALUE {};
    // ctrl.all = sub_module.IndRegRead(m_tetr_num, CONTROL1);
    // ctrl.SPI_FMC = spi;
    // sub_module.IndRegWrite(m_tetr_num, CONTROL1, ctrl.all);
    // LOG("SPI_FMC = %s", spi ? "enable" : "disable");

    return BRDerr_OK;
}

///
/// \brief      Получение формата каналов (только для `exam_edac`). Регистр
///             ADM2IFnr_FORMAT [0x12] Выбор формата данных.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvDAC::CtrlGetFormat(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) format = ArgumentCast<ADM2IF_FORMAT>(args);

    // uint32_t val {};
    // IndRegRead(module, mTetrNum, ADM2IFnr_FORMAT, &val);
    // LOG(INF("BRDctrl_DAC_GETFORMAT [ADM2IFnr_FORMAT]: %08Xh"), val);

    format.AsWhole = 0;
    format.ByBits.Pack = std::ceil(m_bits / 8.0); // в байтах
    // format.ByBits.Code = 0; // IQ код
    // format.ByBits.Align = 0; // align to high-order bit
    LOG("BRDctrl_DAC_GETFORMAT | %d bits", format.ByBits.Pack * 8);

    return BRDerr_OK;
}

///
/// \brief      Получение коэффициента интерполяции сигнала (только для
///             `exam_edac`).
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvDAC::CtrlGetInterpFactor(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) interp = ArgumentCast<uint32_t>(args);

    interp = 1;
    LOG("BRDctrl_DAC_GETINTERP | %d", interp);

    return BRDerr_OK;
}

///
/// \brief      Получение SYNC параметров (только для `exam_edac`).
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvDAC::CtrlGetSyncMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) sync_mode = ArgumentCast<BRD_SyncMode>(args);

    // uint32_t val {};
    // IndRegRead(module, mTetrNum, ADM2IFnr_FMODE, &val);
    // LOG(INF("BRDctrl_DAC_GETSYNCMODE | ADM2IFnr_FMODE: %08Xh"), val);

    constexpr auto BRDclks_SMCLK = 7; // Submodule clock
    sync_mode.clkSrc = BRDclks_SMCLK; // источник тактовой частоты находится на субмодуле
    // sync_mode.clkSrc = BRDclks_EXTCLK; // внешний источник тактовой частоты

    //  частота тактового генератора в Гц
    sync_mode.clkValue = 0.0; // для BRDclks_SMCLK

    // частота дискретизации в Гц (SampleRate)
    sync_mode.rate = m_dac.get_sample_rate() * 1'000'000;

    return BRDerr_OK;
}

///
/// \brief      Установка режима работы тетрады "мастер/single" (только для
///             `exam_edac`).
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvDAC::CtrlSetMaster(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) value = ArgumentCast<uint32_t>(args);

    LOG("BRDctrl_DAC_SETMASTER | %d", value);

    // Бит Master = 1 - работа тетрады в режиме SINGLE
    auto mode = ADM2IF_MODE0 {};
    mode.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_MODE0);
    mode.ByBits.Master = value;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_MODE0, mode.AsWhole);

    return BRDerr_OK;
}

///
/// \brief      Возвращает буфер трассировки в виде текста (только для
///             `exam_edac`). Нужен для отображения дополнительной информации
///             при выводе счётчика циклов.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvDAC::CtrlGetTraceText(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) textbuf = ArgumentCast<BRD_TraceText>(args);

    BRDC_snprintf(textbuf.pText, textbuf.sizeb,
        _BRDC(" JESD: GRP=%02X FRM=%02X SUM=%02X INI=%02X | GRP=%02X FRM=%02X SUM=%02X INI=%02X | 0x%08X 0x%08X "),
        jesd_status.first[0], jesd_status.first[1], jesd_status.first[2], jesd_status.first[3],
        jesd_status.second[0], jesd_status.second[1], jesd_status.second[2], jesd_status.second[3],
        jesd_status_fpga.first, jesd_status_fpga.second);

    return BRDerr_OK;
}
