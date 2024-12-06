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
#include <ctrladmpro/ctrldac.h>
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

    // поиск службы DAC среди всех доступных служб
    uint32_t count = 0;
    BRD_serviceList(device, 0, nullptr, 0, &count);
    auto list = std::vector<BRD_ServList>(count);
    BRD_serviceList(device, 0, list.data(), list.size(), &count);

    BRD_ServList capture_srv {}; // служба для захвата
    logger.println("Service List:");
    for (auto const& item : list) {
#if defined(UNICODE)
        if (std::wstring(reinterpret_cast<BRDCHAR const*>(&item.name)).find(L"DAC") != std::wstring::npos) {
#else
        if (std::string(reinterpret_cast<BRDCHAR const*>(&item.name)).find("DAC") != std::string::npos) {
#endif
            capture_srv = item;
            logger.println("  [{:04x}] {yellow+}", item.attr, item.name);
        } else
            logger.println("  [{:04x}] {}", item.attr, item.name);
    }

    // цикл инициализаций, для сбора статистики
    size_t LMFCCountCoeff {};
    std::vector<size_t> counts_array {};
    double PCLKsperMF {};
    std::vector<size_t> latency_array {};

    logger.println("Running {yellow+} launches ...", counts);
    for (auto i = 0; i < counts; i++) {
        // захватим службу DAC
        // logger.println("Capture {yellow+} Service", capture_srv.name);
        auto mode = uint32_t(BRDcapt_SHARED); // режим захвата служб
        auto dac = BRD_capture(device, 0, &mode, capture_srv.name, 1000);
        if (dac < 1)
            throw std::runtime_error(insys::logger::format("Not capture {yellow+} [{x}]", capture_srv.name, uint32_t(dac)));

        // берём настройки из файла `exam_DAC.ini`, секции `device0_..DAC..0`
        // logger.println("Programing {yellow+} from `ini` file", capture_srv.name);
        BRD_IniFile ini { _BRDC("./exam_edac.ini"), _BRDC("device0_") };
        BRDC_strcat(ini.sectionName, capture_srv.name);
        if (auto err = BRD_ctrl(dac, 0, BRDctrl_DAC_READINIFILE, &ini); BRD_errcmp(err, BRDerr_OK) == false)
            throw std::runtime_error(insys::logger::format("Not read `exam_edac.ini` file [{x}]", uint32_t(err)));

        // для правильного расчёта необходимо игнорировать в ini-файле
        // калибровочные значения для JESD линков (Delay и Variable Delay Buffer)
        // их необходимо установить в ноль
        auto jesd_lmfc = BRD_JesdLmfc {};
        BRD_ctrl(dac, 0, BRDctrl_JESD_SETLMFC, &jesd_lmfc);

        // запуск программирования SPD-регистров на прямую
        // параметр RegFileName в ini-файле
        INIReader reader { "./exam_edac.ini" };
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
            RegRwSpd(dac, (BRDCHAR*)wstr.c_str());
        }

        // выставим, принудительно, программный старт
        auto start_mode = BRD_StartMode {};
        BRD_ctrl(dac, 0, BRDctrl_DAC_SETSTARTMODE, &start_mode);

        BRD_JesdLmfcCount jesd_lmfc_count {};
        BRD_ctrl(dac, 0, BRDctrl_JESD_SETLMFCCOUNT, &jesd_lmfc_count);

        // сбросим FIFO и запустим службу DAC
        auto enable { 1 };
        BRD_ctrl(dac, 0, BRDctrl_DAC_FIFORESET, nullptr);
        BRD_ctrl(dac, 0, BRDctrl_DAC_ENABLE, &enable);

        // чтение latency delay для JESD линков
        auto latency = BRD_JesdLmfcLatency {};
        BRD_ctrl(dac, 0, BRDctrl_JESD_GETLMFC, &latency);
        BRD_ctrl(dac, 0, BRDctrl_JESD_GETLMFCCOUNT, &jesd_lmfc_count);

        // остановим службу DAC
        enable = 0;
        BRD_ctrl(dac, 0, BRDctrl_DAC_ENABLE, &enable);

        // обработка полученной информации
        PCLKsperMF = latency.K / (4. / latency.F);
        // if (latency.delay_link0 == 0 || latency.delay_link0 == 1) latency.delay_link0 += PCLKsperMF;
        // if (latency.delay_link1 == 0 || latency.delay_link1 == 1) latency.delay_link1 += PCLKsperMF;
        latency_array.push_back(latency.delay_link0);
        latency_array.push_back(latency.delay_link1);
        // printf("    [%02d] Get delay latency: %2zd, %2zd\r", i, latency.delay_link0, latency.delay_link1);

        counts_array.push_back(jesd_lmfc_count.count);
        LMFCCountCoeff = (jesd_lmfc_count.K * jesd_lmfc_count.F) >> 2;
        logger.println("[{:02}] Get delay latency: {yellow+:2}, coeff: {yellow+} ({:2}, {:2})", i + 1, jesd_lmfc_count.count, LMFCCountCoeff, latency.delay_link0, latency.delay_link1);

        // отпустим службу
        BRD_release(dac, 0);
    }
    logger.println("");

    logger.print("JESD Counts:");
    for (auto item : counts_array)
        logger.print("{}", item);
    logger.println("");

    {
        auto max = std::max_element(counts_array.begin(), counts_array.end());
        logger.println("LMFCCount: {yellow+}", counts_array.at(std::distance(counts_array.begin(), max)) + LMFCCountCoeff);
    }

    logger.print("DAC LMFC Delays:");
    for (auto item : latency_array)
        logger.print("{}", item);
    logger.println("");

    auto min_max = std::minmax_element(begin(latency_array), end(latency_array));
    auto min = *min_max.first;
    auto max = *min_max.second;

    if ((max - min) < (PCLKsperMF / 2)) {
        min += PCLKsperMF;
        max += PCLKsperMF;
    } else {
        min += PCLKsperMF;
        auto tt = max;
        max = min;
        min = tt;
    }
    logger.println("min: {yellow+}, max: {yellow+}, PCLKsperMF: {yellow+}", min, max, size_t(PCLKsperMF));

    auto LMFCVar = (max + 2) - (min - 2);
    auto LMFCDel = (min - 2) % size_t(PCLKsperMF);
    logger.println("LMFCVar: {yellow+}, LMFCDel: {yellow+}", LMFCVar, LMFCDel);

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
