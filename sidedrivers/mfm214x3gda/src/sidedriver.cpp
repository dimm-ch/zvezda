#include "sidedriver.h"
// #include <filesystem>

#ifndef NDEBUG
#define NDEBUG // отключение любой отладочной информации для этого класса
#endif
#include "log.h"
bool debug_info = true; // признак вывода отладочных сообщений

///
/// \brief      Конструктор получает массив строк из brd.ini, расположенных
///             между `#begin SUBUNIT` и `#end` в виде (key, value). Например:
///             [0]:key=btype0, val=base2fmc; [1]:key=type, val=mfm214x3gda
///
/// \param      pBrdInitData  указатель на массив структур PINIT_Data : [key -
///                           строка, val - строка]
/// \param      sizeInitData  количество строк/структур
///
Cmfm214x3gda::Cmfm214x3gda(PBRD_InitData pBrdInitData, long sizeInitData)
{
    LOG("%s", info);
    LOG("ctor (pBrdInitData: %p, sizeInitData: %zu) -> this: %p", pBrdInitData, size_t(sizeInitData), this);

    for (auto i = 0; i < sizeInitData; i++)
        LOG("- BrdInitData[%d]: %s = %s", i,
            BARDY_STR(pBrdInitData[i].key), BARDY_STR(pBrdInitData[i].val));

    m_lmx.setup_spi(
        [this](size_t reg) -> uint16_t { return SpdRead(m_sync_tetr_num, 0, 0, reg); },
        [this](size_t reg, uint16_t val) -> void { SpdWrite(m_sync_tetr_num, 0, 0, reg, val); });
};

///
/// \brief Инициализация тетрад и микросхем, для которых не будут созданы
///        службы, например, тетрада `SYNC`
///
auto Cmfm214x3gda::SideDriverInit() -> int
{
    // все службы зависят от тетрады SYNC
    // ищем порядковый номер SYNC тетрады
    m_sync_tetr_num = FindID(SYNC_ID, 1);

    // если тетрада SYNC не найдена, вернём ошибку
    if (m_sync_tetr_num == -1) {
        LOG("ERROR: not found SYNC tetrade");
        return -1;
    } else
        return 0;
}

///
/// \brief      Первоначальная инициализация модуля
///
/// \param      pSubmodEEPROM  Указатель на массив байтов EEPROM
/// \param      pIniString     ??? строка инициализации ???
///
/// \note       Вызывается из `SIDE_initAuto`
///
/// \return     Любое отличное от SUBERR(BRDerr_OK) значение говорит об ошибке
///
auto Cmfm214x3gda::Init(uint8_t* pSubmodEEPROM, BRDCHAR* pIniString) -> S32
{
    LOG("SIDE_initAuto (pSubmodEEPROM: %p, pIniString: %s)",
        pSubmodEEPROM, (pIniString == nullptr) ? "nullptr" : BARDY_STR(pIniString));

    m_PID = 0xBAADF00D; // значение, если нет EEPROM

    // анализ данных из EEPROM
    if (pSubmodEEPROM != nullptr) {
        analysis_eeprom(pSubmodEEPROM);
        // дамп EEPROM субмодуля
        // auto icr = reinterpret_cast<ICR_IdBase*>(pSubmodEEPROM);
        // LOG("- EEPROM (0x%X|%d bytes)", icr->wSizeAll, icr->wSizeAll);
        // dump_icr(pSubmodEEPROM, icr->wSizeAll);
    }

    LOG("- type: %02zX, version: %02zX, pid: %zd", m_type, m_version, size_t(m_PID));
    BRDC_sprintf(m_name, _BRDC("FM214x3GDA_%X"), m_PID);

    // Добавление служб/сервисов в массив служб сайд-драйвера
    m_SrvList.push_back(&m_srv_dac);
    m_SrvList.push_back(&m_srv_adc);

    return SUBERR(BRDerr_OK); // изменение на `return BRDerr_OK` приводит к падению :-)
};

