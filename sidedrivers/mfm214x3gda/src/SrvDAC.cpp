#include <bitset>
#include <thread>

#include "SrvDAC.h"
#include "sidedriver.h"

// #ifndef NDEBUG
// #define NDEBUG // отключение любой отладочной информации для этого класса
// #endif
#include "log.h"

///
/// Конструктор
///
SrvDAC::SrvDAC(Cmfm214x3gda* parent)
    : CService(-1, _BRDC("DAC214x3GDA"), -1, parent, nullptr, 0)
{
    // LOG("ctor (parent: %p) -> this: %p", parent, this);

    m_dac.setup_spi(
        [this, parent](size_t reg) -> uint8_t { return parent->SpdRead(m_tetr_num, 0, 0, reg); },
        [this, parent](size_t reg, uint8_t val) -> void { parent->SpdWrite(m_tetr_num, 0, 0, reg, val); });

    // атрибуты службы
    m_attribute = BRDserv_ATTR_DIRECTION_OUT | BRDserv_ATTR_STREAMABLE_OUT | BRDserv_ATTR_SDRAMABLE;

    // добавление команд службы
    m_Cmd.push_back(new CMD_Info { BRDctrl_SETDEBUGINFO, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetDebugInfo) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_SETMAINNCO, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetMainNCO) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_DDSENABLE, 0, 0, 0, CmdEntry(&SrvDAC::CtrlDDSEnable) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_SETDDSAMPL, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetDDSAmpl) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_SETCHANNELNCO, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetChannelNCO) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_SETJESDMODE, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetJESDMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_CALIBRATION, 0, 0, 0, CmdEntry(&SrvDAC::CtrlCalibration) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_SETSTARTLEVEL, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetStartLevel) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_SETSYSREF, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetSysRef) });

    // общие команды службы
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_GETSRCSTREAM, 0, 0, 0, CmdEntry(&SrvDAC::CtrlGetSrcStream) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_SETSOURCE, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetSource) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_SETCYCLMODE, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetCyclingMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_ENABLE, 0, 0, 0, CmdEntry(&SrvDAC::CtrlEnable) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_FIFORESET, 0, 0, 0, CmdEntry(&SrvDAC::CtrlFifoReset) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_PUTDATA, 0, 0, 0, CmdEntry(&SrvDAC::CtrlPutData) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_FIFOSTATUS, 0, 0, 0, CmdEntry(&SrvDAC::CtrlFifoStatus) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_SETCHANMASK, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetChanMask) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_GETCHANMASK, 0, 0, 0, CmdEntry(&SrvDAC::CtrlGetChanMask) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_SETCLKMODE, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetClkMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_SETSTARTMODE, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetStartMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_GETSTARTMODE, 0, 0, 0, CmdEntry(&SrvDAC::CtrlGetStartMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_SETSTDELAY, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetStartDelay) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_WRDELAY, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetDelay) });

    m_Cmd.push_back(new CMD_Info { BRDctrl_JESD_SETLMFCCOUNT, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetLMFCCount) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_JESD_GETLMFCCOUNT, 0, 0, 0, CmdEntry(&SrvDAC::CtrlGetLMFCCount) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_JESD_SETLMFC, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetLMFC) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_JESD_GETLMFC, 0, 0, 0, CmdEntry(&SrvDAC::CtrlGetLMFC) });

    // команды службы, необходимые только для `exam_edac`
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_GETCFG, 0, 0, 0, CmdEntry(&SrvDAC::CtrlGetCfg) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_READINIFILE, 0, 0, 0, CmdEntry(&SrvDAC::CtrlReadIniFile) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_GETFORMAT, 0, 0, 0, CmdEntry(&SrvDAC::CtrlGetFormat) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_GETINTERP, 0, 0, 0, CmdEntry(&SrvDAC::CtrlGetInterpFactor) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_GETSYNCMODE, 0, 0, 0, CmdEntry(&SrvDAC::CtrlGetSyncMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_GETSTARTMODE, 0, 0, 0, CmdEntry(&SrvDAC::CtrlGetStartMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DAC_SETMASTER, 0, 0, 0, CmdEntry(&SrvDAC::CtrlSetMaster) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_GETTRACETEXT, 0, 0, 0, CmdEntry(&SrvDAC::CtrlGetTraceText) });
}

