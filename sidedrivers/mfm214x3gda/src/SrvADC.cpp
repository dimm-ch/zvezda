#include <bitset>
#include <cmath>
#include <thread>

#include "SrvADC.h"
#include "sidedriver.h"

// #ifndef NDEBUG
// #define NDEBUG // отключение любой отладочной информации для этого класса
// #endif
#include "log.h"

///
/// Конструктор
///
SrvADC::SrvADC(Cmfm214x3gda* parent)
    : CService(-1, _BRDC("ADC214x3GDA"), -1, parent, nullptr, 0)
{
    // LOG("ctor (parent: %p) -> this: %p", parent, this);

    m_adc.setup_spi(
        [this, parent](size_t reg) -> uint8_t { return parent->SpdRead(m_tetr_num, 0, 0, reg); },
        [this, parent](size_t reg, uint8_t val) -> void { parent->SpdWrite(m_tetr_num, 0, 0, reg, val); });

    // атрибуты службы
    m_attribute = BRDserv_ATTR_DIRECTION_IN | BRDserv_ATTR_STREAMABLE_IN | BRDserv_ATTR_SDRAMABLE;

    // добавление команд службы
    m_Cmd.push_back(new CMD_Info { BRDctrl_SETDEBUGINFO, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetDebugInfo) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETMODE, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETJESD, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetJesd) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETINVERTCHANNELA, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetInvertChannelA) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETINPUTFSVOLTAGE, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetInputFSVoltage) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETINPUTBUFFERCONTROL, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetInputBufferControl) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETSTARTLEVEL, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetStartLevel) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETCLOCKDELAYCONTROL, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetClockDelayControl) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETCLOCKDELAYCHANNEL, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetClockDelayChannel) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DDC_SETFCPHASE, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetDDCFcPhase) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETFASTDETECTCHANNEL, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetFastDetectChannel) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETPRESYNC, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetPreSync) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETTIMESTAMP, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetTimeStamp) });

    // общие команды службы
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_GETSRCSTREAM, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetSrcStream) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_ENABLE, 0, 0, 0, CmdEntry(&SrvADC::CtrlEnable) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_FIFORESET, 0, 0, 0, CmdEntry(&SrvADC::CtrlFifoReset) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_FIFOSTATUS, 0, 0, 0, CmdEntry(&SrvADC::CtrlFifoStatus) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETCHANMASK, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetChanMask) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_GETCHANMASK, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetChanMask) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETCLKMODE, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetClkMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DDC_SETFC, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetDDCFc) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DDC_GETFC, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetDDCFc) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETSTARTMODE, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetStartMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_GETSTARTMODE, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetStartMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_DDC_SETINPSRC, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetDDCInputSource) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETSTDELAY, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetStartDelay) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETACQDATA, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetAcqCount) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETSKIPDATA, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetSkipCount) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETTITLEMODE, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetTitleMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETTITLEDATA, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetTitleData) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETPRETRIGMODE, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetPretrigMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_GETPRETRIGMODE, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetPretrigMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_JESD_SETLMFCCOUNT, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetLMFCCount) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_JESD_GETLMFCCOUNT, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetLMFCCount) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_JESD_GETLINKERRORSTATUS, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetJESDLinkErrorStatus) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_JESD_GETSYNCSTATUS, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetJESDSyncStatus) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_JESD_GETDEBUGSTATUS, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetJESDDebugStatus) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_JESD_SETPRBS, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetJESDPrbs) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_JESD_GETPRBSSTATUS, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetJESDPrbsStatus) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_WRDELAY, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetDelay) });

    // команды службы, необходимые только для `exam_adc`
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_GETCFG, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetCfg) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_READINIFILE, 0, 0, 0, CmdEntry(&SrvADC::CtrlReadIniFile) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_GETSYNCMODE, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetSyncMode) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_RDDELAY, 0, 0, 0, CmdEntry(&SrvADC::CtrlReadDelay) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_GETFORMAT, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetFormat) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_GETINPRANGE, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetInpRange) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_GETBIAS, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetBias) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_ISCOMPLEX, 0, 0, 0, CmdEntry(&SrvADC::CtrlIsComplex) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_GETFC, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetDDCFc) });
    // m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_GETINTERP,    0,0,0, CmdEntry(&SrvADC::CtrlGetInterpFactor) });
    m_Cmd.push_back(new CMD_Info { BRDctrl_ADC_SETMASTER, 0, 0, 0, CmdEntry(&SrvADC::CtrlSetMaster) });

    m_Cmd.push_back(new CMD_Info { BRDctrl_GETTRACETEXT, 0, 0, 0, CmdEntry(&SrvADC::CtrlGetTraceText) });
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
auto SrvADC::CtrlSetDebugInfo(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) param = ArgumentCast<bool>(args);

    debug_info = param;
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
auto SrvADC::CtrlIsAvailable(void* pDev, void* pServData, ULONG cmd, void* args) -> int
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

    // ищем порядковый номер ADC тетрады
    m_tetr_num = sub_module.FindID(ADC_ID, 1);

    // нет такой тетрады, службу нельзя захватывать
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
auto SrvADC::CtrlCapture(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);

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
auto SrvADC::CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    return BRDerr_CMD_UNSUPPORTED; // for free subservice
}