//
// \brief      Поиск тетрады по ID
//
// \param      id      Идентификатор тетрады
// \param      id_num  Номер экземпляра
//
// \return     Возвращает порядковый номер тетрады. При ошибке номер равен -1.
//
auto Cmfm214x3gda::FindID(size_t id, size_t id_num) -> ssize_t
{
    if (base_dev == nullptr) {
        LOG("ERROR: not found tetrade [base_dev == nullptr]");
        return -1;
    }

    ssize_t num = GetTetrNumEx(base_dev, 0 /*srvMainIdx*/, id, id_num /*instantNum*/);
    if (num == -1) {
        LOG("ERROR: tetrade 0x%04zX.%zd not found", id, id_num);
    } else {
        LOG("Found %04zX.%zd tetrade on %zd number", id, id_num, num);
    }
    return num;
};

///
/// \brief Анализирование данных EEPROM и заполнение найденных параметров субмодуля
///
auto Cmfm214x3gda::analysis_eeprom(uint8_t const* eeprom) -> void
{
    auto icr_base = reinterpret_cast<ICR_IdBase const*>(eeprom);
    if (icr_base->wTag != BASE_ID_TAG)
        return;
    auto eeprom_end = eeprom + icr_base->wSizeAll;
    eeprom += icr_base->wSize + sizeof(icr_base->wTag) + sizeof(icr_base->wSize);

    do {
        icr_base = reinterpret_cast<ICR_IdBase const*>(eeprom);
        // пропускаем теги мнимых "концов/окончаний", т.к. за ними могут оказаться данные
        if (icr_base->wTag == END_TAG || icr_base->wTag == ALT_END_TAG) {
            eeprom += sizeof(icr_base->wTag);
            continue;
        }

        switch (icr_base->wTag) {
        case ADM_ID_TAG: { // общие параметры субмодуля
            auto icr = reinterpret_cast<ICR_IdAdm const*>(eeprom);
            m_type = icr->wType; // тип субмодуля
            m_version = icr->bVersion; // версия субмодуля
            m_PID = icr->dSerialNum; // серийный (физический) номер
            break;
        }
        case ADCKHZ_CFG_TAG: { // конфигурационная структура АЦП (частоты выражены в килогерцах)
            auto icr = reinterpret_cast<ICR_Cfg0101 const*>(eeprom);
            m_srv_adc.m_bits = icr->bBits;
            m_srv_adc.m_encoding = icr->bEncoding;
            m_srv_adc.m_inp_range = icr->wRange / 1'000.; // переводим в вольты
            m_srv_adc.m_min_rate = icr->dMinRate / 1'000'000.; // переводим в МГц
            m_srv_adc.m_max_rate = icr->dMaxRate / 1'000'000.; // переводим в МГц
            break;
        }

        case DACKHZ_CFG_TAG: { // конфигурационная структура ЦАП (частоты выражены в килогерцах)
            auto icr = reinterpret_cast<ICR_Cfg0311 const*>(eeprom);
            m_srv_dac.m_bits = icr->bBits;
            m_srv_dac.m_encoding = icr->bEncoding;
            m_srv_dac.m_out_range = icr->wRange / 1'000.; // переводим в вольты
            m_srv_dac.m_min_rate = icr->dMinRate / 1'000'000.; // переводим в МГц
            m_srv_dac.m_max_rate = icr->dMaxRate / 1'000'000.; // переводим в МГц
            break;
        }

        case SUBMODULE_CFG_TAG: { // конфигурационная структура субмодуля
            m_adc_count = *reinterpret_cast<uint8_t const*>(eeprom + 5);
            m_dac_count = *reinterpret_cast<uint8_t const*>(eeprom + 6);
            m_fosc = *reinterpret_cast<uint32_t const*>(eeprom + 8) / 1'000.0; // переводим в МГц
            m_start_level = *reinterpret_cast<uint16_t const*>(eeprom + 32) / 1'000.0; // переводим в вольты
            m_adc_min_bitrate = *reinterpret_cast<uint32_t const*>(eeprom + 16);
            m_adc_max_bitrate = *reinterpret_cast<uint32_t const*>(eeprom + 20);
            m_dac_min_bitrate = *reinterpret_cast<uint32_t const*>(eeprom + 24);
            m_dac_max_bitrate = *reinterpret_cast<uint32_t const*>(eeprom + 28);
            m_pll_present = *reinterpret_cast<uint8_t const*>(eeprom + 14);
            m_adc_present = *reinterpret_cast<uint8_t const*>(eeprom + 12);
            m_dac_present = *reinterpret_cast<uint8_t const*>(eeprom + 13);
        }
        }

        eeprom += icr_base->wSize + sizeof(icr_base->wTag) + sizeof(icr_base->wSize);
    } while (eeprom < eeprom_end);
}