///
/// \brief      Установка признака вывода отладочных сообщений.
///
/// \param      pDev       The dev
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvDAC::CtrlSetDebugInfo(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    debug_info = (bool)(args) ? true : false;
    return BRDerr_OK;
};

///
/// \brief      Проверка доступности службы и её подготовка к работе. При вызове
///             BRD_capture() и BRD_serviceList() -> Brd_Serv_List() происходит
///             проверка доступности службы.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvDAC::CtrlIsAvailable(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) param = ArgumentCast<SERV_CMD_IsAvailable>(args);
    param.isAvailable = false;

    // проверка на повторную инициализацию
    if (m_isAvailable) {
        param.isAvailable = m_isAvailable;
        param.attr = m_attribute;
        return BRDerr_OK;
    }

    // LOG("SERVcmd_SYS_ISAVAILABLE (pDev: %p, pServData: %p, cmd: %zu, args: %p)",
    //     pDev, pServData, size_t(cmd), args);

    // инициализация модуля
    if (sub_module.SideDriverInit() == -1)
        return BRDerr_ERROR;

    // проверим доступность SYNC тетрады
    // службу нельзя захватывать, если нет SYNC тетрады
    if (sub_module.m_sync_tetr_num == -1)
        return BRDerr_ERROR;

    // ищем порядковый номер DAC тетрады
    // службу нельзя захватывать, если нет такой тетрады
    m_tetr_num = sub_module.FindID(DAC_ID, 1);
    if (m_tetr_num == -1)
        return BRDerr_ERROR;

    // сделаем службу доступной для захвата
    m_isAvailable = true;
    param.isAvailable = m_isAvailable;
    param.attr = m_attribute;

    return BRDerr_OK;
}

///
/// \brief      Захват службы
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     результат работы
///
auto SrvDAC::CtrlCapture(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);

    // не будем захватывать не доступную службу
    if (m_tetr_num == -1)
        return BRDerr_ERROR;

    // Reset тетрады
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_MODE0, 1);
    std::this_thread::sleep_for(100ms);
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_MODE0, 0);

    // Очистим теневые регистры [0x01 .. 0x1F]
    for (auto reg_num = 0x01; reg_num < 0x20; reg_num++)
        sub_module.IndRegWrite(m_tetr_num, reg_num, 0x00);

    // получим размер буфера FIFO
    auto fifo_size = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_FSIZE); // размер FIFO в FTYPE'ах
    m_fifo_type = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_FTYPE); // размер шины в битах
    m_fifo_size = (m_fifo_type >> 3) * ((fifo_size == 0) ? 1 : fifo_size);
    // LOG("FifoType: %zu, FifoSize: %zu (Fifo: %zu)", m_fifo_type, fifo_size, m_fifo_size);

    return BRDerr_OK;
}

///
/// Освобождение службы
///
auto SrvDAC::CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    return BRDerr_CMD_UNSUPPORTED; // for free subservice
}

///
/// Получение порядкового номера тетрады
///
/// \brief Необходимо стриму для установки тетрады источника данных
///        Вызывается командой `SERVcmd_SYS_GETADDRDATA`
///
auto SrvDAC::CtrlGetAddrData(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto tetrada = static_cast<uint32_t*>(args);

    *tetrada = m_tetr_num;
    return BRDerr_OK;
}

///
/// Получение порядкового номера тетрады
///
/// \brief Необходимо стриму для установки тетрады источника данных
///        Вызывается командой `BRDctrl_DAC_GETSRCSTREAM`
///
auto SrvDAC::CtrlGetSrcStream(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto tetrada = static_cast<uint32_t*>(args);

    *tetrada = m_tetr_num;
    return BRDerr_OK;
}

///
/// Выбор исходника для данных (FIFO или SDRAM)
///
///
auto SrvDAC::CtrlSetSource(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto source = *static_cast<uint32_t*>(args);

    LOG("BRDctrl_DAC_SETSOURCE %d", source);

    auto mode = ADM2IF_MODE1 {};
    mode.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_MODE1);
    mode.ByBits.Out = source;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_MODE1, mode.AsWhole);

    return BRDerr_OK;
}