///
/// Получение порядкового номера тетрады
///
/// \brief Необходимо стриму для установки тетрады источника данных
///        Вызывается командой `SERVcmd_SYS_GETADDRDATA`
///
auto SrvADC::CtrlGetAddrData(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) tetrada = ArgumentCast<uint32_t>(args);

    tetrada = m_tetr_num;
    return BRDerr_OK;
}

///
/// Получение порядкового номера тетрады
///
/// \brief Необходимо стриму для установки тетрады источника данных
///        Вызывается командой `BRDctrl_ADC_GETSRCSTREAM`
///
auto SrvADC::CtrlGetSrcStream(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) tetrada = ArgumentCast<uint32_t>(args);

    tetrada = m_tetr_num;
    return BRDerr_OK;
}

///
/// Сброс FIFO
///
auto SrvADC::CtrlFifoReset(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);

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
auto SrvADC::CtrlFifoStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) status = ArgumentCast<uint32_t>(args);

    status = sub_module.DirRegRead(m_tetr_num, ADM2IFnr_STATUS);

    return BRDerr_OK;
}

///
/// Активация/деактивация ЦАП
///
auto SrvADC::CtrlEnable(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) enable = ArgumentCast<uint32_t>(args);

    // программный старт/останов тетрады
    ADM2IF_MODE0 mode {};
    mode.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_MODE0);
    mode.ByBits.Start = enable;
    mode.ByBits.Master = true;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_MODE0, mode.AsWhole);

    // LOG("BRDctrl_ADC_ENABLE | %s, 0x%X", enable ? "enabel" : "disable", mode.AsWhole);
    return BRDerr_OK;
}

///
/// \brief      Установка маски используемых каналов. Регистр ADM2IFnr_CHAN1
///             [0x10]. Выбор каналов биты [7..0] MASK.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     result
///
auto SrvADC::CtrlSetChanMask(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) value = ArgumentCast<uint32_t>(args);

    auto chan1 = ADM2IF_CHAN1 {};
    chan1.ByBits.ChanSel = value;

    LOG("BRDctrl_ADC_SETCHANMASK = %04Xh", value);

    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_CHAN1, chan1.AsWhole);
    return BRDerr_OK;
}

///
/// \brief      Получение маски каналов ADC/DDC. Регистр ADM2IFnr_CHAN [0x10]
///             Выбор каналов биты [7..0] MASK.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     result
///
auto SrvADC::CtrlGetChanMask(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) chan_mask = ArgumentCast<uint32_t>(args);

    auto chan1 = ADM2IF_CHAN1 {};
    chan1.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_CHAN1); // 0x10
    chan_mask = chan1.ByBits.ChanSel;

    // проверка на включённый DDC
    if (chan_mask == 0) {
        chan1.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_CHAN1 + 1); // 0x11
        chan_mask = chan1.ByBits.ChanSel;
    }

    // auto chan = std::bitset<32>(sub_module.IndRegRead(m_tetr_num, ADM2IFnr_CHAN1));
    // auto chan_iq = std::bitset<32>();
    // if (chan.test(0)) chan_iq.set(0), chan_iq.set(1);
    // if (chan.test(1)) chan_iq.set(2), chan_iq.set(3);
    // *chan_mask = chan_iq.to_ulong();

    // LOG("BRDctrl_ADC_GETCHANMASK | %04Xh", *chan_mask);

    return BRDerr_OK;
}

///
/// \brief      Установка и инициализация схемы тактирования модуля.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       Адрес структуры `BRD_ClkMode`
///
/// \return     result
///
auto SrvADC::CtrlSetClkMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) clk_mode = ArgumentCast<BRD_ClkMode_FM214x3GDA>(args);
    auto ctrl = CONTROL1_0x17 {};

    // Схема тактирования АЦП
    // -------------------------------------------------------------------------
    //
    // +-----------+   +---------------+             +----------+
    // | Ситезатор |   | Делитель HMC  |   3200 МГц  |   АЦП    |
    // |    LMX    +---> регистр FDVR  +------------->  AD9208  |
    // |           |   |               |   максимум  |          |
    // +-----------+   +---------------+             +----------+
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

    // Подбор коэффициентов деления входной тактовой частоты
    // для АЦП (FDVR). Максимальная частота 3100 МГц.
    // -------------------------------------------------------------------------
    auto FDVR = 8; // значение по умолчанию
    for (auto div : std::vector<int> { 1, 2, 4, 8 }) {
        if ((sub_module.m_lmx_Fout / div) > 3'200)
            continue;
        FDVR = div;
        break;
    }
    sub_module.IndRegWrite(sub_module.m_sync_tetr_num, 0x14, FDVR);
    LOG("ADC input: %.02f MHz [ FDVR: %d ]", sub_module.m_lmx_Fout / FDVR, FDVR);

    // AD9208
    // -------------------------------------------------------------------------
    ctrl.PWUP_ADC = true; // включим питание ADC
    sub_module.IndRegWrite(m_tetr_num, 0x17, ctrl.all);
    m_adc.reset();
    m_adc.get_chip_id();
    m_adc.set_fclk(sub_module.m_lmx_Fout / FDVR);

    // LTC2991 | Инициализация монитора напряжений и температур
    // -------------------------------------------------------------------------
    // sub_module.m_monitor.reset();

    return BRDerr_OK;
}

