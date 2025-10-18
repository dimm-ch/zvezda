#include <bitset>
#include <cmath>

#include "INIReader.h"
#include "SrvADC.h"
#include "brd_string.h"
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
auto SrvADC::CtrlGetCfg(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm414x3g>(pDev);
    decltype(auto) param = ArgumentCast<BRD_AdcCfg>(args);

    // параметры АЦП
    param.Bits = m_adc0.is_complex() ? 16 : m_bits; // разрядность: ADC = 14, DDC = 16 бит
    param.Encoding = m_encoding; // тип кодировки (0 - прямой, 1 - двоично-дополнительный)
    param.MinRate = m_min_rate * 1'000'000; // минимальная частота дискретизации в Гц
    param.MaxRate = m_max_rate * 1'000'000; // максимальная частота дискретизации в Гц
    param.InpRange = m_inp_range * 1'000; // выходной диапазон в мВ
    param.FifoSize = m_fifo_size; // размер FIFO АЦП (в байтах)
    param.NumChans = 4; // число каналов
    param.ChanType = 0; // тип канала (1 - RF (gain in dB))

    return BRDerr_OK;
}

///
/// \brief      Чтение параметров из `ini` файла и инициализация модуля (только
///             для `exam_edac`). Инициализация модуля необходима только для
///             программ, типа `exam_adc`, которые не отправляют специфические
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
auto SrvADC::CtrlReadIniFile(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm414x3g>(pDev);
    decltype(auto) ini = ArgumentCast<BRD_IniFile>(args);
    auto status = uint32_t {};

    LOG("MFM414x3G:BRDctrl_ADC_READINIFILE { fileName: %s, sectionName: %s }",
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
    auto debug_info = reader.GetBoolean(BARDY_STR(ini.sectionName), "DebugInfo", false);
    CtrlSetDebugInfo(pDev, pServData, BRDctrl_SETDEBUGINFO, &debug_info);

    m_timeout_msec = reader.GetInteger(BARDY_STR(_BRDC("Option")), "TimeoutSec", 5) * 1000;

    // настройка источника тактовой частоты для АЦП
    auto clk_mode = BRD_ClkMode_FM414x3G {};
    clk_mode.src = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "ClockSource", 0x02));
    clk_mode.Fout = reader.GetReal(BARDY_STR(ini.sectionName), "ClockValue", 2'900.0);
    clk_mode.Fosc = reader.GetReal(BARDY_STR(ini.sectionName), "BaseClockValue", sub_module.m_fosc);
    clk_mode.PwrA = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "LMXPowerA", 63)) & 0b11'1111;
    clk_mode.PwrB = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "LMXPowerB", 63)) & 0b11'1111;
    clk_mode.OutA = reader.GetBoolean(BARDY_STR(ini.sectionName), "LMXOutA", true);
    clk_mode.OutB = reader.GetBoolean(BARDY_STR(ini.sectionName), "LMXOutB", true);
    status = CtrlSetClkMode(pDev, pServData, BRDctrl_ADC_SETCLKMODE, &clk_mode);
    if (status != BRDerr_OK) {
        m_global_error = true;
        return BRDerr_ERROR;
    }

    // установка параметров стартовой синхронизации
    auto start_mode = BRD_StartMode {};
    start_mode.startSrc = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StartBaseSource", 0)); // Start source
    start_mode.startInv = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StartBaseInverting", 0)); // Start inversion
    start_mode.trigOn = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StartMode", 1)); // Trigger start mode on
    start_mode.trigStopSrc = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StopSource", 0)); // Stop source for trigger start
    start_mode.stopInv = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StopInverting", 0)); // Stop inversion
    start_mode.reStartMode = uint32_t(reader.GetBoolean(BARDY_STR(ini.sectionName), "ReStart", false)); // Restart mode
    CtrlSetStartMode(pDev, pServData, BRDctrl_ADC_SETSTARTMODE, &start_mode);

    // установка уровня и входного сопротивления внешнего старта
    auto start_level = BRD_StartLevel_FM414x3G {};
    start_level.level = reader.GetReal(BARDY_STR(ini.sectionName), "StartLevel", sub_module.m_start_level);
    start_level.resistance = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StartResistance", 0));
    CtrlSetStartLevel(pDev, pServData, BRDctrl_ADC_SETSTARTLEVEL, &start_level);

    // установка уровня и выбор источника сигнала SysRef
    auto sysref = BRD_SysRefConfig_FM414x3G{};
    sysref.level = reader.GetReal(BARDY_STR(ini.sectionName), "SysRefLevel", 0.0);
    sysref.select = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "SysRefConfig", 0x00));
    bool ext_sysref = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "SysRefExt", 0x00));
    if( ext_sysref )
        sysref.select = 2;
    CtrlSetSysRef(pDev, pServData, BRDctrl_ADC_SETSYSREF, &sysref);

    // установка clock delay для АЦП каналов (задержка принимаемого сигнала)
    auto clock_delay_control = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "ClockDelayControl", 0));
    CtrlSetClockDelayControl(pDev, pServData, BRDctrl_ADC_SETCLOCKDELAYCONTROL, &clock_delay_control);
    auto clock_delay = BRD_ClockDelayChannel_FM414x3G {};
    for (auto i = 0U; i < 4; i++) {
        clock_delay.channel = i;
        clock_delay.divider_phase = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "ClockDividerPhase" + std::to_string(i), 0));
        clock_delay.superfine_delay = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "ClockSuperfineDelay" + std::to_string(i), 0));
        clock_delay.fine_delay = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "ClockFineDelay" + std::to_string(i), 0xC0));
        CtrlSetClockDelayChannel(pDev, pServData, BRDctrl_DDC_SETFC, &clock_delay);
    }

    // установка режима АЦП
    auto adc_mode = BRD_AdcMode_FM414x3G {};
    if (reader.GetBoolean(BARDY_STR(ini.sectionName), "DDCEnable", false)) {
        // auto chan_mask = 0;
        // CtrlSetChanMask(pDev, pServData, BRDctrl_ADC_SETCHANMASK, &chan_mask);

        // DDC параметры
        adc_mode.ddc_decimate = reader.GetInteger(BARDY_STR(ini.sectionName), "DDCDecimation", 2);
        adc_mode.channel_mask = reader.GetInteger(BARDY_STR(ini.sectionName), "DDCChannelMask", 0x01);
        // adc_mode.ddc_mixer_select = reader.GetInteger(BARDY_STR(ini.sectionName), "DDCMixerSelect", 0);

        switch (adc_mode.channel_mask) {
        case 0x01:
        case 0x11:
            adc_mode.app_mode = 0b0001;
            break; // One DDC
        case 0x03:
        case 0x33:
            adc_mode.app_mode = 0b0010;
            break; // Two DDC
        case 0x0F:
        case 0xFF:
            adc_mode.app_mode = 0b0011;
            break; // Four DDC
        }

    } else {
        adc_mode.app_mode = 0b0000; // ADC Full Bandwidth
        adc_mode.channel_mask = reader.GetInteger(BARDY_STR(ini.sectionName), "ChannelMask", 0x03);
    }
    CtrlSetMode(pDev, pServData, BRDctrl_ADC_SETMODE, &adc_mode);

    // Настройка/запуск JESD'ов
    auto ext_core_clk = reader.GetBoolean(BARDY_STR(ini.sectionName), "JESD_CoreClockExt", false); // Признак использования внешней тактовой частоты для CoreClk
    status = CtrlSetJesd(pDev, pServData, BRDctrl_ADC_SETJESD, &ext_core_clk);
    if (status != BRDerr_OK) {
        m_global_error = true;
        return BRDerr_ERROR;
    }

    // установка значений кадрового режима
    auto start_delay = BRD_EnVal {};
    start_delay.enable = reader.GetBoolean(BARDY_STR(ini.sectionName), "StartDelayEnable", false);
    start_delay.value = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "StartDelayCounter", 0));
    CtrlSetStartDelay(pDev, pServData, BRDctrl_ADC_SETSTDELAY, &start_delay);

    auto acq_count = BRD_EnVal {};
    acq_count.enable = reader.GetBoolean(BARDY_STR(ini.sectionName), "AcquiredSampleEnable", false);
    acq_count.value = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "AcquiredSampleCounter", 0));
    CtrlSetAcqCount(pDev, pServData, BRDctrl_ADC_SETACQDATA, &acq_count);

    auto skip_count = BRD_EnVal {};
    skip_count.enable = reader.GetBoolean(BARDY_STR(ini.sectionName), "SkipSampleEnable", false);
    skip_count.value = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "SkipSampleCounter", 0));
    CtrlSetSkipCount(pDev, pServData, BRDctrl_ADC_SETSKIPDATA, &skip_count);

    // установка значений заголовков кадрового режима
    auto title_mode = BRD_EnVal {};
    title_mode.enable = reader.GetBoolean(BARDY_STR(ini.sectionName), "TitleEnable", false);
    title_mode.value = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "TitleSize", 0));
    CtrlSetTitleMode(pDev, pServData, BRDctrl_ADC_SETTITLEMODE, &title_mode);
    auto title_data = uint32_t(reader.GetReal(BARDY_STR(ini.sectionName), "TitleData", 0));
    CtrlSetTitleData(pDev, pServData, BRDctrl_ADC_SETTITLEDATA, &title_data);

    // Установка NCO и Phase для DDC 0..7
    auto freq = BRD_ValChan {};
    for (auto i = 0U; i < 8; i++) {
        freq.chan = i;
        freq.value = reader.GetReal(BARDY_STR(ini.sectionName), "DDCFc" + std::to_string(i), 725.0);
        CtrlSetDDCFc(pDev, pServData, BRDctrl_DDC_SETFC, &freq);
        freq.value = reader.GetReal(BARDY_STR(ini.sectionName), "DDCFc_Phase" + std::to_string(i), 0.0);
        CtrlSetDDCFcPhase(pDev, pServData, BRDctrl_DDC_SETFCPHASE, &freq);
    }

    // Подключение каналов АЦП к входам DDC IQ
    auto input = BRD_DdcInputIQ_FM414x3G {};
    for (auto i = 0U; i < 8; i++) {
        input.channel = i;
        input.src_i = reader.GetReal(BARDY_STR(ini.sectionName), "DDCInputI" + std::to_string(i), 0);
        input.src_q = reader.GetReal(BARDY_STR(ini.sectionName), "DDCInputQ" + std::to_string(i), 0);
        status = CtrlSetDDCInputSource(pDev, pServData, BRDctrl_DDC_SETINPSRC, &input);
    }

    // Параметр Invert Channel A
    if (reader.GetBoolean(BARDY_STR(ini.sectionName), "InvertChannelA", false)) {
        CtrlSetInvertChannelA(pDev, pServData, BRDctrl_ADC_SETINVERTCHANNELA, nullptr);
    }

    // Параметр Input Full-Scale Voltage для каналов
    // auto = reader.GetReal(BARDY_STR(ini.sectionName), "DDCInputI" + std::to_string(i), 0);
    auto voltage = BRD_ValChan {};
    for (auto i = 0U; i < 4; i++) {
        voltage.chan = i;
        voltage.value = reader.GetReal(BARDY_STR(ini.sectionName), "InputFSVoltageCh" + std::to_string(i), 1.7);
        CtrlSetInputFSVoltage(pDev, pServData, BRDctrl_ADC_SETINPUTFSVOLTAGE, &voltage);
    }

    // Параметр Input Buffer Control для каналов
    auto buf = BRD_ValChan {};
    for (auto i = 0U; i < 4; i++) {
        buf.chan = i;
        buf.value = reader.GetInteger(BARDY_STR(ini.sectionName), "InputBufferControlCh" + std::to_string(i), 0);
        CtrlSetInputBufferControl(pDev, pServData, BRDctrl_ADC_SETINPUTBUFFERCONTROL, &buf);
    }

    // Параметры Fast Detect'а для каналов АЦП
    auto fd = BRD_FastDetect_FM414x3G {};
    for (auto i = 0U; i < 4; i++) {
        fd.channel = i;
        fd.enable = reader.GetBoolean(BARDY_STR(ini.sectionName), "FDControl" + std::to_string(i), false);
        fd.threshold_upper = reader.GetReal(BARDY_STR(ini.sectionName), "FDUpper" + std::to_string(i), 0.0);
        fd.threshold_lower = reader.GetReal(BARDY_STR(ini.sectionName), "FDLower" + std::to_string(i), 0.0);
        fd.dwell_time = uint16_t(reader.GetInteger(BARDY_STR(ini.sectionName), "FDDwellTime" + std::to_string(i), 1));
        CtrlSetFastDetectChannel(pDev, pServData, BRDctrl_ADC_SETFASTDETECTCHANNEL, &fd);
    }

    // Калибровочные значения для JESD линков
    auto jesd_lmfc_count = BRD_JesdLmfcCount {};
    jesd_lmfc_count.count = size_t(reader.GetInteger(BARDY_STR(ini.sectionName), "LMFCCount", 0));
    CtrlSetLMFCCount(pDev, pServData, BRDctrl_JESD_SETLMFCCOUNT, &jesd_lmfc_count);

    // Включение/Отключение режима предварительной синхронизации JESD'ов
    auto presync = BRD_PreSync_FM414x3G {};
    presync.enable = reader.GetBoolean(BARDY_STR(ini.sectionName), "PreSync", false);
    status = CtrlSetPresync(pDev, pServData, BRDctrl_ADC_SETPRESYNC, &presync);
    if (status != BRDerr_OK)
        return BRDerr_ERROR;

    // Включение/Отключение режима PreTrigger'а
    auto pretrigger = BRD_PretrigMode {};
    auto pretrigger_mode = size_t(reader.GetInteger(BARDY_STR(ini.sectionName), "PreTriggerMode", 0));
    if (pretrigger_mode != 0) {
        pretrigger.enable = true;
        pretrigger.assur = (pretrigger_mode == 2) ? 1 : 0;
        pretrigger.external = (pretrigger_mode == 3) ? 1 : 0;
        pretrigger.size = uint32_t(reader.GetInteger(BARDY_STR(ini.sectionName), "PreTriggerSamples", 16));
        status = CtrlSetPretrigMode(pDev, pServData, BRDctrl_ADC_SETPRETRIGMODE, &pretrigger);
        if (status != BRDerr_OK)
            return BRDerr_ERROR;
    }

    // Калибровочные значения для JESD линков
    auto prbs_type_str = reader.Get(BARDY_STR(ini.sectionName), "PrbsType", "");
    PRBS_TYPE prbs_type = PRBS_TYPE::NONE;
    if (prbs_type_str == "PN9")
        prbs_type = PRBS_TYPE::PN9;
    if (prbs_type_str == "PN23")
        prbs_type = PRBS_TYPE::PN23;
    CtrlSetJESDPrbs(pDev, pServData, BRDctrl_JESD_SETPRBS, &prbs_type);

    // Включение/Отключение режима временных меток TimeStamp в потоке
    auto timestamp = BRD_TimeStamp_FM414x3G {};
    timestamp.enable = reader.GetBoolean(BARDY_STR(ini.sectionName), "TimeStamp", false);
    timestamp.polarity_negative = reader.GetBoolean(BARDY_STR(ini.sectionName), "TimeStamp_PolarityNegative", false);
    status = CtrlSetTimeStamp(pDev, pServData, BRDctrl_ADC_SETTIMESTAMP, &timestamp);
    if (status != BRDerr_OK)
        return BRDerr_ERROR;

    // Обработка параметров для проекта "Криотрейд"
    if (m_cryo) {
        auto param = BRD_ValChan {};
        for (auto i = 0U; i < 4; i++) {
            param.chan = i;
            param.value = reader.GetReal(BARDY_STR(ini.sectionName), "CompLevel" + std::to_string(i), 0.0);
            CtrlSetCompLevel(pDev, pServData, BRDctrl_ADC_SETCOMPLEVEL, &param);
            param.value = reader.GetBoolean(BARDY_STR(ini.sectionName), "CompEnable" + std::to_string(i), false);
            CtrlSetCompEnable(pDev, pServData, BRDctrl_ADC_SETCOMPENABLE, &param);
            param.value = reader.GetBoolean(BARDY_STR(ini.sectionName), "CompInverse" + std::to_string(i), false);
            CtrlSetCompInverse(pDev, pServData, BRDctrl_ADC_SETCOMPINVERSE, &param);
        }
    }

    // замена потока ADC на тестовую последовательность
    auto test_seq = reader.GetBoolean(BARDY_STR(ini.sectionName), "TestSequence", false);
    CtrlSetTestSequence(pDev, pServData, BRDctrl_ADC_SETTESTSEQUENCE, &test_seq);

    // установка стандартной задержки для IDELAYE3 (UltraScale)
    auto std_delay = BRD_StdDelay {};
    for (auto i = 0U; i < 16; i++)
        if (reader.HasValue(BARDY_STR(ini.sectionName), "StdDelay" + std::to_string(i))) {
            std_delay.nodeID = i;
            std_delay.value = reader.GetInteger(BARDY_STR(ini.sectionName), "StdDelay" + std::to_string(i), 0);
            CtrlSetDelay(pDev, pServData, BRDctrl_ADC_WRDELAY, &std_delay);
        }

    return BRDerr_OK;
}