///
/// Установка режима циклического вывода данных из FIFO и SDRAM
///
auto SrvDAC::CtrlSetCyclingMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) param = ArgumentCast<bool>(args);

    auto mode = ADM2IF_MODE0 {};
    mode.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_MODE0);
    mode.ByBits.Cycle = param;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_MODE0, mode.AsWhole);

    // LOG("BRDctrl_DAC_SETCYCLMODE | MODE0: %04X, Cycling Mode: %s", mode.AsWhole, param ? "enable" : "disable");
    return BRDerr_OK;
}

///
/// Сброс FIFO
///
auto SrvDAC::CtrlFifoReset(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);

    auto mode = ADM2IF_MODE0 {};
    mode.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_MODE0);
    mode.ByBits.FifoRes = 1;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_MODE0, mode.AsWhole); // FIFO Reset
    this_thread::sleep_for(200us);
    mode.ByBits.FifoRes = 0;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_MODE0, mode.AsWhole); // End FIFO Reset
    this_thread::sleep_for(200us);

    return BRDerr_OK;
}

///
/// Получить статус FIFO
///
auto SrvDAC::CtrlFifoStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto status = static_cast<uint32_t*>(args);

    *status = sub_module.DirRegRead(m_tetr_num, ADM2IFnr_STATUS);

    return BRDerr_OK;
}

///
/// Загрузить данные в FIFO
///
auto SrvDAC::CtrlPutData(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto data = *static_cast<BRD_DataBuf*>(args);

    LOG("BRDctrl_DAC_PUTDATA | %ph, %d", data.pData, data.size);

    // пишем массив данных в прямой регистр DATA (FIFO тетрады)
    sub_module.DirRegWrites(m_tetr_num, ADM2IFnr_DATA, data.pData, data.size);

    // !!! DEBUG !!!
    // auto f = fopen("data_fifo.bin", "wb+");
    // fwrite(data.pData, data.size, 1, f);
    // fclose(f);

    return BRDerr_OK;
}

///
/// Активация/деактивация ЦАП
///
auto SrvDAC::CtrlEnable(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto enable = *static_cast<uint32_t*>(args);

    // получим статистику по JESD'ам перед отключением передачи данных
    if (enable == false) {
        jesd_status = m_dac.jesd_get_links_status();
        jesd_status_fpga = std::make_pair<uint32_t, uint32_t>(
            sub_module.SpdRead(m_tetr_num, 8, 0, (m_jesd_phy_id == 0x15) ? 0x060 : 0x038),
            sub_module.SpdRead(m_tetr_num, 9, 0, (m_jesd_phy_id == 0x15) ? 0x060 : 0x038));
    }

    // программный старт/останов тетрады
    ADM2IF_MODE0 mode {};
    mode.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_MODE0);
    mode.ByBits.Start = enable;
    mode.ByBits.Master = true;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_MODE0, mode.AsWhole);

    // LOG("BRDctrl_DAC_ENABLE | %s, 0x%X", enable ? "enabel" : "disable", mode.AsWhole);
    return BRDerr_OK;
}

///
/// \brief      Установка маски используемых каналов.  Регистр ADM2IFnr_CHAN
///             [0x10] Выбор каналов биты [7..0] MASK.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     result
///
auto SrvDAC::CtrlSetChanMask(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = *static_cast<uint32_t*>(args);

    auto chan1 = ADM2IF_CHAN1 {};
    chan1.ByBits.ChanSel = value;

    LOG("BRDctrl_DAC_SETCHANMASK | %04Xh", value);

    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_CHAN1, chan1.AsWhole);
    return BRDerr_OK;
}

///
/// \brief      Получение маски каналов. Регистр ADM2IFnr_CHAN [0x10] Выбор
///             каналов биты [7..0] MASK.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     result
///
auto SrvDAC::CtrlGetChanMask(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto chan_mask = static_cast<uint32_t*>(args);

    auto chan1 = ADM2IF_CHAN1 {};
    chan1.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_CHAN1);
    *chan_mask = chan1.ByBits.ChanSel;

    // auto chan = std::bitset<32>(sub_module.IndRegRead(m_tetr_num, ADM2IFnr_CHAN1));
    // auto chan_iq = std::bitset<32>();
    // if (chan.test(0)) chan_iq.set(0), chan_iq.set(1);
    // if (chan.test(1)) chan_iq.set(2), chan_iq.set(3);
    // *chan_mask = chan_iq.to_ulong();

    LOG("BRDctrl_DAC_GETCHANMASK | %04Xh", *chan_mask);

    return BRDerr_OK;
}