///
/// \brief      Установка режима работы АЦП.
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlSetMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) mode = ArgumentCast<BRD_AdcMode_FM214x3GDA>(args);

    LOG("BRDctrl_ADC_SETMODE | { AppMode:%zd, ChannelMask:%02zX, DDC Decimate:%zd }",
        mode.app_mode, mode.channel_mask, mode.ddc_decimate);

    if (mode.app_mode == 0) {
        // режим ADC | установка CHAN_ADC
        sub_module.IndRegWrite(m_tetr_num, 0x10, mode.channel_mask);
        sub_module.IndRegWrite(m_tetr_num, 0x11, 0);
        // режим одного АЦП, который устанавливается не в chip mode, а значением M для JESD'а
        if (mode.channel_mask == 0b0001)
            mode.app_mode = 0b1110;
        if (mode.channel_mask == 0b0010)
            mode.app_mode = 0b1111;
    } else {
        // режим DDC | установка CHAN_ADC и CHAN_DDC
        sub_module.IndRegWrite(m_tetr_num, 0x10, 0);
        sub_module.IndRegWrite(m_tetr_num, 0x11, mode.channel_mask);
    }

    LOG("- CHAN_ADC [0x10] = %04zX, CHAN_DDC [0x11] = %04zX",
        sub_module.IndRegRead(m_tetr_num, 0x10),
        sub_module.IndRegRead(m_tetr_num, 0x11));

    m_adc.set_mode(mode.app_mode, mode.ddc_decimate);

    return BRDerr_OK;
}

///
/// \brief      Настройка JESD'ов ПЛИС и АЦП
///
/// \param      pDev       The development
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlSetJesd(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) param = ArgumentCast<bool>(args);

    auto ext_core_clk = param;
    LOG("BRDctrl_ADC_SETJESD %s", ext_core_clk ? "| Using External Core Clock" : "");

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

    // Настройка JESD'ов в АЦП
    // -------------------------------------------------------------------------
    // получим идентификатор JESD PHY, используемый в ПЛИС
    m_jesd_phy_id = sub_module.IndRegRead(m_tetr_num, 0x110);
    LOG("JESD Phy ID: %s [%02zXh]", m_jesd_phy_id == 0x13 ? "CPLL" : "QPLL", m_jesd_phy_id);

    switch (m_jesd_phy_id) {
    case 0x0F: // QPLL JESD204B
    case 0x15: // QPLL JESD204C
        if (m_adc.set_jesd(15'500.0) == false)
            return BRDerr_ERROR;
        break;
    case 0x13: // CPLL
        if (m_adc.set_jesd(12'500.0) == false)
            return BRDerr_ERROR;
        break;
    default:
        LOG("Unknow JESD Phy ID");
        return BRDerr_ERROR;
    }

    // Установка CoreCLK и RefCLK для JESD'ов в ПЛИС
    // -------------------------------------------------------------------------
    if (sub_module.SetJesdCoreAndRefCLK(cmd, m_adc.get_lane_rate(), ext_core_clk) == false)
        return BRDerr_ERROR;

    // JESD FPGA PHY's QPLL/CPLL GTX Common
    // ------------------------------------
    if ((m_jesd_phy_id == 0x0F) || (m_jesd_phy_id == 0x15))
        sub_module.SetJesdQPLL(cmd);
    else // m_jesd_phy_id == 0x13
        sub_module.SetJesdCPLL(cmd);

    // JESD FPGA приёмники
    // -------------------------------------------------------------------------
    auto jesd_L = m_adc.get_jesd_L();

    auto ctrl = CONTROL1_0x17 {};
    ctrl.all = sub_module.IndRegRead(m_tetr_num, 0x17);
    if (sub_module.m_sync_slave == false)
        ctrl.RX_RESET = 1; // взведём reset, если модуль в режиме MASTER
    sub_module.IndRegWrite(m_tetr_num, 0x17, ctrl.all);
    // выставим количестов JESD линий
    ctrl.RX_LANES = (jesd_L == 8) ? 0b00 : (jesd_L == 4) ? 0b01
        : (jesd_L == 2)                                  ? 0b10
        : (jesd_L == 1)                                  ? 0b11
                                                         : 0b00;
    ctrl.RX_RESET = 0; // отпустим reset
    sub_module.IndRegWrite(m_tetr_num, 0x17, ctrl.all);
    std::this_thread::sleep_for(200ms);

    std::vector<size_t> JESD_FPGA_devs = { 8, /*9*/ };
    for (auto dev : JESD_FPGA_devs) {
        if (m_jesd_phy_id == 0x15) {
            // параметры JESD204C
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x024, 0b10); // Enables the AXI4-Stream Data interface
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x034, 0x01); // subclass 1 mode
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x040, // lanes in use
                (jesd_L == 1) ? 0b00000001 : (jesd_L == 2) ? 0b00000011
                    : (jesd_L == 4)                        ? 0b00001111
                    : (jesd_L == 8)                        ? 0b11111111
                                                           : 0xF);
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x050, 0x01); // SysRef Handling
            auto reg_03C = sub_module.SpdRead(m_tetr_num, dev, 0, 0x03C);
            reg_03C |= 1 << 16; // screambling on
            reg_03C |= m_adc.get_jesd_F() - 1; // Octets per Frame - 1
            reg_03C |= (32 - 1) << 8; // Frames per MultiFrame - 1
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x03C, reg_03C);

            LOG("JESD link %d: HEX 020=%02zX, 024=%02zX, 034=%02zX, 03C=%04zX, 040=%02zX, 050=%04zX", dev,
                sub_module.SpdRead(m_tetr_num, dev, 0, 0x020), sub_module.SpdRead(m_tetr_num, dev, 0, 0x024),
                sub_module.SpdRead(m_tetr_num, dev, 0, 0x034), sub_module.SpdRead(m_tetr_num, dev, 0, 0x03C),
                sub_module.SpdRead(m_tetr_num, dev, 0, 0x040), sub_module.SpdRead(m_tetr_num, dev, 0, 0x050));

            // Reset (изменения в регистрах фиксируются после ресета)
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x020, 1);
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x020, 0);
            std::this_thread::sleep_for(20ms);
            if ((sub_module.SpdRead(m_tetr_num, dev, 0, 0x020) & 1) != 0) {
                LOG("JESD No Reset");
                return BRDerr_ERROR;
            }
        } else {
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x004, 0x10001); // disable watchdog timer & reset
            std::this_thread::sleep_for(20ms);
            if ((sub_module.SpdRead(m_tetr_num, dev, 0, 0x004) & 1) != 0) {
                LOG("JESD No Reset");
                return BRDerr_ERROR;
            }

            // параметры JESD204B
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x00C, 0x01); // screambling on
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x02C, 0x01); // subclass 1 mode
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x028, // lanes in use
                (jesd_L == 1) ? 0b00000001 : (jesd_L == 2) ? 0b00000011
                    : (jesd_L == 4)                        ? 0b00001111
                    : (jesd_L == 8)                        ? 0b11111111
                                                           : 0xF);
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x020, m_adc.get_jesd_F() - 1); // Octets per Frame - 1
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x024, 32 - 1); // Frames per MultiFrame - 1
            sub_module.SpdWrite(m_tetr_num, dev, 0, 0x010, 0x01);

            LOG("JESD link %d: HEX 004=%02zX, 00C=%02zX, 010=%02zX, 020=%02zX, 024=%02zX, 028=%02zX, 02Ch=%02zX", dev,
                sub_module.SpdRead(m_tetr_num, dev, 0, 0x004), sub_module.SpdRead(m_tetr_num, dev, 0, 0x00C),
                sub_module.SpdRead(m_tetr_num, dev, 0, 0x010), sub_module.SpdRead(m_tetr_num, dev, 0, 0x020),
                sub_module.SpdRead(m_tetr_num, dev, 0, 0x024), sub_module.SpdRead(m_tetr_num, dev, 0, 0x028),
                sub_module.SpdRead(m_tetr_num, dev, 0, 0x02C));
        }
    }

    return BRDerr_OK;
}