///
/// \brief      Получение SYNC параметров (только для `exam_adc`).
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlGetSyncMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm414x3g>(pDev);
    decltype(auto) sync_mode = ArgumentCast<BRD_SyncMode>(args);

    // uint32_t val {};
    // IndRegRead(module, mTetrNum, ADM2IFnr_FMODE, &val);
    // LOG(INF("BRDctrl_ADC_GETSYNCMODE | ADM2IFnr_FMODE: %08Xh"), val);

    constexpr auto BRDclks_SMCLK = 7; // Submodule clock
    sync_mode.clkSrc = BRDclks_SMCLK; // источник тактовой частоты находится на субмодуле
    // sync_mode->clkSrc = BRDclks_EXTCLK; // внешний источник тактовой частоты

    //  частота тактового генератора в Гц
    sync_mode.clkValue = 0.0; // для BRDclks_SMCLK

    // частота дискретизации в Гц (SampleRate)
    sync_mode.rate = m_adc0.get_sample_rate() * 1'000'000;

    return BRDerr_OK;
}

///
/// \brief      Получение параметров стартовой синхронизации (только для
///             `exam_edac`).
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlGetStartMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm414x3g>(pDev);
    decltype(auto) start_mode = ArgumentCast<BRD_StartMode>(args);

    // есть режим старта от субмодуля и от несущей платы
    // читаем Board Mode Register[ADM2IFnr_MODE0] в тетраде ADC
    // mode0 = (ADM2IF_MODE0) REGREADIND(ADM2IFnr_MODE0)
    // смотрим бит mode0.Master, если не выставлен,
    // то режим старта от несущей платы ...

    // выбрав плату, читаем Start Mode Register[ADM2IFnr_STMODE]
    // stmode = (PADM2IF_STMODE) REGREADIND(ADM2IFnr_STMODE)
    // режимы SrcStart и SrcStop в файле `ctrladmpro.h` BRDsts_...

    // пример кода:
    //   файл: \BardyLITE\brdlib\src\adcsrv.cpp
    //   функция: CAdcSrv::GetStartMode()

    //    3                   2                   1                   0
    //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    // ┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
    // │                               │R│I│ │ SrcStop │T│I│ │ SrcStart│
    // └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
    //                     Restart Mode ┘ │             │ └ Inverting Start Signal
    //              Inverting Stop Signal ┘             └ Trigger Start Mode
    //

    // в нашем случае читаем режим старта в несущей плате
    // if(pMode0->ByBits.Master) // Single
    auto val = ADM2IF_STMODE {};
    val.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_STMODE);
    LOG("BRDctrl_ADC_GETSTARTMODE | ADM2IFnr_STMODE: %08Xh", val.AsWhole);

    start_mode.startSrc = val.ByBits.SrcStart; // select of start signal
    start_mode.trigStopSrc = val.ByBits.SrcStop; // select of stop signal
    start_mode.startInv = val.ByBits.InvStart; // inverting start signal
    start_mode.stopInv = val.ByBits.InvStop; // inverting stop signal
    start_mode.trigOn = val.ByBits.TrigStart; // trigger start mode
    start_mode.reStartMode = val.ByBits.Restart; // Restart mode

    return BRDerr_OK;
}