///
/// \brief      Установка и инициализация источника тактовой частоты для ЦАП.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       Адрес структуры `BRD_ClkMode`
///
/// \return     result
///
auto SrvDAC::CtrlSetClkMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto& clk_mode = *static_cast<BRD_ClkMode_FM214x3GDA*>(args);
    auto ctrl = CONTROL1_0x17 {};

    // Схема тактирования АЦП
    // -------------------------------------------------------------------------
    //
    // +-----------+             +----------+
    // | Ситезатор |  14500 МГц  |   ЦАП    |
    // |    LMX    +------------->  AD9176  |
    // |           |   максимум  |          |
    // +-----------+             +----------+
    //

    // Инициализация LMX2594 [ SYNC ]
    // -------------------------------------------------------------------------
    // TODO: проверить на уже запущеный LMX
    //       если система тактирования модуля уже была настроена для
    //       другой службы, используем выставленные значения
    auto result = sub_module.SetClkMode(clk_mode.src, clk_mode.Fout, clk_mode.Fosc,
        clk_mode.PwrA, clk_mode.PwrB, clk_mode.OutA, clk_mode.OutB);
    if (result == false)
        return BRDerr_ERROR;
    LOG("DAC input: %.02f MHz", sub_module.m_lmx_Fout);

    // AD9176
    // -------------------------------------------------------------------------
    ctrl.DAC_RESET = 1;
    sub_module.IndRegWrite(m_tetr_num, 0x17, ctrl.all);
    ctrl.DAC_RESET = 0;
    sub_module.IndRegWrite(m_tetr_num, 0x17, ctrl.all);
    m_dac.reset();
    m_dac.get_chip_id();
    m_dac.set_fclk(sub_module.m_lmx_Fout, clk_mode.Mult);
    if (m_dac.pll_setup() == false)
        return BRDerr_ERROR;
    // m_dac.digital_gain_setup();
    m_dac.enable_tx();

    // LTC2991 | Инициализация монитора напряжений и температур
    // -------------------------------------------------------------------------
    // sub_module.m_monitor.reset();

    return BRDerr_OK;
}

auto SrvDAC::CtrlSetMainNCO(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = *static_cast<BRD_NCO_FM214x3GDA*>(args);

    // SampleRate = 11'796'480'000
    // DDSM_FTW = ( Fcarrier / Fnco,clk ) * 2^48
    // Fcarrier is the output frequency of the NCO = 530'000'000 Hz
    // Fnco,clk is the sampling clock frequency of the NCO = 11'796'480'000 Hz

    // auto DDSM_FTW0 = uint64_t(round((chans.ch0 / 11'796'480'000.) * std::pow(2,48)));
    // auto DDSM_FTW1 = uint64_t(round((chans.ch1 / 11'796'480'000.) * std::pow(2,48)));
    // LOG("BRDctrl_DAC_SETFINALNCO | ch0 = %.0f Hz | DDSM_FTW: %012llXh", chans.ch0, DDSM_FTW0);
    // LOG("BRDctrl_DAC_SETFINALNCO | ch1 = %.0f Hz | DDSM_FTW: %012llXh", chans.ch1, DDSM_FTW1);
    // m_dac.nco_main_setup(DDSM_FTW0, DDSM_FTW1);

    LOG("BRDctrl_DAC_SETMAINNCO | ch%zd = %.0f Hz, phase = %.0f", value.num_chan, value.freq, value.phase);
    m_dac.nco_main_setup(value.num_chan, value.freq, value.phase);

    return BRDerr_OK;
}

auto SrvDAC::CtrlDDSEnable(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = *static_cast<size_t*>(args);

    LOG("BRDctrl_DAC_DDSENABLE | %s", value ? "true" : "false");
    m_dac.dds_enable(value);

    return BRDerr_OK;
}

auto SrvDAC::CtrlSetDDSAmpl(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = *static_cast<BRD_AMPL_FM214x3GDA*>(args);

    LOG("BRDctrl_DAC_SETDDSAMPL | ch%zd = %04Xh", value.num_chan, value.ampl);
    m_dac.dds_amplitude(value.num_chan, value.ampl);

    return BRDerr_OK;
}