///
/// \brief      Установка значения центральной частоты всех DDC
///
/// \param      pDev       The dev
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlSetDDCFc(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) val_chan = ArgumentCast<BRD_ValChan>(args);

    LOG("BRDctrl_DDC_SETFC | ch: %d, freq: %.02f MHz", val_chan.chan, val_chan.value);
    m_adc.set_nco(val_chan.chan, val_chan.value);

    return BRDerr_OK;
}

///
/// \brief      Чтение значения центральной частоты всех DDC
///
/// \param      pDev       The dev
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlGetDDCFc(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) val_chan = ArgumentCast<BRD_ValChan>(args);

    val_chan.value = m_adc.get_nco(val_chan.chan) * 1'000'000; // в Гц
    // LOG("BRDctrl_DDC_GETFC | ch: %d, freq: %.02f MHz", val_chan.chan, val_chan.value);

    return BRDerr_OK;
}

///
/// \brief      Установка фазы центральной частоты всех DDC
///
/// \param      pDev       The dev
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlSetDDCFcPhase(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) val_chan = ArgumentCast<BRD_ValChan>(args);

    LOG("BRDctrl_DDC_SETFCPHASE | ch: %d, phase: %.02f\xF8", val_chan.chan, val_chan.value);
    m_adc.set_nco_offset(val_chan.chan, val_chan.value);

    return BRDerr_OK;
}

auto SrvADC::CtrlSetLMFCCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = *static_cast<BRD_JesdLmfcCount*>(args);

    sub_module.IndRegWrite(m_tetr_num, 0x19, value.count);
    sub_module.IndRegWrite(m_tetr_num, 0x1A, (/*jesd_K*/ 32 * m_adc.get_jesd_F()) >> 2);

    LOG("BRDctrl_JESD_SETLMFCCOUNT | LMFC Count = %zd, LMFC Ratio = %zd", sub_module.IndRegRead(m_tetr_num, 0x19), sub_module.IndRegRead(m_tetr_num, 0x1A));

    return BRDerr_OK;
}

auto SrvADC::CtrlGetLMFCCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = static_cast<BRD_JesdLmfcCount*>(args);

    constexpr auto LMFC_COUNT_GET = 0x218;
    value->count = sub_module.IndRegRead(m_tetr_num, LMFC_COUNT_GET);
    value->K = 32;
    value->F = m_adc.get_jesd_F();

    LOG("BRDctrl_JESD_GETLMFCCOUNT | K:%zd, F:%zd, LMFCCount:%zd", value->K, value->F, value->count);

    return BRDerr_OK;
}