///
/// \brief      Вывод ICR-байтов в лог (EEPROM)
///
/// \param      mem   Указатель на массив байтов EEPROM
/// \param      len   Длина массива
///
/// \return     void
///
auto Cmfm214x3gda::dump_icr(uint8_t* mem, size_t len) -> void
{
    constexpr auto width = size_t { 16 };

    for (auto row = 0u; row < len / width + ((len % width) ? 1 : 0); row++) {
        LOG_WITHOUT_NEWLINE("- ");
        LOG_WITHOUT_NAMECLASS("0x%05zX: ", row * width); // локальный адрес
        auto column = 0u;
        for (column = 0u; column < std::min(width, len - (row * 16u)); column++)
            LOG_WITHOUT_NAMECLASS("%02X ", mem[(row * width) + column]); // байты
        for (; column < 16; column++) // отступы для последней не полной строки
            LOG_WITHOUT_NAMECLASS("   ");
        LOG_WITHOUT_NAMECLASS("| "); // разделитель байты | символы
        for (column = 0u; column < std::min(width, len - (row * 16u)); column++)
            LOG_WITHOUT_NAMECLASS("%c", isprint(mem[(row * width) + column]) ? mem[(row * width) + column] : '.'); // символы
        LOG_WITHOUT_NAMECLASS("\n");
    }
}

///
/// \brief      Обработка команды `CMD_getinfo`. Необходимо заполнить некоторую
///             информацию
///
/// \param      pInfo  Указатель на информационную структуру базовой платы
///
/// \return     void
///
auto Cmfm214x3gda::GetDeviceInfo(BRD_Info* pInfo) -> void
{
    LOG("CMD_getinfo (pInfo: %p)", pInfo);

    // pInfo->name = m_name;
    BRDC_strcpy(pInfo->name, m_name);
    pInfo->verMajor = VER_MAJOR;
    pInfo->verMinor = VER_MINOR;
    pInfo->boardType = (m_type << 16) | m_version;
    pInfo->pid = m_PID;

    // LOG("- size: " << pInfo->size);
    // LOG("- code: " << pInfo->code);
    LOG("- boardType: %X", pInfo->boardType);
    LOG("- name: '%s'", BARDY_STR(pInfo->name));
    LOG("- pid: %zu (%zX)", size_t(pInfo->pid), size_t(pInfo->pid));
    // LOG("- busType: " << pInfo->busType);
    // LOG("- bus: " << pInfo->bus);
    // LOG("- dev: " << pInfo->dev);
    // LOG("- slot: " << pInfo->slot);
    LOG("- verMajor: %zX", size_t(pInfo->verMajor));
    LOG("- verMinor: %zX", size_t(pInfo->verMinor));
    // LOG("- subunitType[16]: " << pInfo->subunitType[0]);
    // LOG("- base[8]: " << pInfo->base[0]);
    // LOG("- vectors[4]: " << pInfo->vectors[0]);
};