auto SrvDAC::CtrlSetChannelNCO(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = *static_cast<BRD_NCO_FM214x3GDA*>(args);

    LOG("BRDctrl_DAC_SETCHANNELNCO | ch%zd = %.0f Hz, phase = %.0f, gain = %.0f", value.num_chan, value.freq, value.phase, value.gain);
    m_dac.nco_channel_setup(value.num_chan, value.freq, value.phase);
    m_dac.set_digital_gain(value.num_chan, value.gain);

    return BRDerr_OK;
}

auto SrvDAC::CtrlSetJESDMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = *static_cast<BRD_JESDMode_FM214x3GDA*>(args);

    // Схема тактирования JESD'ов
    // -----------------------------------------------------------------------------
    //                                             EXT CORE CLK <--+
    //                                                             |    +----------+
    // +-----------+   +---------------+             +----------+  |  +-> JESD АЦП |
    // | Ситезатор |   | Делитель HMC  |   1600 МГц  | Делитель |  |  | +----------+
    // |    LMX    +---> регистр FDVR2 +------------->   LMK    +--+--+
    // |           |   |               |   максимум  |          |     | +----------+
    // +-----------+   +---------------+             +----------+     +-> JESD ЦАП |
    //                                                                  +----------+

    // Настройка JESD'ов в ЦАП
    // -------------------------------------------------------------------------
    LOG("BRDctrl_DAC_SETJESDMODE | %s %zd x%zd x%zd %s",
        value.dual ? "Dual" : "Single", value.mode, value.ch_mode, value.dp_mode, value.ext_core_clk ? "Using External Core Clock" : "");
    if (m_dac.jesd_mode_setup(value.dual, value.mode, value.ch_mode, value.dp_mode) == false)
        return BRDerr_ERROR;

    // Установка CoreCLK и RefCLK для JESD'ов в ПЛИС
    // -------------------------------------------------------------------------
    if (sub_module.SetJesdCoreAndRefCLK(cmd, m_dac.get_lane_rate(), value.ext_core_clk) == false)
        return BRDerr_ERROR;

    // JESD FPGA PHY's QPLL/CPLL GTX Common
    // -------------------------------------------------------------------------
    // получим идентификатор JESD PHY, используемый в ПЛИС
    m_jesd_phy_id = sub_module.IndRegRead(m_tetr_num, 0x110);
    LOG("JESD Phy ID: %s [%02zXh]", m_jesd_phy_id == 0x13 ? "CPLL" : "QPLL", m_jesd_phy_id);
    bool result = false;
    switch (m_jesd_phy_id) {
    case 0x13:
        result = sub_module.SetJesdCPLL(cmd);
        break;
    case 0x0F: // QPLL JESD204B
    case 0x15: // QPLL JESD204C
        result = sub_module.SetJesdQPLL(cmd);
        break;
    default:
        LOG("Unknow JESD Phy ID");
        break;
    }
    if (result == false)
        return BRDerr_ERROR;

    // JESD FPGA передатчики
    // -------------------------------------------------------------------------
    auto ctrl = CONTROL1_0x17 {};
    ctrl.all = sub_module.IndRegRead(m_tetr_num, 0x17);
    if (sub_module.m_sync_slave == false)
        ctrl.TX_RESET = 1; // взведём reset, если модуль в режиме MASTER
    sub_module.IndRegWrite(m_tetr_num, 0x17, ctrl.all);
    ctrl.TX_RESET = 0; // отпустим reset
    ctrl.JESD_MODE = value.mode;
    ctrl.DUAL_LINK = value.dual;
    sub_module.IndRegWrite(m_tetr_num, 0x17, ctrl.all);
    LOG("CONTROL1: 0x%08zX", sub_module.IndRegRead(m_tetr_num, 0x17));
    std::this_thread::sleep_for(200ms);

    auto jesd_info = m_dac.jesd_get_info(); // получим информацию о JESD
    auto L_mask = std::bitset<8>(); // переведём количество линий в биты
    for (size_t i = 0; i < jesd_info.L; i++)
        L_mask.set(i);

    // SPD 8 - JESD204B link 0
    // SPD 9 - JESD204B link 1 <-- Не участвует в Single режиме
    for (auto link = 0; link < (ctrl.DUAL_LINK ? 2 : 1); link++) {
        auto spd_dev = 8 + link; // номер устройства на шине SPD
        if (m_jesd_phy_id == 0x15) {
            // параметры JESD204C
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x024, 0b10); // Enables the AXI4-Stream Data interface
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x034, 0x01); // subclass 1 mode
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x040, L_mask.to_ullong()); // lanes in use
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x050, 0x01); // SysRef Handling
            auto reg_03C = sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x03C);
            reg_03C |= 1 << 16; // screambling on
            reg_03C |= jesd_info.F - 1; // Octets per Frame - 1
            reg_03C |= (jesd_info.K - 1) << 8; // Frames per MultiFrame - 1
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x03C, reg_03C);

            LOG("JESD link %d: HEX 020=%02zX, 024=%02zX, 034=%02zX, 03C=%04zX, 040=%02zX, 050=%04zX", spd_dev,
                sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x020), sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x024),
                sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x034), sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x03C),
                sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x040), sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x050));

            // Reset (изменения в регистрах фиксируются после ресета)
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x020, 1);
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x020, 0);
            std::this_thread::sleep_for(20ms);
            if ((sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x020) & 1) != 0) {
                LOG("JESD No Reset");
                return BRDerr_ERROR;
            }
        } else {
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x004, 0x10001); // disable watchdog timer & reset
            std::this_thread::sleep_for(20ms);
            if ((sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x004) & 1) != 0) {
                LOG("JESD No Reset");
                return BRDerr_ERROR;
            }

            // параметры JESD204B
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x00C, 0x01); // screambling on
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x02C, 0x01); // subclass mode
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x028, L_mask.to_ullong()); // lanes in use
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x020, jesd_info.F - 1); // Octets per Frame - 1
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x024, jesd_info.K - 1); // Frames per MultiFrame - 1
            sub_module.SpdWrite(m_tetr_num, spd_dev, 0, 0x010, 0x01); // SysREF handling: No SysREF is required on a Re-Sync, Core aligns LMFC counter on all SysREF

            LOG("JESD link %d: HEX 004=%02zX, 00C=%02zX, 010=%02zX, 020=%02zX, 024=%02zX, 028=%02zX, 02Ch=%02zX", link,
                sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x004), sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x00C),
                sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x010), sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x020),
                sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x024), sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x028),
                sub_module.SpdRead(m_tetr_num, spd_dev, 0, 0x02C));
        }
    }

    // JESD Status
    if (m_dac.jesd_serdes_setup(0xFF, true) == false)
        return BRDerr_ERROR;
    // m_dac.jesd_crossbar_setup(); // после Reset'а устанавливается прямой порядок линий
    if (m_dac.jesd_links_enable() == false)
        return BRDerr_ERROR;

    // Без вторичного ресета не всегда происходит синхронизация JESD'ов
    ctrl.all = sub_module.IndRegRead(m_tetr_num, 0x17);
    if (sub_module.m_sync_slave == false)
        ctrl.TX_RESET = 1; // взведём reset, если модуль в режиме MASTER
    sub_module.IndRegWrite(m_tetr_num, 0x17, ctrl.all);
    ctrl.TX_RESET = 0; // отпустим reset
    sub_module.IndRegWrite(m_tetr_num, 0x17, ctrl.all);

    return BRDerr_OK;
}

