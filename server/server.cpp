#include "server.hpp"
#include "total.h"
#include <condition_variable>
#include <mutex>
#include <thread>

S32 x_DevNum = -1; // кол-во у-в
BRD_Handle x_hADC = 0; // сервис АЦП
BRD_Handle x_hDAC = 0; // сервис ЦAП
BRD_Handle x_hREG = 0; // сервис REG0
int x_lid = 1;
BRD_Handle x_handleDevice = 0;
int x_mode = BRDopen_SHARED;
std::thread* workerThread;
std::thread* workerThreadA;

int main()
try {
    globalDeviceIndex = 0;

    auto signal_handler = [](int signal) {
        is_cancel = true;
        std::string signal_str {};
        switch (signal) {
        case SIGINT:
            signal_str = "SIGINT";
            break;
        case SIGILL:
            signal_str = "SIGILL";
            break;
        case SIGFPE:
            signal_str = "SIGFPE";
            break;
        case SIGSEGV:
            signal_str = "SIGSEGV";
            break;
        case SIGTERM:
            signal_str = "SIGTERM";
            break;
        case SIGABRT:
            signal_str = "SIGABRT";
            break;
        default:
            signal_str = "UNKNOWN";
            break;
        }
        std::fprintf(stderr, "\n%s\n", signal_str.c_str());
    };
    // регистрируем обработчик сигналов
    std::signal(SIGINT, signal_handler);
    std::signal(SIGILL, signal_handler);
    std::signal(SIGFPE, signal_handler);
    std::signal(SIGSEGV, signal_handler);
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGABRT, signal_handler);

    // инициализируем управляющую библиотеку Bardy
    // auto deviceStore = BRD_DeviceStore(_BRDC("brd.ini"));
    // получаем ссылку на список обнаруженных устройств
    // auto& deviceList = deviceStore.deviceList();

    // изменение IP-адреса
    const auto ifname = get_interface();
    std::fprintf(stdout, "Interface = %s\n", ifname.c_str());
    if (ifname == "") {
        std::fprintf(stderr, "[ERROR] Can't find suitable network interface");
        throw std::runtime_error { "Get interface error" };
    }
    // change_ip_addr(geoPos, ifname);

    // создаем сервер, который будет выполнять команды
    auto serverIP = "0.0.0.0"s; //"192.186.5.75"s;
    auto serverPort = 7778;
    printf("Create ServerExecutor w. IP=%s Port=%d\n", serverIP.c_str(), serverPort);
    ServerExecutor serverExecutor(serverIP, serverPort);
    // добавляем обработчики команд
    // serverExecutor.add_command<WriteSpdExecutor>();
    // serverExecutor.add_command<ReadSpdExecutor>();
    serverExecutor.add_command<GetSysmonExecutor>();
    serverExecutor.add_command<CommandProcessor>();
    serverExecutor.add_command<FastRegsAccess>();
    serverExecutor.add_command<DacControl>();
    serverExecutor.add_command<AdcControl>();
    serverExecutor.add_command<FmcSync>();

    // запускаем поток обработки команд
    serverExecutor.start();
    std::fprintf(stdout, "Server start with - %s\n", ifname.c_str());
    // так как поток обработки команд запускается в режиме detach
    // стоим и ждем пока не поступит сигнал прерывания работы
    int i = 0;
    while (!is_cancel) {
        std::this_thread::sleep_for(100ms);
        // проверяем была ли ошибка в процессе работы сервера
        auto serverError = serverExecutor.error();
        if (serverError) {
            // прокидываем исключение, которое возникло в потоке сервера
            std::rethrow_exception(serverError);
        }
        i++;
    }
    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    std::fprintf(stderr, "\n%s\n", e.what());
    return EXIT_FAILURE;
} catch (...) {
    std::fprintf(stderr, "\nUnexpected exception!\n");
    return EXIT_FAILURE;
}