///
/// \brief      Обработка команды `CMD_init` Получаем информацию о базовой
///             плате, которая необходима в функции `RegCtrl` для пересылки
///             команд `DEVScmd_*`. Также производим инициализацию тетрад и
///             микросхем, для которых не будут созданы службы
///
/// \param      pDev    Адрес базового модуля
/// \param      pEntry  Адрес входной функции базового модуля
/// \param      pName   Имя базового модуля
///
/// \return     void
///
auto Cmfm214x3gda::BaseInfoInit(void* pDev, DEVS_API_entry* pEntry, BRDCHAR* pName) -> void
{
    LOG("CMD_init (pDev: %p, pEntry: %p, pName: '%s')", pDev, pEntry, BARDY_STR(pName));

    // запоминаем адрес объекта, точку входа и имя базовой платы
    base_dev = static_cast<CBaseModule*>(pDev); // CBaseModule *
    base_entry = pEntry; // адрес функции для отправления команд `DEVScmd_*`
    BRDC_strcpy(base_name, pName); // имя

    // 0x00C : VendorID [7:0]  = 0x56
    // 0x00D : VendorID [15:8] = 0x04
    // SpdWrite(m_sync_num,0,0,0x00, 0x18); // SPI init
    // LOG("- AD9528 VendorID: %02zX %02zX", SpdRead(m_sync_num,0,0,0x0C), SpdRead(m_sync_num,0,0,0x0D));

    // Инициализация механизма запуска двух exam'ов одновременно
    // 1. Проверяется файл блокировки на присутствие в системе
    // 2. Если файла нет, создаём его
    // 3. Пытаемся монопольно захватить файл (LOCK)
    // 4. Если захват прошёл успешно, то мы управляем SYNC-тетрадой
    // 5. ... во всех других случаях считаем тетраду SYNC уже настроенной
    m_sync_fname_lock = BARDY_STR(base_name) + std::string("_lock.tmp");
    m_sync_fname_data = BARDY_STR(base_name) + std::string("_data.tmp");
    auto f = fopen(m_sync_fname_lock.c_str(), "r");
    if (f == nullptr) {
        f = fopen(m_sync_fname_lock.c_str(), "w");
        if (f == nullptr) {
            LOG("ERROR: No create file `%s`", m_sync_fname_lock.c_str());
            return;
        }
        fclose(f);
    }
    m_sync_lock = boost::interprocess::file_lock(m_sync_fname_lock.c_str());
    m_sync_slave = !m_sync_lock.try_lock(); // false: текущий процесс управляет SYNC тетрадой; true: SYNC тетрада уже настроена
}

///
/// \brief      Пересылка команд базовой плате `DEVScmd_*`
///
/// \param      cmd        Номер команды из `DEVScmd_*`
/// \param      pRegParam  Структура с параметрами команды
///
/// \return     Результат выполнения команды на базовой плате
///
auto Cmfm214x3gda::RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam) -> LONG
{
#if defined(DEBUG) && defined(DEBUG_REGCTRL)
    auto cmd_str = std::string {};
    switch (cmd) {
    case 0:
        cmd_str = "REGREADDIR";
        break;
    case 1:
        cmd_str = "REGREADSDIR";
        break;
    case 2:
        cmd_str = "REGREADIND";
        break;
    case 3:
        cmd_str = "REGREADSIND";
        break;
    case 4:
        cmd_str = "REGWRITEDIR";
        break;
    case 5:
        cmd_str = "REGWRITESDIR";
        break;
    case 6:
        cmd_str = "REGWRITEIND";
        break;
    case 7:
        cmd_str = "REGWRITESIND";
        break;
    case 8:
        cmd_str = "SETSTATIRQ";
        break;
    case 9:
        cmd_str = "CLEARSTATIRQ";
        break;
    case 10:
        cmd_str = "GETBASEADR";
        break;
    case 11:
        cmd_str = "WAITSTATIRQ";
        break;
    case 12:
        cmd_str = "PACKEXECUTE";
        break;
    default:
        cmd_str = "NONE";
    }
    LOG("RegCtrl (cmd: 0x%zX 'DEVScmd_%s', pRegParam: %p)", size_t(cmd), cmd_str.c_str(), pRegParam);
#endif

    return (base_entry)(base_dev, nullptr, cmd, pRegParam);
}