auto SrvDAC::CtrlCalibration(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);

    m_dac.enable_calibration();

    return BRDerr_OK;
}

auto SrvDAC::CtrlSetLMFC(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = *static_cast<BRD_JesdLmfc*>(args);

    m_dac.jesd_set_lmfc(value.delay, value.variable);

    LOG("BRDctrl_JESD_SETLMFC | delay = %zd, variable = %zd", value.delay, value.variable);

    return BRDerr_OK;
}

auto SrvDAC::CtrlGetLMFC(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = static_cast<BRD_JesdLmfcLatency*>(args);

    auto jesd_info = m_dac.jesd_get_info(); // получим информацию о JESD
    auto latency = m_dac.jesd_get_lmfc(); // получим информацию о латентности
    value->delay_link0 = latency & 0xFF;
    value->delay_link1 = (latency & 0xFF00) >> 8;
    value->L = jesd_info.L;
    value->K = jesd_info.K; // необходимо для расчётов
    value->F = jesd_info.F;

    LOG("BRDctrl_JESD_GETLMFC | delay_link0:%zd, delay_link1:%zd, L:%zd, K:%zd, F:%zd",
        value->delay_link0, value->delay_link1, value->L, value->K, value->F);

    return BRDerr_OK;
}

///
/// \brief      Установка параметров стартовой синхронизации
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvDAC::CtrlSetStartMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
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
    //   функция: CAdcSrv::SetStartMode()

    //    3                   2                   1                   0
    //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    // ┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
    // │                               │R│I│ │ SrcStop │T│I│ │ SrcStart│
    // └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
    //                     Restart Mode ┘ │             │ └ Inverting Start Signal
    //              Inverting Stop Signal ┘             └ Trigger Start Mode
    //

    // в нашем случае режим старта в несущей плате
    // if(pMode0->ByBits.Master) // Single
    auto val = ADM2IF_STMODE {};
    val.ByBits.SrcStart = start_mode.startSrc;
    val.ByBits.SrcStop = start_mode.trigStopSrc;
    val.ByBits.InvStart = start_mode.startInv;
    val.ByBits.InvStop = start_mode.stopInv;
    val.ByBits.TrigStart = start_mode.trigOn;
    val.ByBits.Restart = start_mode.reStartMode;

    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_STMODE, val.AsWhole);
    LOG("BRDctrl_DAC_SETSTARTMODE | ADM2IFnr_STMODE: %08Xh", val.AsWhole);

    return BRDerr_OK;
}

