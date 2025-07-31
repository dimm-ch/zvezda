#include <chrono>
#include <clocale>
#include <csignal>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// #include "INIReader.h"
#include "../../contribs/INIReader/include/INIReader.h"
#include "brdapi.h"
#include "brdexam.h"
#include "concol.h"
#include "ctrlreg.h"
#include "fmc146v_sync.h"

#if __cplusplus < 201402L
#error \
    "This file requires compiler and library support for the ISO C++ 2014 standard or higher."
#endif

using namespace concol;
using namespace concol_literals;
using namespace chappi;
using namespace std::string_literals;
using namespace InSys;
using namespace InSys::Bardy;
using namespace InSys::Bardy::Registers;

// static constexpr auto g_BrdIniFileNameDefault { ("brd.ini") };
// static constexpr auto g_SettingsFileNameDefault { "exam_fmc146v_sync.ini" };

// FIXME: управление на прямую через службу Reg
// FIXME: временный (для отладки) вариант управления
// FIXME: поддерживается управление одной платой FMC146V

#define ERROR_ASSERT_X(success, message)                            \
    if (!(success)) {                                               \
        concol::color::printf("{red}Error:{+red} " message "{}\n"); \
        awaitExit();                                                \
    }

auto getDevInfoList(S32 DevNum)
{
    std::vector<uint32_t> lidArray {};
    BRD_LidList brdLidList {};
    lidArray.resize(DevNum);
    brdLidList.item = uint32_t(lidArray.size());
    brdLidList.pLID = lidArray.data();
    auto status = BRD_lidList(brdLidList.pLID, brdLidList.item, &brdLidList.itemReal);
    if (!BRD_errcmp(status, BRDerr_OK)) {
        throw brd_runtime_error { status, "Get LID list error!" };
    }
    lidArray.resize(brdLidList.itemReal);
    brdLidList.item = uint32_t(lidArray.size());
    BRD_Info brdinfo {};
    brdinfo.size = sizeof(brdinfo);
    CDeviceInfoList deviceInfoList {};
    for (auto& lid : lidArray) {
        status = BRD_getInfo(lid, &brdinfo);
        if (!BRD_errcmp(status, BRDerr_OK)) {
            throw brd_runtime_error { status, "Get device info error!" };
        }
        deviceInfoList.emplace_back(lid, brdinfo);
    }
    return deviceInfoList;
}