///
/// \brief      Пересылка команд службе
///
/// \param      handle    Хендл службы ???
/// \param      cmd       Номер команды из `SERVcmd_*` или `BRDctrl_*`
/// \param      arg       Адрес блока параметров
/// \param      pContext  The context
///
/// \return     { description_of_the_return_value }
///
auto Cmfm214x3gda::SrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, void* pContext) -> ULONG
{
#if defined(DEBUG) && defined(DEBUG_SRVCTRL)
    auto cmd_str = std::string {};
    switch (cmd) {
    case 1:
        cmd_str = "SERVcmd_SYS_ISAVAILABLE";
        break;
    case 2:
        cmd_str = "SERVcmd_SYS_CAPTURE";
        break;
    case 3:
        cmd_str = "SERVcmd_SYS_RELEASE";
        break;
    case 4:
        cmd_str = "SERVcmd_SYS_GETADDRDATA";
        break;
    default:
        switch (cmd >> 8) {
        case 8:
            cmd_str = "BRDctrl_DAQ";
            break; // DAQ control
        case 9:
            cmd_str = "BRDctrl_CMN";
            break; // common control parameters
        case 10:
            cmd_str = "BRDctrl_ADC";
            break; // ADC control parameters
        case 11:
            cmd_str = "BRDctrl_DAC";
            break; // DAC control parameters
        case 12:
            cmd_str = "BRDctrl_DDC";
            break; // DDC control parameters
        case 13:
            cmd_str = "BRDctrl_PIO";
            break; // PIO control parameters
        case 14:
            cmd_str = "BRDctrl_STREAM";
            break; // STREAM control parameters
        case 15:
            cmd_str = "BRDctrl_SVEAM";
            break; // SVEAM control parameters
        case 16:
            cmd_str = "BRDctrl_DIOIN";
            break; // DIOIN control parameters
        case 17:
            cmd_str = "BRDctrl_DIOOUT";
            break; // DIOOUT control parameters
        case 18:
            cmd_str = "BRDctrl_SDRAM";
            break; // SDRAM control parameters
        case 19:
            cmd_str = "BRDctrl_DSPNODE";
            break; // DSP node control parameters
        case 20:
            cmd_str = "BRDctrl_TEST";
            break; // Test control parameters
        case 21:
            cmd_str = "BRDctrl_FOTR";
            break; // FOTR control parameters
        case 22:
            cmd_str = "BRDctrl_QM";
            break; // QM9857 control parameters
        case 23:
            cmd_str = "BRDctrl_GC5016";
            break; // GC5016 control parameters
        case 24:
            cmd_str = "BRDctrl_DDS";
            break; // DDS control parameters
        case 25:
            cmd_str = "BRDctrl_SYNCDAC";
            break; // SyncDac control parameters
        case 26:
            cmd_str = "BRDctrl_COMMON";
            break; // real common control area
        case 27:
            cmd_str = "BRDctrl_NET";
            break; // net (still EMAC) control parameters
        default:
            cmd_str = "NONE";
        }
    }
    LOG("SrvCtrl (handle: %zu, cmd: 0x%zX '%s %zu', arg: %p, pContext: %p)",
        size_t(handle), size_t(cmd), cmd_str.c_str(), cmd > 4 ? size_t(cmd & 0xFF) : 0, arg, pContext);
#endif

    return CModule::SrvCtrl(handle, cmd, arg, pContext);
}

///
/// \brief      Обработка команды `CMD_getServList`. Заполняем массив служб
///             информацией для регистрации: имя, аттрибуты и индекс,
///             присвоенные в конструкторе.
///
/// \param      srv_list  Адрес массива созданных нами служб
///
/// \return     void
///
auto Cmfm214x3gda::GetServ(SERV_ListItem srv_list[]) -> void
{
    LOG("CMD_getServList (srv_list: %p, size: %zu)", srv_list, size_t(m_SrvList.size()));

    // заполение массива информацией о службах
    for (auto i = 0U; i < m_SrvList.size(); i++) {
        BRDC_strcpy(srv_list[i].name, m_SrvList[i]->GetName()); // return m_name
        srv_list[i].attr = m_SrvList[i]->GetAttribute(); // return m_attribute
        srv_list[i].idxMain = m_SrvList[i]->GetIndex(); // return m_index
        LOG("- [%02u] name: %s, attr: %zX, index: %zd",
            i, BARDY_STR(srv_list[i].name), size_t(srv_list[i].attr), ssize_t(srv_list[i].idxMain));
    }
}