///
/// \brief      Получение параметров стартовой синхронизации
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvDAC::CtrlGetStartMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) start_mode = ArgumentCast<BRD_StartMode>(args);

    auto val = ADM2IF_STMODE {};
    val.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_STMODE);
    LOG("BRDctrl_DAC_GETSTARTMODE | ADM2IFnr_STMODE: %08Xh", val.AsWhole);

    start_mode.startSrc = val.ByBits.SrcStart; // select of start signal
    start_mode.trigStopSrc = val.ByBits.SrcStop; // select of stop signal
    start_mode.startInv = val.ByBits.InvStart; // inverting start signal
    start_mode.stopInv = val.ByBits.InvStop; // inverting stop signal
    start_mode.trigOn = val.ByBits.TrigStart; // trigger start mode
    start_mode.reStartMode = val.ByBits.Restart; // Restart mode

    return BRDerr_OK;
}

///
/// Установка значения начальной задержки в отсчётах (кадровый режим)
///
auto SrvDAC::CtrlSetStartDelay(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) param = ArgumentCast<BRD_EnVal>(args);

    auto mode = ADM2IF_MODE0 {};
    mode.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_MODE0);
    mode.ByBits.Cnt0En = param.enable;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_MODE0, mode.AsWhole);

    // конвертирование количества отсчётов на канал в размер FIFO слов
    // = (отсчёты на канал * размер отсчёта в байтах * кол-во каналов) / (ширина FIFO / 8)
    auto channel_mask = uint32_t {};
    CtrlGetChanMask(pDev, pServData, 0, &channel_mask);
    auto delay_count = uint32_t(param.value * sizeof(uint16_t) * std::bitset<32>(channel_mask).count() / (m_fifo_type >> 3));

    // записываем 16 бит значения,
    // если есть признак 32-х битных счётчиков, пишем старшие 16 бит значения
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_CNT0, delay_count & 0xFFFF);
    if (sub_module.IndRegRead(m_tetr_num, ADM2IFnr_TRES) & (1 << 10))
        sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_ECNT0, delay_count >> 16);

    LOG("BRDctrl_DAC_SETSTDELAY | enable: %s, count: %d", param.enable ? "true" : "false", delay_count);

    return BRDerr_OK;
}

auto SrvDAC::CtrlSetStartLevel(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) val = ArgumentCast<BRD_StartLevel_FM214x3GDA>(args);

    if (val.level < 0.0)
        val.level = 0.0;
    if (val.level > 5.0)
        val.level = 5.0;
    if (val.resistance > 1)
        val.resistance = 1;

    LOG("BRDctrl_DAC_SETSTARTLEVEL | level: %.02f V, resistance: %s", val.level, val.resistance ? "50 Ohm" : "1 kOhm");

    sub_module.SetStartLevel(val.level, val.resistance);

    return BRDerr_OK;
}