///
/// `exam_adc` Получение значений задержки стартовой синхронизации
///
/// \brief Регистр TRES [0x103] Ресурсы тетрады
///        бит 11 STD_DELAY, если 1, то тетрада поддерживает узел
///        управления задержками (регистр STD_DELAY  по адресу 0x2F0)
///
auto SrvADC::CtrlReadDelay(void* pDev, void* /*pServData*/, ULONG /*cmd*/, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm414x3g>(pDev);
    decltype(auto) std_delay = ArgumentCast<BRD_StdDelay>(args);

    // пример кода:
    //   файл: \BardyLITE\brdlib\src\adcsrv.cpp
    //   функция: CAdcSrv::StdDelay()

    // uint32_t val {};
    // IndRegRead(module, mTetrNum, ADM2IFnr_TRES, &val);
    // if(param.val & 0x800) {
    //     ... работа с регистром STD_DELAY ...
    //     return BRDerr_OK;
    // } else return BRDerr_CMD_UNSUPPORTED;

    LOG("BRDctrl_ADC_RDDELAY | UNSUPPORTED");
    return BRDerr_CMD_UNSUPPORTED;
}

///
/// \brief      Получение формата каналов (только для `exam_adc`). Регистр
///             ADM2IFnr_FORMAT [0x12] Выбор формата данных.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlGetFormat(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm414x3g>(pDev);
    decltype(auto) format = ArgumentCast<ADM2IF_FORMAT>(args);

    // uint32_t val {};
    // IndRegRead(module, mTetrNum, ADM2IFnr_FORMAT, &val);
    // LOG(INF("BRDctrl_ADC_GETFORMAT [ADM2IFnr_FORMAT]: %08Xh"), val);

    format.AsWhole = 0;
    format.ByBits.Pack = std::ceil(m_bits / 8.0); // в байтах
    // format.ByBits.Code = 0; // двоично дополнительный код
    // format.ByBits.Align = 0; // align to high-order bit
    //LOG("BRDctrl_ADC_GETFORMAT | %d bits", format.ByBits.Pack * 8);

    return BRDerr_OK;
}