///
/// \brief      Вызывается по результату регистрации служб функцией `GetServ()`.
///             К имени службы добавляется порядковый номер и присваивается
///             правильный индекс, вместо -1. Этой информацией необходимо
///             перезаписать значения, которые хранятся в массиве `m_SrvList`.
///
/// \param      srv_list  Адрес структуры службы
///
/// \return     void
///
auto Cmfm214x3gda::SetSrvList(SERV_ListItem srv_list[]) -> void
{
    LOG("SetSrvList (srv_list: %p, size: %zu)", srv_list, size_t(m_SrvList.size()));

    for (auto i = 0U; i < m_SrvList.size(); i++) {
        m_SrvList[i]->SetName(srv_list[i].name);
        m_SrvList[i]->SetIndex(srv_list[i].idxMain);

        // Создать в службе разделяемую память с указателем
        // void* CServise::m_pConfig и скопировать в нее целиком
        // структуру SRV_CFG
        // SRV_CFG cfg;
        // m_SrvList[i]->SetCfgMem(&cfg, sizeof(SRV_CFG));

        // вызываем с пустой структурой конфигурации,
        // для создания mutex'ов, которые защищают вызов `DoCmd`
        m_SrvList[i]->SetCfgMem(nullptr, 0);

        LOG("- [%02u] name: %s, attr: %zX, index: %zd",
            i, BARDY_STR(srv_list[i].name), size_t(srv_list[i].attr), ssize_t(srv_list[i].idxMain));
    }
}

///
/// \brief      Чтение косвенного регистра тетрады
///
/// \param      tetr_num  Порядковый номер тетрады
/// \param      reg_num   Номер регистра
///
/// \return     Значение из регистра
///
auto Cmfm214x3gda::IndRegRead(size_t tetr_num, size_t reg_num) -> size_t
{
    DEVS_CMD_Reg param {};
    param.tetr = tetr_num;
    param.reg = reg_num;
    (base_entry)(base_dev, nullptr, DEVScmd_REGREADIND, &param);
    // dch
    // printf("<RD_IND> Tetr=%d  Reg=%X  Val=%X\n", tetr_num, reg_num, param.val);
    return param.val;
}

///
/// \brief      Запись косвенного регистра тетрады
///
/// \param      tetr_num  Порядковый номер тетрады
/// \param      reg_num   Номер регистра
/// \param      val       Значение для записи в регистр
///
/// \return     void
///
auto Cmfm214x3gda::IndRegWrite(size_t tetr_num, size_t reg_num, size_t val) -> void
{
    DEVS_CMD_Reg param {};
    param.tetr = tetr_num;
    param.reg = reg_num;
    param.val = val;
    (base_entry)(base_dev, nullptr, DEVScmd_REGWRITEIND, &param);
    // dch
    // printf("<WR_IND> Tetr=%d  Reg=%X  Val=%X\n", tetr_num, reg_num, val);
}

///
/// \brief      Чтение прямого регистра тетрады
///
/// \param      tetr_num  Порядковый номер тетрады
/// \param      reg_num   Номер регистра
///
/// \return     Значение из регистра
///
auto Cmfm214x3gda::DirRegRead(size_t tetr_num, size_t reg_num) -> size_t
{
    DEVS_CMD_Reg param {};
    param.tetr = tetr_num;
    param.reg = reg_num;
    (base_entry)(base_dev, nullptr, DEVScmd_REGREADDIR, &param);
    // dch
    // printf("<RD_DIR> Tetr=%d  Reg=%X  Val=%X\n", tetr_num, reg_num, param.val);
    return param.val;
}