auto SrvDAC::CtrlSetSysRef(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) val = ArgumentCast<BRD_SysRefConfig_FM214x3GDA>(args);

    if (val.level < 0.0)
        val.level = 0.0;
    if (val.level > 5.0)
        val.level = 5.0;

    LOG("BRDctrl_DAC_SETSYSREF | level: %.02f V, config: 0x%X", val.level, val.select);

    sub_module.SetSysRefDAC(val.level, val.select);

    return BRDerr_OK;
}

auto SrvDAC::CtrlSetLMFCCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = *static_cast<BRD_JesdLmfcCount*>(args);

    sub_module.IndRegWrite(m_tetr_num, 0x19, value.count);
    auto jesd_info = m_dac.jesd_get_info(); // получим информацию о JESD
    sub_module.IndRegWrite(m_tetr_num, 0x1A, (jesd_info.K * jesd_info.F) >> 2);

    LOG("BRDctrl_DAC_SETJESDLMFCCOUNT | LMFC Count = %zd, LMFC Ratio = %zd",
        sub_module.IndRegRead(m_tetr_num, 0x19), sub_module.IndRegRead(m_tetr_num, 0x1A));

    return BRDerr_OK;
}

auto SrvDAC::CtrlGetLMFCCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = static_cast<BRD_JesdLmfcCount*>(args);

    constexpr auto LMFC_COUNT_GET = 0x218;
    value->count = sub_module.IndRegRead(m_tetr_num, LMFC_COUNT_GET);

    auto jesd_info = m_dac.jesd_get_info(); // получим информацию о JESD
    value->K = jesd_info.K; // необходимо для расчётов
    value->F = jesd_info.F;

    LOG("BRDctrl_DAC_GETJESDLMFCCOUNT | LMFCCount:%zd", value->count);

    return BRDerr_OK;
}

auto SrvDAC::CtrlSetPreSync(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = static_cast<BRD_PreSync_FM214x3GDA*>(args);
    auto presync = PRESYNC_0x18 {};
    presync.all = sub_module.IndRegRead(m_tetr_num, 0x18);

    LOG("BRDctrl_DAC_SETPRESYNC | Enable: %s, Type: %s", value->enable ? "ON" : "OFF", value->type == BRD_PRESYNC_TYPE_FM214x3GDA::SYNC ? "SYNC" : "ASYNC");

    presync.MODE = value->enable;
    presync.START = size_t(value->type);
    sub_module.IndRegWrite(m_tetr_num, 0x18, presync.all);

    return BRDerr_OK;
}

///
/// \brief Установка стандартной задержки для IDELAYE3 (UltraScale)
///
auto SrvDAC::CtrlSetDelay(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) param = ArgumentCast<BRD_StdDelay>(args);

    using reg_0x2F0 = union {
        uint32_t all;
        struct {
            uint32_t
                DELAY : 9, // значение задержки
                USED : 1, // признак реализации задержки в ПЛИС
                ID : 4, // идентификатор узла задержки:
                        // 0 - ADC input 0, ...
                        // 8 - External Start, ...
                        // 12 - SYNX inp0, ...
                WRITE : 1, // запись в поле задержки
                RESET : 1; // сброс в исходное состояние
            ;
        };
    };

    auto std_delay = reg_0x2F0 {};
    std_delay.DELAY = 0b1'1111'1111;
    std_delay.ID = param.nodeID;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_STDDELAY, std_delay.all);
    std_delay.all = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_STDDELAY);
    if (std_delay.USED) {
        std_delay.WRITE = 1;
        std_delay.DELAY = param.value;
        sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_STDDELAY, std_delay.all);
    } else {
        LOG("BRDctrl_DAC_WRDELAY | Bad NodeID: %d", param.nodeID);
        return BRDerr_BAD_NODEID;
    }

    LOG("BRDctrl_DAC_WRDELAY | NodeID: %d, 0x2F0 = %04Xh [%04Xh]", param.nodeID, std_delay.all, sub_module.IndRegRead(m_tetr_num, ADM2IFnr_STDDELAY));
    return BRDerr_OK;
}