///
/// `exam_adc` Получение входного диапазона канала/ов
///
auto SrvADC::CtrlGetInpRange(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm414x3g>(pDev);
    decltype(auto) val_chan = ArgumentCast<BRD_ValChan>(args);

    val_chan.value = m_inp_range;

    LOG("BRDctrl_ADC_GETINPRANGE CH:%d Range:%f", val_chan.chan, val_chan.value);
    return BRDerr_OK;
}

///
/// `exam_adc` Получение смещения нуля для каналов
///
auto SrvADC::CtrlGetBias(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm414x3g>(pDev);
    decltype(auto) val_chan = ArgumentCast<BRD_ValChan>(args);

    // TODO: значение нужно брать из общей конфигурации CtrlGetCfg()
    // по умолчанию 0.0
    val_chan.value = 0.0; // cfg->Bias = 0

    LOG("BRDctrl_ADC_GETBIAS CH:%d Range:%f", val_chan.chan, val_chan.value);
    return BRDerr_OK;
}

///
/// \brief      Команда вызывается перед запуском процесса сбора данных (только
///             для `exam_adc`). Здесь мы анализируем глобальную переменную
///             ошибок, которая взводится при не правильной инициализации модуля
///             и возвращаем ошибку, при необходимости. `exam-adc` анализирует
///             результат этой функции и не запустит сбор данных.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlPrepareStart(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    // TODO: можно удалить, т.к. уже есть отлов ошибок.
    if (m_global_error)
        return BRDerr_BAD_PARAMETER;

    return BRDerr_OK;
}