////////////////////////////////////////////////////////////////// CommandProcessor
json CommandProcessor::execute(const json& request)
{

    json response = request;
    std::string message = "Parameter is missing: ";
    if (!request.contains("com")) {
        response["error"] = message + "com'";
        return response;
    }

    std::string line = request["com"];
    std::string param;
    if (request.contains("param"))
        param = request["param"];

    printf("Receive command com> %s: %s  \n", line.c_str(), param.c_str());
    if (!isCommand(request, response, line, param, paramReg)) {
        response["error"] = "Unrecognized error!!";
    }

    return response;
}

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void sleep_for_milliseconds(int milliseconds)
{
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000); // Преобразуем миллисекунды в микросекунды
#endif
}

#include <cstdlib>

void launch_application(const std::string& command)
{
    std::system(command.c_str());
}

bool CommandProcessor::isCommand(const json& request, json& response, std::string& cmd, std::string& param, commandLineParams& params)
{
    if (cmd == "init") {
        std::string fileini = "brd.ini";
        if (request.contains("path")) {
            fileini = request["path"];
        }
        // инициализировать библиотеку
        S32 status = BRD_init(fileini.c_str(), &x_DevNum);
        if (!BRD_errcmp(status, BRDerr_OK)) {
            BRDC_printf(_BRDC("ERROR: BARDY Initialization = 0x%X \n"), status);
            exit(0);
        }
        BRD_displayMode(BRDdm_VISIBLE | BRDdm_CONSOLE); // режим вывода информационных сообщений : отображать все уровни на консоле

    } else if (cmd == "lid") {
        printLids();
        if (param.empty()) {
            response["error"] = "Bad param in key-groupe com:lid";
            return false;
        }
        if (request.contains("mode")) {
            std::string sm = request["mode"];
            x_mode = atoi(sm.c_str());
        } else
            x_mode = BRDopen_EXCLUSIVE;
        BRD_close(x_handleDevice);
        x_lid = atoi(param.c_str());
        x_handleDevice = BRD_open(x_lid, x_mode, &x_mode);
        if (x_handleDevice <= 0)
            BRDC_printf("<*> ERROR open device  %d in mode %s\n", x_lid, getStrOpenModeDevice(x_mode).c_str());
        else
            BRDC_printf("<*> Current LId = %d in mode %s \n", x_lid, getStrOpenModeDevice(x_mode).c_str());
    } else if (cmd == "serv") {
        //
    } else if (cmd == "info") {
        printInfo(params, x_handleDevice);
    } else if (cmd == "release" || cmd == "cleanup") {
        BRD_cleanup();
    } else if (cmd == "stop-server") { // команда остановки сервера
        //
        BRD_cleanup();
        // std::abort();
        exit(1);
    } else if (cmd == "version") { //
        std::string ver = std::to_string(version_srv_hi) + "." + std::to_string(version_srv_lo);
        printf("Server version %s \n", ver.c_str());
        response["version"] = ver;
    } else if (cmd == "system") { //
        if (request.contains("launch")) {
            std::string e = request["launch"];
            std::thread t(launch_application, e);
            t.detach();
            printf("Launch execute: %s\n", e.c_str());
            return true;
        }
        response["error"] = "Error of launch execute";
        return false;
    } else if (cmd == "pause" || cmd == "pause-ms") { //
        int time = 1000;
        if (request.contains("time")) {
            std::string ts = request["time"];
            time = atoi(ts.c_str());
        }
        printf("<Command> pause  %d ms \n", time);
        sleep_for_milliseconds(time);
    } else {
        response["error"] = "Unrecognized command!!";
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////// FastRegsAccess
json FastRegsAccess::execute(const json& request)
{
    // paramReg.hService = getService(indexDev, 0, _BRDC("REG"))->getHandle();

    json response = request;
    std::string message = "Parameter is missing: ";
    if (!request.contains("reg")) {
        response["error"] = message + "reg'";
        return response;
    }

    std::string line = request["reg"];
    printf("Receive command reg > %s:  \n", line.c_str());

    if (parsingLine(line, paramReg)) {
        if (x_hREG <= 0) {
            S32 status;
            U32 mode = BRDcapt_SHARED;
            x_hREG = BRD_capture(x_handleDevice, 0, &mode, _BRDC("REG0"), 5000);
        }
        if (x_hREG <= 0) {
            BRDC_printf(_BRDC("REG0 NOT capture (%X)\n"), x_hREG);
            response["error"] = "REG0 NOT capture !!";
            return response;
        }
        paramReg.hService = x_hREG;
        U32 stst = processReg(paramReg);
        printf("%s: %s tetrada=%d register=0x%X value=0x%X\n",
            (stst > 0) ? "Ok" : "Err", paramReg.write ? "Write" : "Read", paramReg.tetrad,
            paramReg.reg, paramReg.value);
        if (!paramReg.write)
            response["value"] = std::to_string(paramReg.value);
        if (stst <= 0)
            response["error"] = "error execution ..";
    } else {
        printf(" .. the input >%s< is not recognized ..\n", line.c_str());
    }

    return response;
}

bool FastRegsAccess::parsingLine(std::string& line, commandLineParams& params)
{
    if (line.empty())
        return false;
    bool write = false;
    std::string s, tetr, reg, val;
    uint32_t t, r, v;
    int n = 0;
    s = line;
    n = s.find(':');
    if (n < 0)
        return false;
    tetr = s.substr(0, n);
    s = s.substr(n + 1);
    n = s.find('=');
    reg = s.substr(0, n);
    if (n > 0) {

        write = true;
        val = s.substr(n + 1);
    }
    // cout << "REco line =" << line.c_str() << "  tetr=" << tetr.c_str() << "  reg=" << reg.c_str() << "   val=" << val.c_str() << endl;
    params.write = write;
    params.tetrad = strtoull(tetr.c_str(), NULL, 0);
    params.reg = strtoull(reg.c_str(), NULL, 0);
    params.value = strtoull(val.c_str(), NULL, 0);
    // cout << "Convert  tetr=0x" << hex << params.tetrad << "  reg=" << params.reg << "   val=" << params.value << endl;
    return true;
}

////////////////////////////////////////////////////////////////// DacControl

std::mutex mtx;
std::mutex mtxa;
std::condition_variable cv;
std::condition_variable cva;
bool isWorkerRunning = true; // Флаг состояния рабочего потока
bool isWorkerRunningA = true; // Флаг состояния рабочего потока ADC
std::string filenameDac = "fileoutDac.txt";

#include <fstream>
#include <iostream>
#include <sstream>

// Глобальная переменная для хранения старого буфера cout
std::streambuf* old_cout_buffer = nullptr;

void redirect_stdout(std::ostringstream& oss)
{
    old_cout_buffer = std::cout.rdbuf(oss.rdbuf());
}

void restore_stdout()
{
    if (old_cout_buffer != nullptr) {
        std::cout.rdbuf(old_cout_buffer);
    }
}

void dacWork()
{
    printf("<SRV> Thread for DAC created ..\n");
    // std::ostringstream s;
    // redirect_stdout(s);
    //
    // printf("<DEBUG> Debug string for check working redirect\n");
    //
    isWorkerRunning = true;
    WorkMode(isWorkerRunning);
    //
    // std::ofstream file(filenameDac);
    // if (file.is_open()) {
    //    file << s.str();
    //    file.close();
    //} else
    //    printf("File %s not save!\n", filenameDac.c_str());
    // restore_stdout();

    // Сигнализируем основному потоку о завершении работы
    {
        std::lock_guard<std::mutex> lock(mtx);
        isWorkerRunning = false;
    }
    cv.notify_one(); // Пробуждаем основной поток
    printf("<SRV> Thread for DAC stoped ..\n");
}

json DacControl::execute(const json& request)
{

    json response = request;
    std::string message = "Parameter is missing: ";
    if (!request.contains("dac")) {
        response["error"] = message + "dac";
        return response;
    }

    std::string scmd = request["dac"];
    std::string sfile;
    if (request.contains("path"))
        sfile = request["path"];
    // printf("Receive command dac-comc> %s: %s  \n", scmd.c_str(), sfile.c_str());

    if (scmd == "config") {
        if (sfile.empty()) {
            response["error"] = "Error: filename is empty ..";
            return response;
        }
        ReadIniFileOption(sfile);

    } else if (scmd == "capture") {
        int mode = 1; // BRDopen_SHARED
        int capt = 1; // BRDcapt_SHARED
        if (request.contains("device")) {
            std::string sm = request["device"];
            if (sm == "excl")
                mode = BRDopen_EXCLUSIVE;
            else
                mode = atoi(sm.c_str());
        }
        if (request.contains("service")) {
            std::string sm = request["service"];
            if (sm == "excl")
                capt = BRDcapt_EXCLUSIVE;
            else
                capt = atoi(sm.c_str());
        }
        CaptureAllDac(mode, capt);

    } else if (scmd == "get-dac-num") {
        // вернуть g_nDacNum
        response["value"] = g_nDacNum;

    } else if (scmd == "spi-reg") {
        if (request.contains("expr")) { // от expression
            std::string es = request["expr"];
            if (!parsSpiCommand(es)) {
                response["error"] = "spi-reg : parser error!!";
                return response;
            }
            calcSpi(response);
        }

    } else if (scmd == "nco-channel" || scmd == "nco-main") {
        std::string sch, frq, phs;
        int ch = 0;
        double fr = 0.0, ph = 0.0;
        if (request.contains("ch")) {
            sch = request["ch"];
            ch = atoi(sch.c_str());
        }
        if (request.contains("frq")) {
            frq = request["frq"];
            fr = atof(frq.c_str());
        }
        if (request.contains("phase")) {
            phs = request["phase"];
            ph = atof(phs.c_str());
        }

        if (scmd == "nco-channel")
            nco_channel_setup(ch, fr, ph);
        else
            nco_main_setup(ch, fr, ph);

    } else if (scmd == "set-dac") {
        ReadIniFileDevice();
        SetAllDac();
        SetMasterSlave();

    } else if (scmd == "work") {
        if (request.contains("mode")) {
            std::string ms = request["mode"];
            g_nWorkMode = atoi(ms.c_str());
        }
        if (request.contains("file")) {
            filenameDac = request["file"];
        }
        workerThread = new std::thread(dacWork);
        // WorkMode();
    } else if (scmd == "release" || scmd == "stop") {
        // Остановка рабочего потока
        std::lock_guard<std::mutex> lock(mtx);
        // printf("<**> DAC mutex unlock ..\n");
        isWorkerRunning = false;
        // printf("<**> DAC isWorkerRunning = false ..\n");
        cv.notify_one(); // Пробуждаем рабочий поток, если он спит
        // printf("<**> DAC notify_one ..\n");
        //  Ожидание завершения рабочего потока
        std::unique_lock<std::mutex> lk(mtx);
        // printf("<**> DAC thread wakeup ..\n");
        cv.wait(lk, [] { return !isWorkerRunning; });
        // printf("<**> DAC thread touch ..\n");
        //  Завершение рабочего потока
        if (workerThread->joinable()) {
            workerThread->join();
        }
        // Освобождение ЦАПов
        // printf("<**> DAC start function ReleaseAllDac() ..\n");
        ReleaseAllDac();
        printf("<SRV> DAC released ..\n");

    } else if (scmd == "list-parameters") {
        ListParameters();
    } else {
        response["error"] = "Unrecognized error!!";
    };

    return response;
}

void DacControl::calcSpi(json& resp)
{
    S32 status;
    U32 mode = BRDcapt_SHARED;
    BRD_Handle hSrv = BRD_capture(x_handleDevice, 0, &mode, _BRDC("REG0"), 5000);
    if (hSrv <= 0) {
        BRDC_printf(_BRDC("REG0 NOT capture (%X)\n"), hSrv);
        resp["error"] = "REG0 NOT capture !!";
        return;
    }
    if (!write_) {
        U32 val = SpdRead(hSrv, findTetrad(), 0, 0, regNum_);
    } else {
        S32 err = SpdWrite(hSrv, findTetrad(), 0, 0, regNum_, value_);
    }
    // response["result"] =  (write_ ? ("SPI-write: "):("SPI-read: ")) + ("reg=");
    printf("SPI %s  reg=0x%4.4X  val=0x%4.4X\n", write_ ? "write" : "read", regNum_, value_);
    resp["value"] = (value_);
    BRD_release(hSrv, 0);
}

bool DacControl::parsSpiCommand(const std::string es)
{
    if (es.empty())
        return false;
    write_ = false;
    std::string s, sreg, sval;
    int n = 0;
    s = es;
    n = s.find('=');
    sreg = s.substr(0, n);
    if (n > 0) {
        write_ = true;
        sval = s.substr(n + 1);
    }
    regNum_ = strtoull(sreg.c_str(), NULL, 0);
    value_ = strtoull(sval.c_str(), NULL, 0);
    return true;
}

////////////////////////////////////////////////////////////////// AdcControl

void AdcWork()
{
    printf("<SRV> Thread for ADC created ..\n");

    isWorkerRunningA = true;
    workFlow(/*isWorkerRunningA*/);

    // Сигнализируем основному потоку о завершении работы
    {
        std::lock_guard<std::mutex> lock(mtxa);
        isWorkerRunningA = false;
    }
    cva.notify_one(); // Пробуждаем основной поток
    printf("<SRV> Thread for ADC stoped ..\n");
}

json AdcControl::execute(const json& request)
{
    // printf("##### MARKER: ADC:execute begin\n");
    json response = request;
    std::string message = "Parameter is missing: ";
    if (!request.contains("adc")) {
        response["error"] = message + "adc";
        return response;
    }

    std::string scmd = request["adc"];
    std::string sfile;
    if (request.contains("path")) {
        sfile = request["path"];
    }
    // printf("Receive command adc> %s: %s  \n", scmd.c_str(), sfile.c_str());
    // printf("##### MARKER: adc:%s, path=%s\n", scmd.c_str(), sfile.c_str());

    if (scmd == "config") {
        if (sfile.empty()) {
            response["error"] = "error: ini-file dn't found ..";
            return response;
        }
        adcGetOptions(sfile);

    } else if (scmd == "setupADC" || scmd == "power") {
        int mode = BRDopen_EXCLUSIVE;
        if (request.contains("mode")) {
            std::string sm = request["mode"];
            mode = atoi(sm.c_str());
        }
        bool powerOk = false;
        if (x_handleDevice > 0)
            powerOk = checkPower(x_handleDevice, mode);
        else
            response["error"] = "the device is not open!";
        if (!powerOk)
            printf("<ERR> ADC power error .. /n");

    } else if (scmd == "startWorkFlow" || scmd == "work") {
        if (x_handleDevice > 0)
            workerThreadA = new std::thread(AdcWork);
        // workFlow();
        else
            response["error"] = "the device is not open!";

    } else if (scmd == "spi-reg") {
        if (request.contains("expr")) { // от expression
            std::string es = request["expr"];
            if (!parsSpiCommand(es)) {
                response["error"] = "spi-reg : parser error!!";
                return response;
            }
            calcSpi(response);
        }

    } else if (scmd == "release") {
        //        ReleaseAllDac();
        releaseAdc();

    } else if (scmd == "list-parameters") {
        ListParametersAdc();

    } else {
        response["error"] = "Unrecognized error!!";
    };

    return response;
}

bool AdcControl::parsSpiCommand(const std::string es)
{
    if (es.empty())
        return false;
    write_ = false;
    std::string s, sreg, sval;
    int n = 0;
    s = es;
    n = s.find('=');
    sreg = s.substr(0, n);
    if (n > 0) {
        write_ = true;
        sval = s.substr(n + 1);
    }
    regNum_ = strtoull(sreg.c_str(), NULL, 0);
    value_ = strtoull(sval.c_str(), NULL, 0);
    return true;
}

void AdcControl::calcSpi(json& resp)
{
    S32 status;
    U32 mode = BRDcapt_SHARED;
    BRD_Handle hSrv = BRD_capture(x_handleDevice, 0, &mode, _BRDC("REG0"), 5000);
    if (hSrv <= 0) {
        BRDC_printf(_BRDC("REG0 NOT capture (%X)\n"), hSrv);
        resp["error"] = "REG0 NOT capture !!";
        return;
    }
    if (!write_) {
        U32 val = SpdRead(hSrv, findTetrad(), 0, 0, regNum_);
    } else {
        S32 err = SpdWrite(hSrv, findTetrad(), 0, 0, regNum_, value_);
    }
    // response["result"] =  (write_ ? ("SPI-write: "):("SPI-read: ")) + ("reg=");
    printf("SPI %s  reg=0x%4.4X  val=0x%4.4X\n", write_ ? "write" : "read", regNum_, value_);
    BRD_release(hSrv, 0);
}

void DacControl::nco_main_setup(std::size_t chan, double freq, double phase)
{
    double freq_clk = 12000.0; // частота тактирования ЦАП в МГц
    auto DDSM_FTW = uint64_t(round((freq / (freq_clk * 1'000'000)) * std::pow(2, 48)));
    auto DDSM_PHASE = uint16_t(round((phase / 180.) * std::pow(2, 15)));

    printf("[ADC] Main NCO's Setup: DAC%zd = %.0f Hz [%012zXh], Phase = %.0f [%04Xh]", chan, freq, DDSM_FTW, phase, DDSM_PHASE);

    auto page = uint8_t {};
    switch (chan) {
    case 0:
        page = 0x40;
        break; // DAC0
    case 1:
        page = 0x80;
        break; // DAC1
    }

    // Main NCO's && PA Protect
    std::vector<std::pair<std::size_t, uint8_t>> sequence = {
        { 0x008, page }, // Select Main NCO Page
        { 0x112, 0x09 },
        // {0x1E6, 0x00},
        { 0x113, 0x00 }, // Disable Update NCO
        { 0x114, DDSM_FTW >> 0 & 0xFF }, { 0x115, DDSM_FTW >> 8 & 0xFF }, { 0x116, DDSM_FTW >> 16 & 0xFF },
        { 0x117, DDSM_FTW >> 24 & 0xFF }, { 0x118, DDSM_FTW >> 32 & 0xFF }, { 0x119, DDSM_FTW >> 40 & 0xFF },
        { 0x11C, DDSM_PHASE >> 0 & 0xFF }, { 0x11D, DDSM_PHASE >> 8 & 0xFF },
        { 0x113, 0x01 }, // Enable Update NCO
    };
    U32 mode = BRDcapt_SHARED;
    BRD_Handle hSrv = BRD_capture(x_handleDevice, 0, &mode, _BRDC("REG0"), 5000);
    if (hSrv <= 0) {
        BRDC_printf(_BRDC("REG0 NOT capture (%X)\n"), hSrv);
        return;
    }
    printf("* NCO (MAIN) :\n");
    for (auto& addr_val : sequence) {
        // spi_write(addr_val.first, addr_val.second);
        SpdWrite(hSrv, findTetrad(), 0, 0, addr_val.first, addr_val.second);
        printf("  SPI %s  reg=0x%4.4X  val=0x%4.4X\n", "write", addr_val.first, addr_val.second);
    }
    BRD_release(hSrv, 0);
    /**/
}

/// \brief      Установка NCO's на выходах каналов DAC's
///
/// \param      chan   Номер канала Main
/// \param      freq   Частота в Гц
/// \param      phase  Начальная фаза в градусах
///
/// \return     void
///
void DacControl::nco_channel_setup(std::size_t chan, double freq, double phase)
{
    double freq_clk = 12000.0; // частота тактирования ЦАП в МГц
    U32 mode = BRDcapt_SHARED;
    BRD_Handle hSrv = BRD_capture(x_handleDevice, 0, &mode, _BRDC("REG0"), 5000);
    if (hSrv <= 0) {
        BRDC_printf(_BRDC("REG0 NOT capture (%X)\n"), hSrv);
        return;
    }
    uint8_t reg = SpdRead(hSrv, findTetrad(), 0, 0, 0x111);
    auto dp_inter_mode = (reg >> 4) & 0x0F;
    auto DDSC_FTW = uint64_t(round((freq / ((freq_clk * 1'000'000) / dp_inter_mode)) * std::pow(2, 48)));
    auto DDSC_PHASE = uint16_t(round((phase / 180.) * std::pow(2, 15)));

    printf("[ADC] Channel NCO's Setup: CH%zd = %.0f Hz [%012zXh], Phase = %.0f [%04Xh]", chan, freq, DDSC_FTW, phase, DDSC_PHASE);

    auto page = uint8_t {};
    switch (chan) {
    case 0:
        page = 0b000001;
        break; // Ch0
    case 1:
        page = 0b000010;
        break; // Ch1
    case 2:
        page = 0b000100;
        break; // Ch2
    case 3:
        page = 0b001000;
        break; // Ch3
    case 4:
        page = 0b010000;
        break; // Ch4
    case 5:
        page = 0b100000;
        break; // Ch5
    }

    // Channel NCO's
    std::vector<std::pair<std::size_t, uint8_t>> sequence = {
        { 0x008, page }, // Select Main NCO Page
        { 0x130, 0x40 },
        { 0x131, 0x00 }, // Disable Update NCO
        { 0x132, DDSC_FTW >> 0 & 0xFF }, { 0x133, DDSC_FTW >> 8 & 0xFF }, { 0x134, DDSC_FTW >> 16 & 0xFF },
        { 0x135, DDSC_FTW >> 24 & 0xFF }, { 0x136, DDSC_FTW >> 32 & 0xFF }, { 0x137, DDSC_FTW >> 40 & 0xFF },
        { 0x138, DDSC_PHASE >> 0 & 0xFF }, { 0x139, DDSC_PHASE >> 8 & 0xFF },
        { 0x131, 0x01 }, // Enable Update NCO
    };

    printf("* NCO (CHANNELs) :\n");
    for (auto& addr_val : sequence) {
        // spi_write(addr_val.first, addr_val.second);
        SpdWrite(hSrv, findTetrad(), 0, 0, addr_val.first, addr_val.second);
        printf("  SPI %s  reg=0x%4.4X  val=0x%4.4X\n", "write", addr_val.first, addr_val.second);
    }
    BRD_release(hSrv, 0);
    /**/
}

////////////////////////////////////////////////////////////////// FmcSync
json FmcSync::execute(const json& request)
{
    json response = request;
    std::string message = "Parameter is missing: ";
    if (!request.contains("sync")) {
        response["error"] = message + "sync";
        return response;
    }

    std::string scmd = request["sync"];
    std::string sfile;
    if (request.contains("path"))
        sfile = request["path"];
    printf("Receive command sync> %s: %s  \n", scmd.c_str(), sfile.c_str());

    if (scmd == "config") {
        if (sfile.empty()) {
            response["error"] = "Error: filename is empty ..";
            return response;
        }
        startFmc(sfile, x_DevNum);
        response["ok"] = "startFMC function ..";
    }

    return response;
}