///
/// \brief      Запись прямого регистра тетрады
///
/// \param      tetr_num  Порядковый номер тетрады
/// \param      reg_num   Номер регистра
/// \param      val       Значение для записи в регистр
///
/// \return     void
///
auto Cmfm214x3gda::DirRegWrite(size_t tetr_num, size_t reg_num, size_t val) -> void
{
    DEVS_CMD_Reg param {};
    param.tetr = tetr_num;
    param.reg = reg_num;
    param.val = val;
    (base_entry)(base_dev, nullptr, DEVScmd_REGWRITEDIR, &param);
    // dch
    // printf("<WR_DIR> Tetr=%d  Reg=%X  Val=%X\n", tetr_num, reg_num, val);
}

///
/// \brief      Запись нескольких значений в прямой регистр тетрады
///
/// \param      tetr_num  Порядковый номер тетрады
/// \param      reg_num   Номер регистра
/// \param      buf       Адрес буфера для записи в регистр
/// \param      size      Размер буфера
///
/// \return     void
///
auto Cmfm214x3gda::DirRegWrites(size_t tetr_num, size_t reg_num, void* buf, size_t size) -> void
{
    DEVS_CMD_Reg param {};
    // param.idxMain = index;
    param.tetr = tetr_num;
    param.reg = reg_num;
    param.pBuf = buf;
    param.bytes = size;
    (base_entry)(base_dev, nullptr, DEVScmd_REGWRITESDIR, &param);
    // dch
    // printf("<WR_DIR_BLK> Tetr=%d  Reg=%X  size=%X ...\n", tetr_num, reg_num, size);
}

///
/// \brief      Чтение регистра устройства на SPD шине конкретной тетрады
///
/// \param      tetr_num  Порядковый номер тетрады
/// \param      dev       Номер устройства/группы
/// \param      numb      Номер устройства в группе
/// \param      addr      Адрес регистра
///
/// \return     Значение из регистра
///
auto Cmfm214x3gda::SpdRead(size_t tetr_num, size_t dev, size_t numb, size_t addr) -> size_t
{
    auto status = size_t {};

    do { // ожидаем готовность тетрады
        status = DirRegRead(tetr_num, ADM2IFnr_STATUS);
    } while ((status & 1) != 1);

    IndRegWrite(tetr_num, ADM2IFnr_SPDDEVICE, dev);
    IndRegWrite(tetr_num, ADM2IFnr_SPDADDR, addr);
    IndRegWrite(tetr_num, ADM2IFnr_SPDCTRL, 1 | (numb << 4));

    do { // ожидаем готовность тетрады
        status = DirRegRead(tetr_num, ADM2IFnr_STATUS);
    } while ((status & 1) != 1);

    return IndRegRead(tetr_num, ADM2IFnr_SPDDATAL) | IndRegRead(tetr_num, ADM2IFnr_SPDDATAH) << 16;
}

///
/// \brief      Запись в регистр устройства на SPD шине конкретной тетрады
///
/// \param      tetr_num  Порядковый номер тетрады
/// \param      dev       Номер устройства/группы
/// \param      numb      Номер устройства в группе
/// \param      addr      Адрес регистра
/// \param      val       Значение для записи в регистр
///
/// \return     void
///
auto Cmfm214x3gda::SpdWrite(size_t tetr_num, size_t dev, size_t numb, size_t addr, size_t val) -> void
{
    auto status = size_t {};

    do { // ожидаем готовность тетрады
        status = DirRegRead(tetr_num, ADM2IFnr_STATUS);
    } while ((status & 1) != 1);

    IndRegWrite(tetr_num, ADM2IFnr_SPDDEVICE, dev);
    IndRegWrite(tetr_num, ADM2IFnr_SPDADDR, addr);
    IndRegWrite(tetr_num, ADM2IFnr_SPDDATAL, val & 0xFFFF);
    IndRegWrite(tetr_num, ADM2IFnr_SPDDATAH, val >> 16);
    IndRegWrite(tetr_num, ADM2IFnr_SPDCTRL, 2 | (numb << 4));

    do { // ожидаем готовность тетрады
        status = DirRegRead(tetr_num, ADM2IFnr_STATUS);
    } while ((status & 1) != 1);
}
