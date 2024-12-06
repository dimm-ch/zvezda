///
/// Author:  Alexander U.Jurankov
/// E-mail:  devlab@insys.ru
/// Company: InSYS 2021
///
#include <algorithm>
#include <codecvt>
#include <cstdio>
#include <iostream>
#include <locale>
#include <string>
#include <thread>
#include <vector>

#include <INIReader.h>
#include <brd.h>
#include <ctrladmpro/ctrl_jesd.h>
#include <ctrladmpro/ctrladc.h>
#include <logger.hpp>

auto device = BRD_Handle {}; // хенд устройства

// '►':0x10, '‼':0x13, '∙':0xF9, '→':0x1A, '█':0xDB
insys::logger logger { insys::logger::format("{green+}", '\xF9') };
insys::logger logger_err { insys::logger::format("{red+}", "\xDB ERROR:") };

auto BRDC_main(int argc, char* argv[]) -> int
try {
    logger.enable_debug();

    // параметр: количество циклов
    int counts = 0;
    if (argc == 1)
        counts = 20; // default
    else
        sscanf(argv[1], "%d", &counts);

    // инициализируем библиотеку BARDY, используя файл
    if (auto err = BRD_init(_BRDC("brd.ini"), nullptr); BRD_errcmp(err, BRDerr_OK) == false)
        throw std::runtime_error(insys::logger::format("Not loading `brd.ini` file [{x}]", uint32_t(err)));

    // инициализируем библиотеку BARDY, используя строку
    // int32_t lidcount {};
    // constexpr auto brd_ini = _BRDC("[LID:1]\ntype=bambpex\npid=0\n#begin SUBUNIT\ntype=mfm414x3g\n#end");
    // if (auto err = BRD_initEx(BRDinit_STRING, brd_ini, nullptr, &lidcount); BRD_errcmp(err, BRDerr_OK) == false)
    //     throw std::runtime_error(insys::logger::format("Uninitialized BARDY library [{x}]", uint32_t(err)));

    // откроем устройство в разделяемом режиме
    device = BRD_open(1 /*LID 1*/, BRDopen_SHARED, nullptr);
    if (device < 1)
        throw std::runtime_error(insys::logger::format("Not open device LID 1 [{x}]", uint32_t(device)));
    logger.println("Open device LID 1");

    // поиск службы ADC среди всех доступных служб
    uint32_t count = 0;
    BRD_serviceList(device, 0, nullptr, 0, &count);
    auto list = std::vector<BRD_ServList>(count);
    BRD_serviceList(device, 0, list.data(), list.size(), &count);

    BRD_ServList capture_srv {}; // служба для захвата
    logger.println("Service List:");
    for (auto const& item : list) {
#if defined(UNICODE)
        if (std::wstring(reinterpret_cast<BRDCHAR const*>(&item.name)).find(L"ADC") != std::wstring::npos) {
#else
        if (std::string(reinterpret_cast<BRDCHAR const*>(&item.name)).find("ADC") != std::string::npos) {
#endif
            capture_srv = item;
            logger.println("  [{:04x}] {yellow+}", item.attr, item.name);
        } else
            logger.println("  [{:04x}] {}", item.attr, item.name);
    }

    // цикл инициализаций, для сбора статистики
    std::vector<size_t> latency_array {};
    size_t LMFCCountCoeff {};
    logger.println("Running {yellow+} launches ...", counts);
    for (auto i = 0; i < counts; i++) {
        // захватим службу ADC
        // logger.println("Capture {yellow+} Service", capture_srv.name);
        auto mode = uint32_t(BRDcapt_SHARED); // режим захвата служб
        auto adc = BRD_capture(device, 0, &mode, capture_srv.name, 1000);
        if (adc < 1)
            throw std::runtime_error(insys::logger::format("Not capture {yellow+} [{x}]", capture_srv.name, uint32_t(adc)));

        // берём настройки из файла `exam_adc.ini`, секции `device0_..ADC..`
        // logger.println("Programing {yellow+} from `ini` file", capture_srv.name);
        BRD_IniFile ini { _BRDC("./exam_adc.ini"), _BRDC("device0_") };
        BRDC_strcat(ini.sectionName, capture_srv.name);
        if (auto err = BRD_ctrl(adc, 0, BRDctrl_ADC_READINIFILE, &ini); BRD_errcmp(err, BRDerr_OK) == false)
            throw std::runtime_error(insys::logger::format("Not read `exam_adc.ini` file [{x}]", uint32_t(err)));

        // запуск программирования SPD-регистров на прямую
        // параметр RegFileName в ini-файле
        INIReader reader { "./exam_adc.ini" };
#if defined(UNICODE)
        auto section_name = std::wstring(reinterpret_cast<BRDCHAR const*>(&ini.sectionName));
#else
        auto section_name = std::string(reinterpret_cast<BRDCHAR const*>(&ini.sectionName));
#endif
        auto ini_fname = reader.Get(std::string(section_name.cbegin(), section_name.cend()), "RegFileName", "");
        if (ini_fname.empty() == false) {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring wstr = converter.from_bytes(ini_fname.c_str());
            S32 RegRwSpd(BRD_Handle hDev, BRDCHAR * fname);
            RegRwSpd(adc, (BRDCHAR*)wstr.c_str());
        }

        // выставим, принудительно, программный старт
        auto start_mode = BRD_StartMode {};
        BRD_ctrl(adc, 0, BRDctrl_ADC_SETSTARTMODE, &start_mode);

        BRD_JesdLmfcCount jesd_lmfc_count {};
        BRD_ctrl(adc, 0, BRDctrl_JESD_SETLMFCCOUNT, &jesd_lmfc_count);

        // сбросим FIFO и запустим службу ADC
        auto enable { 1 };
        BRD_ctrl(adc, 0, BRDctrl_ADC_FIFORESET, nullptr);
        BRD_ctrl(adc, 0, BRDctrl_ADC_ENABLE, &enable);

        BRD_ctrl(adc, 0, BRDctrl_JESD_GETLMFCCOUNT, &jesd_lmfc_count);

        // остановим службу ADC
        enable = 0;
        BRD_ctrl(adc, 0, BRDctrl_ADC_ENABLE, &enable);

        latency_array.push_back(jesd_lmfc_count.count);
        LMFCCountCoeff = (jesd_lmfc_count.K * jesd_lmfc_count.F) >> 2;
        logger.println("[{:02}] Get delay latency: {yellow+:2}, coeff: {yellow+}", i + 1, jesd_lmfc_count.count, LMFCCountCoeff);

        // получим JESD статусы
        uint32_t status_jesd_link_error = 0;
        BRD_ctrl(adc, 0, BRDctrl_JESD_GETLINKERRORSTATUS, &status_jesd_link_error);
        uint32_t status_jesd_sync = 0;
        BRD_ctrl(adc, 0, BRDctrl_JESD_GETSYNCSTATUS, &status_jesd_sync);
        uint32_t status_jesd_debug = 0;
        BRD_ctrl(adc, 0, BRDctrl_JESD_GETDEBUGSTATUS, &status_jesd_debug);
        logger.println("[{:02}] JESD Status | Link Error: {yellow+:08x}, Sync: {yellow+:04x}, Debug: {yellow+:08x},", i + 1,
            status_jesd_link_error, status_jesd_sync, status_jesd_debug);

        // отпустим службу
        BRD_release(adc, 0);
    }
    logger.println("");

    for (auto delay : latency_array)
        logger.print("{}", delay);
    logger.println("");

    auto max = std::max_element(latency_array.begin(), latency_array.end());
    logger.println("LMFCCount: {yellow+}", latency_array.at(std::distance(latency_array.begin(), max)) + LMFCCountCoeff);

    // правильный выход :-)
    BRD_close(device); // закроем устройство
    BRD_cleanup(); // и библиотеку BARDY
    logger.println("Press ENTER to exit ...");
    std::cin.get();
    std::exit(EXIT_SUCCESS);
} catch (std::exception const& e) {
    logger_err.enable_debug();
    logger_err.println("{}", e.what());
    BRD_close(device); // закроем устройство
    BRD_cleanup(); // и библиотеку BARDY
    logger.println("Press ENTER to exit ...");
    std::cin.get();
    std::exit(EXIT_FAILURE);
}