///
/// \brief      Установка DDC IQ входов на конкретный ADC
///
/// \param      pDev       The dev
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlSetDDCInputSource(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) val = ArgumentCast<BRD_DdcInputIQ_FM214x3GDA>(args);

    LOG("BRDctrl_DDC_SETINPSRC | channel: %zd, I: ADC%zd, Q: ADC%zd", val.channel, val.src_i, val.src_q);

    auto result = m_adc.set_ddc_input_iq(val.channel, val.src_i, val.src_q);

    if (result)
        return BRDerr_OK;
    else
        return BRDerr_BAD_PARAMETER;
}

auto SrvADC::CtrlSetInvertChannelA(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);

    LOG("BRDctrl_ADC_SETINVERTCHANNELA");

    m_adc.set_invert_channel_a();

    return BRDerr_OK;
}

auto SrvADC::CtrlSetInputFSVoltage(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) val = ArgumentCast<BRD_ValChan>(args);

    LOG("BRDctrl_ADC_SETINPUTFSVOLTAGE | channel: %d, voltage: %.02fV", val.chan, val.value);

    m_adc.set_input_fs_voltage(val.chan, val.value);

    return BRDerr_OK;
}

auto SrvADC::CtrlSetInputBufferControl(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) val = ArgumentCast<BRD_ValChan>(args);

    LOG("BRDctrl_ADC_SETINPUTBUFFERCONTROL | channel: %d, input: %zd uA", val.chan, size_t(val.value));

    m_adc.set_input_buffer_control(val.chan, size_t(val.value));

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
auto SrvADC::CtrlSetStartMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
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
    LOG("BRDctrl_ADC_SETSTARTMODE | ADM2IFnr_STMODE: %08Xh", val.AsWhole);

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
auto SrvADC::CtrlGetStartMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) start_mode = ArgumentCast<BRD_StartMode>(args);

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

auto SrvADC::CtrlSetStartLevel(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) val = ArgumentCast<BRD_StartLevel_FM214x3GDA>(args);

    if (val.level < 0.0)
        val.level = 0.0;
    if (val.level > 5.0)
        val.level = 5.0;
    if (val.resistance > 1)
        val.resistance = 1;

    LOG("BRDctrl_ADC_SETSTARTLEVEL | level: %.02f V, resistance: %s", val.level, val.resistance ? "50 Ohm" : "1 kOhm");

    sub_module.SetStartLevel(val.level, val.resistance);

    return BRDerr_OK;
}

auto SrvADC::CtrlSetSysRef(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) val = ArgumentCast<BRD_SysRefConfig_FM214x3GDA>(args);

    if( val.level < 0.0 )
        val.level = 0.0;
    if( val.level > 5.0 )
        val.level = 5.0;

    LOG("BRDctrl_DAC_SETSYSREF | level: %.02f V, config: 0x%X", val.level, val.select);

    sub_module.SetSysRefADC(val.level, val.select);

    return BRDerr_OK;
}

auto SrvADC::CtrlSetClockDelayControl(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) val = ArgumentCast<uint8_t>(args) & 0b111;

    LOG("BRDctrl_ADC_SETCLOCKDELAYCONTROL | 0x110 = %02X", val);
    m_adc.set_clock_delay_control(val);

    return BRDerr_OK;
}

auto SrvADC::CtrlSetClockDelayChannel(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) params = ArgumentCast<BRD_ClockDelayChannel_FM214x3GDA>(args);

    if (params.channel > 4)
        return BRDerr_BAD_PARAMETER;
    params.divider_phase &= 0b1111;
    params.superfine_delay = (params.superfine_delay > 128) ? 128 : params.superfine_delay;
    params.fine_delay = (params.fine_delay > 192) ? 192 : params.fine_delay;

    LOG("BRDctrl_ADC_SETCLOCKDELAYCHANNEL | ch: %zd, Divider Phase: %02d, Superfine Delay: %03d, Fine Delay: %03d",
        params.channel, params.divider_phase, params.superfine_delay, params.fine_delay);
    m_adc.set_clock_delay(params.channel, params.divider_phase, params.superfine_delay, params.fine_delay);

    return BRDerr_OK;
}

///
/// \brief      Установка значения начальной задержки в отсчётах (кадровый
///             режим)
///
/// \param      pDev       The dev
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlSetStartDelay(void* pDev, void* pServData, ULONG cmd, void* args) -> int
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

    LOG("BRDctrl_ADC_SETSTDELAY | enable: %s, count: %d", param.enable ? "true" : "false", delay_count);

    return BRDerr_OK;
}