void startFmc(const std::string& inifile, S32 DevNum)
{
    // устанавливаем обработчик запроса завершения программы
    std::set_terminate(terminateHandler);

    std::signal(SIGINT, abortHandler);
    std::signal(SIGILL, abortHandler);
    std::signal(SIGFPE, abortHandler);
    std::signal(SIGSEGV, abortHandler);
    std::signal(SIGTERM, abortHandler);
    std::signal(SIGABRT, abortHandler);

    try {

        // инициализация работы с клавиатурой
        CKeyboard keyboard {};
        bool verboseInfo = true;
        color::set_enabled(true);
        // получаем информацию об обнаруженных устройствах
        const auto deviceInfoList = getDevInfoList(DevNum);
        // список устройств
        std::vector<CDevice> deviceList {};
        color::printf("Devices:\n"_white_bright);
        // поиск нужного устройства в списке устройств
        for (auto& deviceInfo : deviceInfoList) {
            if (brd_string { deviceInfo.name } == brd_string { _BRDC("FMC146V Zynq") }
                || brd_string { deviceInfo.name } == brd_string { _BRDC("FMC146VZQ") }) {
                // получаем управление базовой платой и добавляем в список устройств
                deviceList.push_back(CBardy::getDevice(deviceInfo));
                // выводим LID и имя устройства
                color::printf("\t{+cyan}[%d] {+white}%s{}\n", deviceInfo.lid, std::to_string(deviceInfo.name).c_str());
            } else {
                // выводим LID и имя устройства
                color::printf("\t{+black}[%d] %s{}\n", deviceInfo.lid, std::to_string(deviceInfo.name).c_str());
            }
        }
        ERROR_ASSERT_X(deviceList.empty() != true, "No devices found!");
        // список обнаруженных узлов SYNC платы FMC146V
        std::vector<CFmc146vSync> fmc146vSyncList {};
        // делаем обход всех устройств
        for (const auto& device : deviceList) {
            // получаем список служб устройства
            const auto serviceInfoList = device->getServiceInfoList();
            // ищем службу REG0
            auto itServiceInfo = serviceInfoList.find(CRegService::BaseName, 0);
            if (itServiceInfo != serviceInfoList.cend()) {
                // служба REG0 найдена
                auto regService = device->getService<CRegService>(*itServiceInfo);
                // проверяем есть ли в устройстве узел SYNC
                if (regService.findTetr(CFmc146vSync::TETR_ID)) {
                    // создаем экземпляр узла SYNC и передаем службу REG0 в узел SYNC
                    // помещаем узел SYNC в список
                    fmc146vSyncList.emplace_back(regService);
                }
            }
        }
        ERROR_ASSERT_X(fmc146vSyncList.empty() != true, "Unit SYNC not found!");
        // выводим число обнаруженных узлов SYNC
        color::printf("\n{+white}Found %d SYNC unit(s){}\n", fmc146vSyncList.size());
        // флаг запроса на прерывание работы
        bool requestCancel {};
        // флаг запроса на обновление параметров
        bool requestUpdate { true };
        // флаг активации синхронизации
        bool requestSyncronize { true };
        // FIXME: >>>
        uint8_t tmp_delayADEL2 {};
        auto tmp_outputADEL { ltc6953_output::out2 };
        // FIXME: <<<
        // читаем конфигурационный файл
        INIReader settingsFile { inifile };
        ERROR_ASSERT_X((settingsFile.ParseError() == 0), "Can't read settings file!");

        // FIXME: >>>
        bool aggregateSyncMode {};
        // FIXME: <<
        // проверяем был ли запрос на обновление параметров
        if (requestUpdate) {
            requestUpdate = false;

            // невозможно прочитать конфигурационный файл
            ERROR_ASSERT_X((settingsFile.Parse() == 0), "Can't read settings file!");
            // разделитель
            color { std::string(44, '-') + "\n"s }.print_black_bright();
            // обновляем параметры для всех плат
            for (auto& fmc146vSync : fmc146vSyncList) {
                // получаем сведения о плате данного узла
                const auto deviceInfo = fmc146vSync.getDeviceInfo();
                // выводим LID и имя устройства
                color::printf("{+cyan}[%d] {+white}%s{}\n", deviceInfo.lid, std::to_string(deviceInfo.name).c_str());
                color { "\t"s + std::string(36, '-') + "\n"s }.print_black_bright();
                color::printf("{+magenta}\tname \thex \tbinary{}\n");
                const auto settingsSection { "FMC146VSYNC" };
                // FIXME: добавить номер платы
                // получаем статусный регистр
                Fmc146v::STATUS_Reg regSTATUS;
                fmc146vSync.getReg(regSTATUS);
                color::printf("\t{+white}STATUS \t{+yellow}0x%4.4X \t%s{}\n", regSTATUS.getValue(),
                    to_binary_string(regSTATUS.getValue()).c_str());
                // обновляем регистр FMODE
                Fmc146v::FMODE_Reg regFMODE {};
                fmc146vSync.getReg(regFMODE);
                regFMODE->Bits.ZYNQ_PWR = settingsFile.GetBoolean(settingsSection, "ZYNQ_PWR", false);
                regFMODE->Bits.OCS_CE = settingsFile.GetBoolean(settingsSection, "OCS_CE", false);
                regFMODE->Bits.LTC6953_CE = settingsFile.GetBoolean(settingsSection, "LTC6953_CE", false);
                regFMODE->Bits.EXTREF = settingsFile.GetBoolean(settingsSection, "EXTREF", false);
                regFMODE->Bits.EXT_EN_N = settingsFile.GetBoolean(settingsSection, "EXT_EN_N", false);
                regFMODE->Bits.GEN_EN = settingsFile.GetBoolean(settingsSection, "GEN_EN", false);
                fmc146vSync.setReg(regFMODE);
                Fmc146v::FMODE_Reg retFMODE {};
                fmc146vSync.getReg(retFMODE);
                // проверяем успешность обновления параметра
                ERROR_ASSERT_X((retFMODE.getValue() == regFMODE.getValue()), "Write FMODE register error!");
                color::printf("\t{+white}FMODE \t{+yellow}0x%2.2X \t%s{}\n", retFMODE.getValue(),
                    to_binary_string<uint8_t>(retFMODE.getValue()).c_str());
                // обновляем регистр DAC
                Fmc146v::DAC_Reg regDAC {};
                fmc146vSync.getReg(regDAC);
                regDAC->Bits.DAC_PD = settingsFile.GetInteger(settingsSection, "DAC_PD", 0);
                regDAC->Bits.DAC_DATA = settingsFile.GetInteger(settingsSection, "DAC_DATA", 0);
                fmc146vSync.setReg(regDAC);
                Fmc146v::DAC_Reg retDAC {};
                fmc146vSync.getReg(retDAC);
                // проверяем успешность обновления параметра
                ERROR_ASSERT_X((retDAC.getValue() == regDAC.getValue()), "Write DAC register error!");
                color::printf("\t{+white}DAC PD \t{+yellow}0x%2.2X \t%s{}\n", retDAC->Bits.DAC_PD,
                    to_binary_string<uint8_t, 2>(retDAC->Bits.DAC_PD).c_str());
                // обновляем регистр MUX
                Fmc146v::MUX_Reg regMUX {};
                fmc146vSync.getReg(regMUX);
                regMUX->Bits.FMC1_MGTCLK_MUX = settingsFile.GetBoolean(settingsSection, "FMC1_MGTCLK_MUX", false);
                regMUX->Bits.FMC2_MGTCLK_MUX = settingsFile.GetBoolean(settingsSection, "FMC2_MGTCLK_MUX", false);
                regMUX->Bits.MUX1_LF_CLK = settingsFile.GetBoolean(settingsSection, "MUX1_LF_CLK", false);
                regMUX->Bits.MUX2_LF_CLK = settingsFile.GetBoolean(settingsSection, "MUX2_LF_CLK", false);
                regMUX->Bits.MUX3_LF_CLK = settingsFile.GetBoolean(settingsSection, "MUX3_LF_CLK", false);
                regMUX->Bits.MUX4_LF_CLK = settingsFile.GetBoolean(settingsSection, "MUX4_LF_CLK", false);
                regMUX->Bits.MUX1_HF_CLK = settingsFile.GetBoolean(settingsSection, "MUX1_HF_CLK", false);
                regMUX->Bits.MUX3_HF_CLK = settingsFile.GetBoolean(settingsSection, "MUX3_HF_CLK", false);
                regMUX->Bits.MUX4_HF_CLK = settingsFile.GetBoolean(settingsSection, "MUX4_HF_CLK", false);
                regMUX->Bits.GAIN1_CLK_EN = settingsFile.GetBoolean(settingsSection, "GAIN1_CLK_EN", false);
                regMUX->Bits.GAIN2_CLK_EN = settingsFile.GetBoolean(settingsSection, "GAIN2_CLK_EN", false);
                regMUX->Bits.GAIN3_CLK_EN = settingsFile.GetBoolean(settingsSection, "GAIN3_CLK_EN", false);
                fmc146vSync.setReg(regMUX);
                Fmc146v::MUX_Reg retMUX {};
                fmc146vSync.getReg(retMUX);
                // проверяем успешность обновления параметра
                ERROR_ASSERT_X((retMUX.getValue() == regMUX.getValue()), "Write MUX register error!");
                color::printf("\t{+white}MUX \t{+yellow}0x%4.4X \t%s{}\n", retMUX.getValue(),
                    to_binary_string(retMUX.getValue()).c_str());
                // обновляем регистр FSRC
                Fmc146v::FSRC_Reg regFSRC {};
                fmc146vSync.getReg(regFSRC);
                regFSRC->Bits._0 = settingsFile.GetBoolean(settingsSection, "FSRC0", false);
                regFSRC->Bits._1 = settingsFile.GetBoolean(settingsSection, "FSRC1", false);
                regFSRC->Bits._2 = settingsFile.GetBoolean(settingsSection, "FSRC2", false);
                regFSRC->Bits._3 = settingsFile.GetBoolean(settingsSection, "FSRC3", false);
                regFSRC->Bits._4 = settingsFile.GetBoolean(settingsSection, "FSRC4", false);
                regFSRC->Bits._5 = settingsFile.GetBoolean(settingsSection, "FSRC5", false);
                regFSRC->Bits._6 = settingsFile.GetBoolean(settingsSection, "FSRC6", false);
                regFSRC->Bits._7 = settingsFile.GetBoolean(settingsSection, "FSRC7", false);
                fmc146vSync.setReg(regFSRC);
                Fmc146v::FSRC_Reg retFSRC {};
                fmc146vSync.getReg(retFSRC);
                // проверяем успешность обновления параметра
                ERROR_ASSERT_X((retFSRC.getValue() == regFSRC.getValue()), "Write FSRC register error!");
                color::printf("\t{+white}FSRC \t{+yellow}0x%2.2X \t%s{}\n", retFSRC.getValue(),
                    to_binary_string<uint8_t>(retFSRC.getValue()).c_str());
                // обновляем регистр FSRC3
                Fmc146v::FSRC3_Reg regFSRC3 {};
                fmc146vSync.getReg(regFSRC3);
                regFSRC3->Bits._0 = settingsFile.GetBoolean(settingsSection, "FSRC3_0", false);
                regFSRC3->Bits._1 = settingsFile.GetBoolean(settingsSection, "FSRC3_1", false);
                regFSRC3->Bits._2 = settingsFile.GetBoolean(settingsSection, "FSRC3_2", false);
                regFSRC3->Bits._3 = settingsFile.GetBoolean(settingsSection, "FSRC3_3", false);
                regFSRC3->Bits._4 = settingsFile.GetBoolean(settingsSection, "FSRC3_4", false);
                regFSRC3->Bits._5 = settingsFile.GetBoolean(settingsSection, "FSRC3_5", false);
                regFSRC3->Bits._6 = settingsFile.GetBoolean(settingsSection, "FSRC3_6", false);
                regFSRC3->Bits._7 = settingsFile.GetBoolean(settingsSection, "FSRC3_7", false);
                fmc146vSync.setReg(regFSRC3);
                Fmc146v::FSRC3_Reg retFSRC3 {};
                fmc146vSync.getReg(retFSRC3);
                // проверяем успешность обновления параметра
                ERROR_ASSERT_X((retFSRC3.getValue() == regFSRC3.getValue()), "Write FSRC3 register error!");
                color::printf("\t{+white}FSRC3 \t{+yellow}0x%2.2X \t%s{}\n", retFSRC3.getValue(),
                    to_binary_string<uint8_t>(retFSRC3.getValue()).c_str());
                color { "\t"s + std::string(36, '-') + "\n"s }.print_black_bright();
                // значение частоты на входе EXT_CLK1
                const double extCLK1_FREQ = settingsFile.GetReal(settingsSection, "EXT_CLK1_FREQ", 10.) * 1.0_MHz;
                if (!regMUX->Bits.MUX2_LF_CLK) {
                    color::printf("\t{+white}EXT CLK1 {+yellow}\t%.3f{+black} MHz{}\n", double(extCLK1_FREQ / 1._MHz));
                }
                // обновляем параметры Si57x
                const double genFREQ = settingsFile.GetReal(settingsSection, "GEN_FREQ", 10.) * 1.0_MHz;
                auto mount_Si57X = settingsFile.GetBoolean(settingsSection, "Si57X", false);
                double freq = genFREQ;
                if (mount_Si57X && regFMODE->Bits.GEN_EN && regMUX->Bits.MUX2_LF_CLK) {
                    fmc146vSync->Si57x.reset();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    const double genFXTAL = settingsFile.GetReal(settingsSection, "GEN_FXTAL", 10.0) * 1.0_MHz;
                    fmc146vSync->Si57x.calib_fxtal(genFXTAL);
                    fmc146vSync->Si57x.freeze_dco(true);
                    fmc146vSync->Si57x.set_freq(genFREQ);
                    fmc146vSync->Si57x.freeze_dco(false);
                    fmc146vSync->Si57x.apply_freq();
                    freq = fmc146vSync->Si57x.get_freq();
                }
                color::printf("\t{+white}GEN FREQ{+yellow} \t%.3f{+black} MHz{}\n", double(freq / 1._MHz));
                // обновляем параметры LMX2594
                if (regFMODE->Bits.OCS_CE) {
                    double refFREQ {};
                    if (!regMUX->Bits.MUX2_LF_CLK) {
                        refFREQ = extCLK1_FREQ;
                    } else {
                        refFREQ = genFREQ;
                    }
                    fmc146vSync->LMX2594.reset();
                    fmc146vSync->LMX2594.update_lock_detect_mux(
                        chappi::lmx2594_lock_detect_mux::lock_detect);
                    // управляем состоянием выхода А
                    auto oscOUTA = settingsFile.GetBoolean(settingsSection, "OSC_OUTA", false);
                    if (oscOUTA) {
                        // устанавливаем выходную мощность канала А
                        auto oscPWRA = settingsFile.GetInteger(settingsSection, "OSC_PWRA",
                            lmx2594_constants::output_power_max);
                        fmc146vSync->LMX2594.update_output_power(
                            lmx2594_output_power { lmx2594_output::outa, int(oscPWRA) });
                        // устанавливаем частоту PLL канала А
                        const double oscFREQA = settingsFile.GetReal(settingsSection, "OSC_FREQA", 100.) * 1.0_MHz;
                        fmc146vSync->LMX2594.update_lock_detect_mux(
                            chappi::lmx2594_lock_detect_mux::readback);
                        // FIXME: >>>
                        try {
                            fmc146vSync->LMX2594.set_frequency(lmx2594_output_frequency {
                                lmx2594_output::outa, refFREQ, double(oscFREQA) });
                        } catch (const std::exception& e) {
                            ERROR_MESSAGE("\n%s\n", e.what());
                            fmc146vSync->LMX2594.update_lock_detect_mux(chappi::lmx2594_lock_detect_mux::lock_detect);
                            color { "\nPress 'Space' to updating parameters and synchronization\n\n" }.print_yellow();
                            do {
                                if (keyboard.isKeyPressed() && keyboard.getKeyCode() == CKeyboard::KeyCode_Space) {
                                    requestUpdate = true;
                                    break;
                                }
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            } while (true);
                            fmc146vSync->LMX2594.update_lock_detect_mux(chappi::lmx2594_lock_detect_mux::lock_detect);
                            if (requestUpdate) {
                                continue;
                            }
                        }
                        // FIXME: <<<
                        fmc146vSync->LMX2594.update_lock_detect_mux(
                            chappi::lmx2594_lock_detect_mux::lock_detect);
                        color::printf("\t{+white}OSC FREQA{+yellow} \t%.3f{+black} MHz{}\n", double(oscFREQA / 1._MHz));
                        color::printf("\t{+white}OSC PWRA{+yellow} \t%d{}\n", oscPWRA);
                        auto oscInteger = fmc146vSync->LMX2594.is_integer_mode();
                        color::printf("\t{+white}OSC MODE{+yellow} \t%s{}\n", (oscInteger) ? "INT" : "FRAC");
                    }
                    fmc146vSync->LMX2594.update_output_enabled(lmx2594_output_enable { lmx2594_output::outa, oscOUTA });
                    // управляем состоянием выхода В
                    auto oscOUTB = settingsFile.GetBoolean(settingsSection, "OSC_OUTB", false);
                    if (oscOUTB) {
                        // устанавливаем выходную мощность канала В
                        auto oscPWRB = settingsFile.GetInteger(settingsSection, "OSC_PWRB", lmx2594_constants::output_power_max);
                        fmc146vSync->LMX2594.update_output_power(lmx2594_output_power { lmx2594_output::outb, int(oscPWRB) });
                        // устанавливаем частоту PLL канала В
                        const double oscFREQB = settingsFile.GetReal(settingsSection, "OSC_FREQB", 100.) * 1.0_MHz;
                        fmc146vSync->LMX2594.update_lock_detect_mux(chappi::lmx2594_lock_detect_mux::readback);
                        // FIXME: >>>
                        for (size_t try_n = 0; try_n < 3; ++try_n) {
                            try {
                                fmc146vSync->LMX2594.set_frequency(lmx2594_output_frequency {
                                    lmx2594_output::outb, refFREQ, double(oscFREQB) });
                                break;
                            } catch (const std::exception& e) {
                                ERROR_MESSAGE("TRY[%d],%s\n", try_n, e.what());
                            }
                        }
                        // FIXME: <<<
                        fmc146vSync->LMX2594.update_lock_detect_mux(chappi::lmx2594_lock_detect_mux::lock_detect);
                        color::printf("\t{+white}OSC FREQB {+yellow} \t%.3f{+black} MHz{}\n", double(oscFREQB / 1._MHz));
                        color::printf("\t{+white}OSC PWRB {+yellow} \t%d{}\n", oscPWRB);
                    }
                    fmc146vSync->LMX2594.update_output_enabled(lmx2594_output_enable { lmx2594_output::outb, oscOUTB });
                }
                if (regFMODE->Bits.LTC6953_CE) {
                    fmc146vSync->LTC6953.reset();
                    fmc146vSync->LTC6953.chip_enable(true);
                    auto vcoIsValid = fmc146vSync->LTC6953.is_vco_valid();
                    ERROR_ASSERT_X((vcoIsValid == true), "VCO error!");
                    if (vcoIsValid) {
                        color::printf("\t{+white}CLK DIST {+green} \tVCO OK{}\n");
                    }
                    auto syncMode = settingsFile.GetInteger(settingsSection, "CLK_DIST_SRQMD"s, 0);
                    // FIXME: >>>
                    if (SyncMode(syncMode) == SyncMode::Agregate) {
                        color::printf("{+green}### Agregate Mode ###{}\n");
                        aggregateSyncMode = true;
                        syncMode = long(ltc6953_srq_mode::sync);
                    }
                    // FIXME: <<<
                    auto pulseCount = settingsFile.GetInteger(settingsSection, "CLK_DIST_SYSCT"s, 0);
                    auto ezSyncMode = settingsFile.GetBoolean(settingsSection, "CLK_DIST_EZMD"s, false);
                    fmc146vSync->LTC6953.set_sync_mode({ ltc6953_srq_mode(syncMode), ltc6953_sysref_pulse_count(pulseCount), ezSyncMode });
                    color::printf("\t{+white}CLK DIST SYSCT \t{+yellow}%d{}\n", pulseCount);
                    color::printf("\t{+white}CLK DIST SRQMD \t{+yellow}%d{}\n", syncMode);
                    color::printf("\t{+white}CLK DIST EZMD \t{+yellow}%d{}\n", ezSyncMode);
                    auto slewRate = settingsFile.GetBoolean(settingsSection, "CLK_DIST_FILTV"s, false);
                    fmc146vSync->LTC6953.set_input_buffer(slewRate);
                    color::printf("\t{+white}CLK DIST FILTV \t{+yellow}%d{}\n", slewRate);
                    color::printf("\t{+white}CLK DIST\tADEL\tDDEL\t DIV\tSRQEN\tOINV\tMODE\tPD{}\n");
                    for (int outputCount {}; outputCount < ltc6953_constants::output_max_num; ++outputCount) {
                        auto settingOption = "CLK_DIST_ADEL"s + std::to_string(outputCount);
                        uint16_t analogDelay = settingsFile.GetInteger(settingsSection, settingOption, 0);
                        fmc146vSync->LTC6953.set_analog_delay({ ltc6953_output(outputCount), analogDelay });
                        // FIXME: >>>
                        if (outputCount == 2) {
                            tmp_delayADEL2 = analogDelay;
                        }
                        // FIXME: <<<
                        settingOption = "CLK_DIST_DDEL"s + std::to_string(outputCount);
                        uint16_t digitalDelay = settingsFile.GetInteger(settingsSection, settingOption, 0);
                        fmc146vSync->LTC6953.set_digital_delay({ ltc6953_output(outputCount), digitalDelay });
                        settingOption = "CLK_DIST_SRQEN"s + std::to_string(outputCount);
                        auto syncEnabled = settingsFile.GetBoolean(settingsSection, settingOption, false);
                        settingOption = "CLK_DIST_DIV"s + std::to_string(outputCount);
                        uint16_t divider = settingsFile.GetInteger(settingsSection, settingOption, 1);
                        fmc146vSync->LTC6953.set_divider({ ltc6953_output(outputCount), divider });
                        settingOption = "CLK_DIST_OINV"s + std::to_string(outputCount);
                        auto inverted = settingsFile.GetBoolean(settingsSection, settingOption, false);
                        fmc146vSync->LTC6953.set_output_inversion({ ltc6953_output(outputCount), inverted });
                        settingOption = "CLK_DIST_MODE"s + std::to_string(outputCount);
                        auto outputSyncMode = settingsFile.GetInteger(settingsSection, settingOption, 0);
                        fmc146vSync->LTC6953.set_output_sync_mode({ ltc6953_output(outputCount),
                            ltc6953_sysref_mode(outputSyncMode), syncEnabled });
                        settingOption = "CLK_DIST_PD"s + std::to_string(outputCount);
                        long outputPowerdownMode = settingsFile.GetInteger(settingsSection, settingOption, 0);
                        fmc146vSync->LTC6953.set_output_powerdown({ ltc6953_output(outputCount),
                            ltc6953_output_powerdown_mode(outputPowerdownMode) });
                        color { "\t\t"s + std::string(60, '-') + "\n"s }.print_black_bright();
                        color::printf("\t\t{+white}OUT%d{}", outputCount);
                        //  FIXME:
                        if (outputPowerdownMode == 0) {
                            color::printf("\t{+yellow}%4.1d{}", analogDelay);
                            color::printf("\t{+yellow}%4.1d{}", digitalDelay);
                            color::printf("\t{+yellow}%4.1d{}", divider);
                            color::printf("\t{+yellow}%4.1d{}", syncEnabled);
                            color::printf("\t{+yellow}%3.1d{}", inverted);
                            color::printf("\t{+yellow}%3.1d{}", outputSyncMode);
                        } else {
                            color::printf("\t{+black}%4.1d{}", analogDelay);
                            color::printf("\t{+black}%4.1d{}", digitalDelay);
                            color::printf("\t{+black}%4.1d{}", divider);
                            color::printf("\t{+black}%4.1d{}", syncEnabled);
                            color::printf("\t{+black}%3.1d{}", inverted);
                            color::printf("\t{+black}%3.1d{}", outputSyncMode);
                        }
                        color::printf("\t{+yellow}%2.1d{}\n", outputPowerdownMode);
                    }
                }
            }
            // параметры успешно обновлены
            color::printf("\t{+green}*** UPDATED ***{}\r");
            std::flush(std::cout);
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            color { "\t"s + std::string(15, ' ') + "\r"s }.print_black_bright();
        }
        if (requestSyncronize) {
            requestSyncronize = false;
            Fmc146v::FMODE_Reg regFMODE {};
            for (auto& fmc146vSync : fmc146vSyncList) {
                // FIXME: >>>
                const auto settingsSection { "FMC146VSYNC" }; // FIXME: добавить номер платы
                auto pulseCount = settingsFile.GetInteger(settingsSection, "CLK_DIST_SYSCT"s, 0);
                auto ezSyncMode = settingsFile.GetBoolean(settingsSection, "CLK_DIST_EZMD"s, false);
                // FIXME: <<<
                fmc146vSync.getReg(regFMODE);
                if (regFMODE->Bits.LTC6953_CE) {
                    // FIXME: >>>
                    if (aggregateSyncMode) {
                        Fmc146v::FSRC_Reg regFSRC {};
                        fmc146vSync.getReg(regFSRC);
                        regFSRC->Bits._0 = 0;
                        regFSRC->Bits._1 = 0;
                        fmc146vSync.setReg(regFSRC);
                        fmc146vSync->LTC6953.set_sync_mode({ ltc6953_srq_mode(SyncMode::Synchronization),
                            ltc6953_sysref_pulse_count(pulseCount), ezSyncMode });
                    }
                    // FIXME: <<<
                    fmc146vSync->LTC6953.sync_request();
                    // FIXME: >>>
                    if (aggregateSyncMode) {
                        Fmc146v::FSRC_Reg regFSRC {};
                        fmc146vSync.getReg(regFSRC);
                        regFSRC->Bits._0 = settingsFile.GetBoolean(settingsSection, "FSRC0", false);
                        regFSRC->Bits._1 = settingsFile.GetBoolean(settingsSection, "FSRC1", false);
                        fmc146vSync.setReg(regFSRC);
                        fmc146vSync->LTC6953.set_sync_mode(
                            { ltc6953_srq_mode(SyncMode::SysRef), ltc6953_sysref_pulse_count(pulseCount), ezSyncMode });
                    }
                    // FIXME: <<<
                }
            }
            color::printf("\t{+green}*** SYNCHRONIZED ***{}\r");
            std::flush(std::cout);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            color { "\t"s + std::string(20, ' ') + "\r"s }.print_black_bright();
        }
        // периодически проверяем температуру и выводим индикатор alive
        static const auto deltaTime = std::chrono::milliseconds(500);
        // текущее время
        const auto currentTime = std::chrono::steady_clock::now();
        // время следующей проверки
        static auto nextTime = currentTime + deltaTime;
        if (currentTime > nextTime) {
            // достигли задонного временного интервала
            nextTime = currentTime + deltaTime;
            for (auto& fmc146vSync : fmc146vSyncList) {
                // получаем статусный регистр
                Fmc146v::STATUS_Reg regSTATUS;
                fmc146vSync.getReg(regSTATUS);
                // проверяем аварийные флаги
                ERROR_ASSERT_X(regSTATUS->Bits.ALARM == false,
                    "Warning! FPGA temp. > 80C");
                ERROR_ASSERT_X(regSTATUS->Bits.OT == false, "Alert! FPGA temp. > 85C");
            }
        }
        // отпускаем поток на заданной время
        // (определяет скорость обработки нажатия)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // } while (!requestCancel);
        // ожидаем завершения работы
        // awaitExit();
    } // try
    catch (const brd_runtime_error& e) {
        ERROR_MESSAGE("\n%s \nBardy error code: 0x%X\n", e.what(), e.get_error());
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return;
    } catch (const std::exception& e) {
        ERROR_MESSAGE("\n%s\n", e.what());
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return;
    } catch (...) {
        ERROR_MESSAGE("\nUnexpected exception!\n");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return;
    }
}