///
/// \brief      Возвращает буфер трассировки в виде текста (только для
///             `exam_adc`). Нужен для отображения дополнительной информации
///             при выводе счётчика циклов.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlGetTraceText(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm414x3g>(pDev);
    decltype(auto) textbuf = ArgumentCast<BRD_TraceText>(args);
    auto data = sub_module.m_monitor.get_data();

    // Регистр состояния сигналов Fast Detect от АЦП
    auto adc_ovr = std::bitset<4>(sub_module.IndRegRead(m_tetr_num, 0x208)).to_string();
    BRDC_snprintf(textbuf.pText, textbuf.sizeb, _BRDC(" %.02f\xF8 V6:%.02f V7:%.02f V8:%.02f ADC_OVR:%s"),
        data.Tint, data.V6, data.V7 * 2, data.V8 * 3, InSys::to_brd_string(adc_ovr).c_str());

    return BRDerr_OK;
}

///
/// \brief      Возвращает признак комплексного сигнала IQ (только для
///             `exam_adc`).
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlIsComplex(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm414x3g>(pDev);
    decltype(auto) is_complex = ArgumentCast<uint32_t>(args);

    if (m_adc0.is_complex())
        is_complex = 1; // для DDC
    else
        is_complex = 0; // для ADC

    return BRDerr_OK;
}

/*
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
auto SrvADC::CtrlGetInterpFactor(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto sub_module = *static_cast<Cmfm414x3g*>(pDev);
    auto interp = static_cast<uint32_t*>(args);

    *interp = 1;
    LOG("BRDctrl_ADC_GETINTERP | %d", *interp);

    return BRDerr_OK;
}
*/

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
auto SrvADC::CtrlSetMaster(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm414x3g>(pDev);
    decltype(auto) value = ArgumentCast<uint32_t>(args);

    LOG("BRDctrl_ADC_SETMASTER | %d", value);

    // Бит Master = 1 - работа тетрады в режиме SINGLE
    auto mode = ADM2IF_MODE0 {};
    mode.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_MODE0);
    mode.ByBits.Master = value;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_MODE0, mode.AsWhole);

    return BRDerr_OK;
}