///
/// \brief      Установка количества принимаемых данных в отсчётах (кадровый
///             режим)
///
/// \param      pDev       The dev
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlSetAcqCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) param = ArgumentCast<BRD_EnVal>(args);

    auto mode = ADM2IF_MODE0 {};
    mode.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_MODE0);
    mode.ByBits.Cnt1En = param.enable;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_MODE0, mode.AsWhole);

    // конвертирование количества отсчётов на канал в размер FIFO слов
    // = (отсчёты на канал * размер отсчёта в байтах * кол-во каналов) / (ширина FIFO / 8)
    auto channel_mask = uint32_t {};
    CtrlGetChanMask(pDev, pServData, 0, &channel_mask);
    auto delay_count = uint32_t(param.value * sizeof(uint16_t) * std::bitset<32>(channel_mask).count() / (m_fifo_type >> 3));

    // записываем 16 бит значения,
    // если есть признак 32-х битных счётчиков, пишем старшие 16 бит значения
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_CNT1, delay_count & 0xFFFF);
    if (sub_module.IndRegRead(m_tetr_num, ADM2IFnr_TRES) & (1 << 10))
        sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_ECNT1, delay_count >> 16);

    LOG("BRDctrl_ADC_SETACQDATA | enable: %s, count: %d", param.enable ? "true" : "false", delay_count);

    return BRDerr_OK;
}

///
/// \brief      Установка количества пропускаемых данных в отсчётах (кадровый
///             режим)
///
/// \param      pDev       The dev
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlSetSkipCount(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) param = ArgumentCast<BRD_EnVal>(args);

    auto mode = ADM2IF_MODE0 {};
    mode.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_MODE0);
    mode.ByBits.Cnt2En = param.enable;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_MODE0, mode.AsWhole);

    // конвертирование количества отсчётов на канал в размер FIFO слов
    // = (отсчёты на канал * размер отсчёта в байтах * кол-во каналов) / (ширина FIFO / 8)
    auto channel_mask = uint32_t {};
    CtrlGetChanMask(pDev, pServData, 0, &channel_mask);
    auto delay_count = uint32_t(param.value * sizeof(uint16_t) * std::bitset<32>(channel_mask).count() / (m_fifo_type >> 3));

    // записываем 16 бит значения,
    // если есть признак 32-х битных счётчиков, пишем старшие 16 бит значения
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_CNT2, delay_count & 0xFFFF);
    if (sub_module.IndRegRead(m_tetr_num, ADM2IFnr_TRES) & (1 << 10))
        sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_ECNT2, delay_count >> 16);

    LOG("BRDctrl_ADC_SETSKIPDATA | enable: %s, count: %d", param.enable ? "true" : "false", delay_count);

    return BRDerr_OK;
}

///
/// \brief      Установка размера заголовка кадрового режима
///
/// \param      pDev       The dev
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlSetTitleMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) param = ArgumentCast<BRD_EnVal>(args);

    // auto size = uint32_t(param.value < 8 ? 0 : (param.value * sizeof(uint32_t) / (m_fifo_type >> 3)));
    auto size = m_fifo_type / 128; // 128 бит, исторически

    auto mode = ADM2IF_TLMODE {};
    mode.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_TLMODE);
    mode.ByBits.TitleOn = param.enable;
    mode.ByBits.Size = size;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_TLMODE, mode.AsWhole);

    LOG("BRDctrl_ADC_SETTITLEMODE | enable: %s, size: %zd bytes", param.enable ? "true" : "false", size);

    return BRDerr_OK;
}

///
/// \brief      Установка персональных данных заголовка кадрового режима
///
/// \param      pDev       The dev
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlSetTitleData(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) val = ArgumentCast<uint32_t>(args);

    sub_module.IndRegWrite(m_tetr_num, 0x20C, val & 0xFFFF);
    sub_module.IndRegWrite(m_tetr_num, 0x20D, val >> 16);

    LOG("BRDctrl_ADC_SETTITLEDATA | data: %08Xh", val);

    return BRDerr_OK;
}

///
/// \brief      Установка параметров Fast Detect'а АЦП
///
/// \param      pDev       The dev
/// \param      pServData  The serv data
/// \param      cmd        The command
/// \param      args       The arguments
///
/// \return     { description_of_the_return_value }
///
auto SrvADC::CtrlSetFastDetectChannel(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) params = ArgumentCast<BRD_FastDetect_FM214x3GDA>(args);

    if (params.channel > 4)
        return BRDerr_BAD_PARAMETER;

    LOG("BRDctrl_ADC_SETFASTDETECTCHANNEL | %s ch: %zd, upper: %.02f, lower: %.02f, dwell: %d",
        params.enable ? "enable" : "disable", params.channel, params.threshold_upper, params.threshold_lower, params.dwell_time);
    m_adc.set_fast_detect(params.channel, params.enable, params.threshold_upper, params.threshold_lower, params.dwell_time);

    return BRDerr_OK;
}

auto SrvADC::CtrlSetPreSync(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    auto& sub_module = *static_cast<Cmfm214x3gda*>(pDev);
    auto value = static_cast<BRD_PreSync_FM214x3GDA*>(args);
    auto presync = PRESYNC_0x18 {};
    presync.all = sub_module.IndRegRead(m_tetr_num, 0x18);

    LOG("BRDctrl_ADC_SETPRESYNC | Enable: %s", value->enable ? "ON" : "OFF");

    // отключим предварительную синхронизацию
    if (value->enable == false) {
        presync.MODE = size_t(BRD_PRESYNC_TYPE_FM214x3GDA::SYNC);
        presync.START = false;
        sub_module.IndRegWrite(m_tetr_num, 0x18, presync.all);
        return BRDerr_OK;
    }

    //установка программного старта для пресинка
    BRD_StartMode start_mode = { 0 };
    uint32_t start_src = 0;
    CtrlGetStartMode(pDev, pServData, BRDctrl_ADC_GETSTARTMODE, &start_mode);
    start_src = start_mode.startSrc;
    start_mode.startSrc = 0;
    CtrlSetStartMode(pDev, pServData, BRDctrl_ADC_SETSTARTMODE, &start_mode);

    // проведём предварительную синхронизацию
    presync.MODE = size_t(BRD_PRESYNC_TYPE_FM214x3GDA::ASYNC);
    presync.START = true;
    sub_module.IndRegWrite(m_tetr_num, 0x18, presync.all);
    presync.START = false;
    sub_module.IndRegWrite(m_tetr_num, 0x18, presync.all);

    // ждём результат
    auto presync_result = false;
    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(m_timeout_msec);
    do {
        presync_result = (sub_module.DirRegRead(m_tetr_num, ADM2IFnr_STATUS) & (1 << 13)) != 0;
    } while ((presync_result == false) && (timeout > std::chrono::steady_clock::now()));

    //возвращение установленного ранее значения
    start_mode.startSrc = start_src;
    CtrlSetStartMode(pDev, pServData, BRDctrl_ADC_SETSTARTMODE, &start_mode);

    if (presync_result) {
        LOG("PreSync Status: OK");
        return BRDerr_OK;
    } else {
        LOG("PreSync Status: ERROR");
        return BRDerr_ERROR;
    }
}

///
/// \brief Установка режима временных меток TimeStamp в потоке.
///
auto SrvADC::CtrlSetTimeStamp(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) value = ArgumentCast<BRD_TimeStamp_FM214x3GDA>(args);
    auto ctrl = CONTROL1_0x17 {};

    LOG("BRDctrl_ADC_SETTIMESTAMP | Enable: %s", value.enable ? "ON" : "OFF");

    // timestamp не может работать с выключенной предварительной синхронизацией JESD'ов
    auto presync = PRESYNC_0x18 {};
    presync.all = sub_module.IndRegRead(m_tetr_num, 0x18);
    if (value.enable && (presync.MODE == size_t(BRD_PRESYNC_TYPE_FM214x3GDA::SYNC))) {
        LOG("ERROR: You cannot use the TimeStamp with the PreSync is OFF.");
        return BRDerr_ERROR;
    }

    // выбор сигнала SYSREF и входного сопротивления внешнего старта
    auto ctrl_sync = Cmfm214x3gda::CONTROL1_0x17 {};
    ctrl_sync.all = sub_module.IndRegRead(sub_module.m_sync_tetr_num, 0x17);
    ctrl_sync.SYSREF_ADC_CNF = value.enable ? 0b11 : ctrl_sync.SYSREF_ADC_CNF; // 11 -  Внешний старт на SYSREF АЦП
    sub_module.IndRegWrite(sub_module.m_sync_tetr_num, 0x17, ctrl_sync.all);

    // включение TimeStamp в ПЛИС
    ctrl.all = sub_module.IndRegRead(m_tetr_num, 0x17);
    ctrl.TIMESTAMP_EN = value.enable;
    sub_module.IndRegWrite(m_tetr_num, 0x17, ctrl.all);

    // включение TimeStamp в ADC
    m_adc.set_time_stamp(value.enable, value.polarity_negative);

    return BRDerr_OK;
}

///
/// \brief Установка параметров претриггера и его FIFO.
///
auto SrvADC::CtrlSetPretrigMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) pretriger_mode = ArgumentCast<BRD_PretrigMode>(args);

    LOG("BRDctrl_ADC_SETPRETRIGMODE | Mode: %s, Samples: %d",
        pretriger_mode.enable ? pretriger_mode.external ? "ON EXT" : pretriger_mode.assur ? "ON ASSUR"
                                                                                          : "ON"
                              : "OFF",
        pretriger_mode.size);

    // претриггер не может работать с выключенной предварительной синхронизацией JESD'ов
    auto presync = PRESYNC_0x18 {};
    presync.all = sub_module.IndRegRead(m_tetr_num, 0x18);
    if (presync.MODE == size_t(BRD_PRESYNC_TYPE_FM214x3GDA::SYNC)) {
        LOG("ERROR: You cannot use the Pretrigger with the PreSync is OFF.");
        return BRDerr_ERROR;
    }

    // установка режима работы
    auto mode = ADM2IF_PRTMODE {};
    mode.ByBits.Enable = pretriger_mode.enable;
    mode.ByBits.Assur = pretriger_mode.assur;
    mode.ByBits.External = pretriger_mode.external;
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_PRTMODE, mode.AsWhole);

    // конвертирование количества отсчётов на канал в размер FIFO слов
    // = (отсчёты на канал * размер отсчёта в байтах * кол-во каналов) / (ширина FIFO / 8)
    auto channel_mask = uint32_t {};
    CtrlGetChanMask(pDev, pServData, BRDctrl_ADC_GETCHANMASK, &channel_mask);
    auto prefifo_size = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_PFSIZE); // размер FIFO претриггерного в FTYPE'ах
    auto sample_size = std::ceil(m_bits / 8.0) * (m_adc.is_complex() ? 2 : 1); // в байтах
    auto precount = std::ceil(pretriger_mode.size * sample_size * std::bitset<32>(channel_mask).count() / (m_fifo_type >> 3));
    if (precount > prefifo_size)
        precount = prefifo_size;
    if (precount < 2) // не должно быть меньше двух слов FIFO
        precount = 2;

    LOG("Pretrigger size %zd FIFO words (%zdx%zd bits)", size_t(precount), prefifo_size, m_fifo_type);
    sub_module.IndRegWrite(m_tetr_num, ADM2IFnr_CNTAF, (prefifo_size - 1) - size_t(precount));
    return BRDerr_OK;
}

///
/// \brief Получение параметров претриггера.
///
auto SrvADC::CtrlGetPretrigMode(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) pretriger_mode = ArgumentCast<BRD_PretrigMode>(args);

    auto mode = ADM2IF_PRTMODE {};
    mode.AsWhole = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_PRTMODE);
    pretriger_mode.enable = mode.ByBits.Enable;
    pretriger_mode.assur = mode.ByBits.Assur;
    pretriger_mode.external = mode.ByBits.External;

    auto channel_mask = uint32_t {};
    CtrlGetChanMask(pDev, pServData, BRDctrl_ADC_GETCHANMASK, &channel_mask);
    auto prefifo_size = sub_module.IndRegRead(m_tetr_num, ADM2IFnr_PFSIZE); // размер FIFO претриггерного в FTYPE'ах
    auto sample_size = std::ceil(m_bits / 8.0) * (m_adc.is_complex() ? 2 : 1); // в байтах
    pretriger_mode.size = ((prefifo_size - 1) - sub_module.IndRegRead(m_tetr_num, ADM2IFnr_CNTAF)) * (m_fifo_type >> 3) / sample_size / std::bitset<32>(channel_mask).count();

    return BRDerr_OK;
}

///
/// \brief Получение значения статусного регистра
///
auto SrvADC::CtrlGetJESDLinkErrorStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) status = ArgumentCast<uint32_t>(args);

    status = sub_module.SpdRead(m_tetr_num, 8, 0, 0x01C);
    return BRDerr_OK;
}

///
/// \brief Получение значения статусного регистра
///
auto SrvADC::CtrlGetJESDSyncStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) status = ArgumentCast<uint32_t>(args);

    status = sub_module.SpdRead(m_tetr_num, 8, 0, (m_jesd_phy_id == 0x15) ? 0x060 : 0x038);
    return BRDerr_OK;
}

///
/// \brief Получение значения статусного регистра
///
auto SrvADC::CtrlGetJESDDebugStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) status = ArgumentCast<uint32_t>(args);

    status = sub_module.SpdRead(m_tetr_num, 8, 0, (m_jesd_phy_id == 0x15) ? 0x05C : 0x03C);
    return BRDerr_OK;
}

///
/// \brief Установка режима тестовой последовательности PRBS
///
auto SrvADC::CtrlSetJESDPrbs(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) type = ArgumentCast<PRBS_TYPE>(args);

    auto ctrl = CONTROL1_0x17 {};
    ctrl.all = sub_module.IndRegRead(m_tetr_num, 0x17);
    ctrl.PRBS_ENABLE = (type == PRBS_TYPE::NONE) ? 0 : 1;
    ctrl.PRBS_MODE = (type == PRBS_TYPE::PN9) ? 0 : 1;
    sub_module.IndRegWrite(m_tetr_num, 0x017, ctrl.all);
    LOG("BRDctrl_JESD_SETPRBS | 0x017 = %04Xh", ctrl.all);

    if (type != PRBS_TYPE::NONE)
        m_adc.set_prbs(
            (type == PRBS_TYPE::PN23)      ? 0b0101
                : (type == PRBS_TYPE::PN9) ? 0b0110
                                           : 0b0000);
    return BRDerr_OK;
}

///
/// \brief Получение состояния ошибок проверки тестовой последовательности PRSB
///
auto SrvADC::CtrlGetJESDPrbsStatus(void* pDev, void* pServData, ULONG cmd, void* args) -> int
{
    decltype(auto) sub_module = ModuleCast<Cmfm214x3gda>(pDev);
    decltype(auto) status = ArgumentCast<BRD_PrbsStatus_FM214x3GDA>(args);

    status.status = sub_module.IndRegRead(m_tetr_num, 0x219);
    status.count_ch0 = sub_module.IndRegRead(m_tetr_num, 0x21A);
    status.count_ch1 = sub_module.IndRegRead(m_tetr_num, 0x21B);
    return BRDerr_OK;
}

///
/// \brief Установка стандартной задержки для IDELAYE3 (UltraScale)
///
auto SrvADC::CtrlSetDelay(void* pDev, void* pServData, ULONG cmd, void* args) -> int
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
        LOG("BRDctrl_ADC_WRDELAY | Bad NodeID: %d", param.nodeID);
        return BRDerr_BAD_NODEID;
    }

    LOG("BRDctrl_ADC_WRDELAY | NodeID: %d, 0x2F0 = %04Xh [%04Xh]", param.nodeID, std_delay.all, sub_module.IndRegRead(m_tetr_num, ADM2IFnr_STDDELAY));
    return BRDerr_OK;
}
