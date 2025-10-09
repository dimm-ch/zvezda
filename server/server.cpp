// clang-format off

#include "server.hpp"
#include "total.h"

S32 x_DevNum = -1; // кол-во у-в

FuDevs DevicesLid[MAX_LID_DEVICES];
int x_lid = 1;
BRD_Handle x_handleDevice = 0;


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

bool setPowerOnIfOff(int lid)
{
    bool retVal = false;

    BRD_Handle handleDevice = DevicesLid[lid].device.handle();
    if (handleDevice > 0) {
        // получаем состояние FMC-питания (если не FMC-модуль, то ошибка)
        BRDextn_FMCPOWER power;
        power.slot = 0;
        S32 status = BRD_extension(handleDevice, 0, BRDextn_GET_FMCPOWER, &power);
        if (BRD_errcmp(status, BRDerr_OK)) {
            if (power.onOff) {
                BRDC_printf(_BRDC("FMC Power: ON %.2f Volt\n"), power.value / 100.);
            } else {
                BRDC_printf(_BRDC("FMC Power is turned off, enabling now...\n"));
                power.onOff = 1;
                status = BRD_extension(handleDevice, 0, BRDextn_SET_FMCPOWER, &power);
                status = BRD_extension(handleDevice, 0, BRDextn_GET_FMCPOWER, &power);
                if (power.onOff)
                    BRDC_printf(_BRDC("FMC Power: ON %.2f Volt\n"), power.value / 100.);
                else
                    BRDC_printf(_BRDC("FMC Power: OFF %.2f Volt\n"), power.value / 100.);
            }
        }
        return true;
    }
    if(! DevicesLid[lid].device.isOpen()) {
        printf("<ERR> setPowerOnIfOff: %s ", DevicesLid[lid].device.error().c_str() );
    }
    return false;
}



bool CommandProcessor::isCommand(const json& request, json& response, std::string& cmd, std::string& param, commandLineParams& params)
{
    U32 x_mode = 0;

    std::string slid;
    x_lid = 1;
    if (request.contains("lid")) {
        slid = request["lid"];
    }    
    if (!slid.empty()) {
        x_lid = atoi(slid.c_str());
        if (x_lid <= 0 || x_lid >= MAX_LID_DEVICES) {
            BRDC_printf("<ERR> Bad LID number!\n");
            response["error"] = "Bad LID!";
            return response;
        }
    }

    if (cmd == "init") {
        std::string fileini = "brd.ini";
        if (request.contains("path")) {
            fileini = request["path"];
        }
        // инициализировать библиотеку
        S32 status = BRD_init(fileini.c_str(), &x_DevNum);
        if (!BRD_errcmp(status, BRDerr_OK)) {
            BRDC_printf(_BRDC("<ERR> BARDY Initialization = 0x%X \n"), status);
            return false;
        }
        else
            printf("<SRV> Library BARDY - is initialize, device found : %d \n", x_DevNum);
        BRD_displayMode(BRDdm_VISIBLE | BRDdm_CONSOLE); // режим вывода информационных сообщений : отображать все уровни на консоле

    }    
    if (cmd == "initex") {
        std::string fileini = "brd.ini";   
        std::string filelog;     
        bool modeAuto = false;
        if (request.contains("path")) {
            fileini = request["path"];
        }
        if (request.contains("log")) {
            filelog = request["log"];
        }
        if (request.contains("auto")) {
            modeAuto = true;
        }        
        // инициализировать библиотеку
        BRD_displayMode(BRDdm_VISIBLE | BRDdm_CONSOLE); 
        U32 modei = BRDinit_FILE | (modeAuto ? BRDinit_AUTOINIT : 0);
        S32 status = BRD_initEx(modei, fileini.c_str(), filelog.c_str(), &x_DevNum);
        if (!BRD_errcmp(status, BRDerr_OK)) {
            BRDC_printf(_BRDC("<ERR> BARDY Ex initialization = 0x%X \n"), status);
            return false;
        }
        else
            printf("<SRV> Library BARDY - is initialize, device found : %d \n", x_DevNum);
    }

    else if (cmd == "open") {
        printLids();
        if (request.contains("mode")) {
            std::string sm = request["mode"];
            x_mode = atoi(sm.c_str());
        } else
            x_mode = BRDopen_SHARED;
        
        DevicesLid[x_lid].device.setMode(x_mode);
        x_handleDevice = DevicesLid[x_lid].device.open(x_lid); 
        if (x_handleDevice <= 0) {
            BRDC_printf("<ERR> Error open device %d in mode %s\n", x_lid, getStrOpenModeDevice(x_mode).c_str());
            response["error"] = DevicesLid[x_lid].device.error();            
            return false;
        } else 
            BRDC_printf("<SRV> Current LID = %d open in mode - %s \n", x_lid, DevicesLid[x_lid].device.strMode().c_str());
        setPowerOnIfOff(x_lid);
        
    } else if (cmd == "chdir") {
        std::string dir;
        if (request.contains("path")) 
            dir = request["path"];
        if(!dir.empty()) {
#ifdef __IPC_LINUX__            
            chdir(dir.c_str());
            char buffer[PATH_MAX];
            if(getcwd(buffer, sizeof(buffer)))
                printf("<SRV >Current dir : %s\n", buffer);
#endif            
        }
        //
    } else if (cmd == "close") {
        //
        DevicesLid[x_lid].device.close();
    
    } else if (cmd == "info") {
        BRD_Version ver;
        BRD_version(x_handleDevice, &ver);
        BRDC_printf("--------------------- INFO ------------------------- \n");
        BRDC_printf("-- Version: Shell v.%d.%d, Driver v.%d.%d --\n",
            ver.brdMajor, ver.brdMinor, ver.drvMajor, ver.drvMinor);
        for (size_t i = 0; i < MAX_LID_DEVICES; i++) {
            if (DevicesLid[i].device.isOpen()) {
                U32 v = 0;
                BRDC_printf("* LID=%d open in %s mode (handle=%X), ver.%d \n", i,
                    getStrOpenModeDevice(DevicesLid[i].device.mode()).c_str(), DevicesLid[i].device.handle(), v);
                
            }
        }
        BRDC_printf("---------------------------------------------------- \n");
    } else if (cmd == "release" || cmd == "cleanup") {
        for (auto& dev : DevicesLid) {
            if (dev.device.isOpen())
                dev.device.close();
        }
        clearAll();

    } else if (cmd == "stop-server") { // команда остановки сервера

        // std::abort();
        clearAll();
        ExcCtrl ec;
        ec.ctrl = END;
        ec.desc = "stop-server";
        throw ec;

    } else if (cmd == "version") { //
        std::string ver = std::to_string(version_srv_hi) + "." + std::to_string(version_srv_lo);
        printf("<SRV> Server version %s \n", ver.c_str());
        response["version"] = ver;

    } else if (cmd == "system") { //
        if (request.contains("launch")) {
            std::string e = request["launch"];
            std::thread t(launch_application, e);
            t.detach();
            printf("<SRV> Launch execute: %s\n", e.c_str());
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
        printf("<SRV> Pause  %d ms \n", time);
        sleep_for_milliseconds(time);
    } 
    else if (cmd == "info") {
        BRD_Info info = {sizeof(BRD_Info)};
        S32 err = BRD_getInfo(x_lid, &info);
        if(err < 0 ){
            response["error"] = "No information is available for this lid!";    
            return false;
        }
        printf("***** Device LID = %d *****\n", x_lid);
        printf("- name        : %s\n", info.name);
        printf("- pid         : %d\n", info.pid);
        printf("- type board  : %d\n", info.boardType);
        printf("- version     : %d.%d\n", info.verMajor, info.verMinor);
        printf("\n");
    }
    else {
        response["error"] = "Unrecognized command!!";
        return false;
    }
    return true;
}

void CommandProcessor::clearAll()
{
    BRD_cleanup();
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

    if (!parsingLine(line, paramReg)) {
        printf("<ERR> .. the input >%s< is not recognized ..\n", line.c_str());
        response["error"] ="The input is not recognized (reg = ?)";
        return response;
    }

    // Определяем LID устройства
    std::string slid;
    if (request.contains("lid"))
        slid = request["lid"];
    x_lid = 1;
    if (!slid.empty()) {
        x_lid = atoi(slid.c_str());
        if (x_lid <= 0 || x_lid >= MAX_LID_DEVICES) {
            BRDC_printf("<ERR> Bad LID number!\n");
            response["error"] = "Bad LID!";
            return response;
        }
    }

    if(!DevicesLid[x_lid].device.isOpen())
        DevicesLid[x_lid].device.open(x_lid);
    if(!DevicesLid[x_lid].servRegs.isCaptured()) {
        U32 mode = BRDcapt_EXCLUSIVE;
        DevicesLid[x_lid].servRegs.setMode(mode);
        x_handleDevice = DevicesLid[x_lid].device.handle();
        DevicesLid[x_lid].servRegs.capture(x_handleDevice, "REG0", 10000); //
    }
    if(!DevicesLid[x_lid].servRegs.isCaptured()){
        BRDC_printf(_BRDC("REG0 NOT capture (%X)\n"), DevicesLid[x_lid].servRegs.handle());
        response["error"] = "REG0 NOT capture !!";



        return response;
    }
    paramReg.hService = DevicesLid[x_lid].servRegs.handle();
    U32 stst = processReg(paramReg);
    printf("<SRV> %s: %s tetrada=%d register=0x%X value=0x%X\n",
            (stst > 0) ? "Ok" : "Err", paramReg.write ? "Write" : "Read", paramReg.tetrad,
            paramReg.reg, paramReg.value);
    if (!paramReg.write)
            response["value"] = std::to_string(paramReg.value);
    if (stst <= 0)
            response["error"] = "error execution ..";
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

void dacWork(int lid)
{
    DevicesLid[lid].dacCtrlThr.isStoped.store(false);
    DevicesLid[lid].dacCtrlThr.stop.store(false);    
    printf("<SRV> Thread for DAC-%d started ..\n", lid);
    WorkMode(lid);
    DevicesLid[lid].dacCtrlThr.isStoped.store(true);
    printf("<SRV> Thread for DAC-%d stoped ..\n", lid);
}

json DacControl::execute(const json& request)
{
    int x_mode = BRDopen_SHARED;
    json response = request;
    error_ = "";
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

    // Определяем LID устройства
    std::string slid;
    if (request.contains("lid")) {
        slid = request["lid"];
    }
    x_lid = 1;
    if (!slid.empty()) {
        x_lid = atoi(slid.c_str());
        if (x_lid <= 0 || x_lid >= MAX_LID_DEVICES) {
            BRDC_printf("<ERR> Bad LID number!\n");
            response["error"] = "Bad LID!";
            return false;
        }
    }
    if (!DevicesLid[x_lid].device.isOpen()) {
        BRDC_printf("<ERR> Device d'nt opened!\n");
        response["error"] = "Device d'nt opened!";
        if(scmd != "list-parameters")
            return false;
    } 
    x_handleDevice = DevicesLid[x_lid].device.handle();

    if (scmd == "config") {
        if (sfile.empty()) {
            response["error"] = "Filename is empty ..";
            return response;
        }
        ReadIniFileOption(sfile, x_lid);

    } else if (scmd == "capture") {        
        U32 capt = BRDcapt_EXCLUSIVE;
        if (request.contains("service")) {
            std::string sm = request["service"];
            if (sm == "excl")
                capt = BRDcapt_EXCLUSIVE;
            else
                capt = atoi(sm.c_str());
        }        
        CaptureAllDac(x_lid, capt);

    } else if (scmd == "get-dac-num") {
        // вернуть g_nDacNum
        int ndc = DevicesLid[x_lid].dac.size();
        response["value"] = ndc;
        BRDC_printf("<SRV> Number of DACs = %d \n", ndc);
        //
    } else if (scmd == "spi-reg") {
        //
        if (request.contains("expr")) { // от expression
            std::string es = request["expr"];
            if (!parsSpiCommand(es)) {
                response["error"] = "spi-reg : parser error!!";
                return response;
            }
            calcSpi(x_lid, response);
        }
        //
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
            nco_channel_setup(x_lid, ch, fr, ph);
        else
            nco_main_setup(x_lid, ch, fr, ph);
        if(!error_.empty()) {
            response["error"] = error_.c_str();
            return response;
        }
    } else if (scmd == "setup") {
        ReadIniFileDevice(x_lid);
        SetAllDac(x_lid);
        SetMasterSlave(x_lid);

    } else if (scmd == "work" || scmd == "start") {
        /*if (request.contains("mode")) {
            std::string ms = request["mode"];
            g_nWorkMode = atoi(ms.c_str());
        }*/
        DevicesLid[x_lid].dacCtrlThr.isStoped.store(true);
        DevicesLid[x_lid].dacCtrlThr.thread = new std::thread(dacWork, x_lid);
        while (DevicesLid[x_lid].dacCtrlThr.isStoped.load())
            usleep(100);
        // WorkMode();
    } else if (scmd == "stop") {
        // Остановка рабочего потока
        DevicesLid[x_lid].dacCtrlThr.stop.store(true);
        //  Ожидание завершения рабочего потока
        bool wt = false;
        while(!wt) {
            wt = DevicesLid[x_lid].dacCtrlThr.isStoped.load();
            usleep(100);
        }
        ReleaseAllDac(x_lid);
        printf("<SRV> DAC-#%d released ..\n", x_lid);

    }   
    else if (scmd == "display-stop") {
        // Остановка вывода ЦАПа
        DevicesLid[x_lid].dac[0].outText = false;
        printf("<SRV> Display out for DAC-%d stoped ..\n", x_lid);
    }
    else if (scmd == "release") {
        // Остановка всех рабочих потоков
        for (size_t i = 1; i < MAX_LID_DEVICES; i++) {
            if (DevicesLid[i].device.isOpen() && !DevicesLid[i].dacCtrlThr.isStoped.load()) {
                DevicesLid[i].dacCtrlThr.stop.store(true);
                bool wt = false;
                while(!wt) {
                    wt = DevicesLid[i].dacCtrlThr.isStoped.load();
                    usleep(100);
                }
                ReleaseAllDac(i);
                printf("<SRV> DAC-#%d released ..\n", i);
            }         
        }        

    } else if (scmd == "list-parameters") {
        ListParameters(x_lid);
    } else {
        response["error"] = "Unrecognized error!!";
    };

    return response;
}

void DacControl::calcSpi(int lid, json& resp)
{
    S32 status;
    U32 mode = BRDcapt_SHARED;
    if(!DevicesLid[lid].device.isOpen()) {
        BRDC_printf(_BRDC("Device LID=(%d) d'nt opened\n"), lid);
        error_ = "Device d'nt opened !!";
        return;
    }

    BRD_Handle hSrv = BRD_capture(DevicesLid[lid].device.handle(), 0, &mode, _BRDC("REG0"), 5000);
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void DacControl::nco_main_setup(int lid, std::size_t chan, double freq, double phase)
{
    double freq_clk = 12000.0; // частота тактирования ЦАП в МГц
    auto DDSM_FTW = uint64_t(round((freq / (freq_clk * 1'000'000)) * std::pow(2, 48)));
    auto DDSM_PHASE = uint16_t(round((phase / 180.) * std::pow(2, 15)));

    if(!DevicesLid[lid].device.isOpen()) {
        BRDC_printf(_BRDC("Device LID=(%d) d'nt opened\n"), lid);
        error_ = "Device d'nt opened !!";
        return;
    }

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
    BRD_Handle hSrv = BRD_capture(DevicesLid[lid].device.handle(), 0, &mode, _BRDC("REG0"), 5000);
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
void DacControl::nco_channel_setup(int lid, std::size_t chan, double freq, double phase)
{
    double freq_clk = 12000.0; // частота тактирования ЦАП в МГц
    if(!DevicesLid[lid].device.isOpen()) {
        BRDC_printf(_BRDC("Device LID=(%d) d'nt opened\n"), lid);
        error_ = "Device d'nt opened !!";
        return;
    }

    U32 mode = BRDcapt_SHARED;
    BRD_Handle hSrv = BRD_capture(DevicesLid[lid].device.handle(), 0, &mode, _BRDC("REG0"), 5000);
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


////////////////////////////////////////////////////////////////// AdcControl

void AdcWork(int lid)
{
    DevicesLid[lid].adcCtrlThr.isStoped.store(false);
    DevicesLid[lid].adcCtrlThr.stop.store(false);    
    printf("<SRV> Thread for ADC-%d started ..\n", lid);
    workFlow(lid);
    DevicesLid[lid].adcCtrlThr.isStoped.store(true);
    printf("<SRV> Thread for ADC-%d stoped ..\n", lid);
}

json AdcControl::execute(const json& request)
{
    // printf("##### MARKER: ADC:execute begin\n");
    U32 x_mode = 0;
    json response = request;
    std::string message = "Parameter is missing: ";
    if (!request.contains("adc")) {
        response["error"] = message + "adc";
        return response;
    }

    std::string scmd = request["adc"];
    std::string sfile, slid;
    if (request.contains("path")) {
        sfile = request["path"];
    }
    if (request.contains("lid")) {
        slid = request["lid"];
    }    
    x_lid = 1;
    if (!slid.empty()) {
        x_lid = atoi(slid.c_str());
        if (x_lid <= 0 || x_lid >= MAX_LID_DEVICES) {
            BRDC_printf("<ERR> Bad LID number!\n");
            response["error"] = "Bad LID!";
            return response;
        }
    }

    if (scmd == "config") {
        if (sfile.empty()) {
            response["error"] = "ini-file dn't found ..";
            return response;
        }
        BRD_displayMode(BRDdm_VISIBLE | BRDdm_CONSOLE);         
        printLids();
        adcGetOptions(x_lid, sfile);
        return response;
    } 

    if (!DevicesLid[x_lid].device.isOpen()) {
        response["error"] = "Device d'nt opened!";
        if(scmd != "list-parameters")
            return response;
    } 
    x_handleDevice = DevicesLid[x_lid].device.handle();

    if (scmd == "setup") {
        int mode = BRDopen_EXCLUSIVE;
        if (request.contains("mode")) {
            std::string sm = request["mode"];
            mode = atoi(sm.c_str());
        }
        //DevicesLid[x_lid].device.reopen(x_lid);
        //x_handleDevice = DevicesLid[x_lid].device.handle();  
        puListLoad(x_lid);      
        if (!checkPower(x_lid)) {
            printf("<ERR> ADC power error .. \n");
            response["error"] = "ADC power error";
        }
        usleep(1000);
        DevicesLid[x_lid].adc.setMode(mode);
        if (!captureServiceAndSetParams(x_lid, mode)) {
            printf("<ERR> command SETUP error .. \n");
            response["error"] = "command SETUP error ..";
        }

    } 
    else if ( scmd == "work" || scmd == "start") {        
        if (x_handleDevice > 0) {
            DevicesLid[x_lid].adcCtrlThr.isStoped.store(true);
            DevicesLid[x_lid].adcCtrlThr.thread = new std::thread(AdcWork, x_lid);
            while (DevicesLid[x_lid].adcCtrlThr.isStoped.load())
                usleep(100);
        }
        else
            response["error"] = "the device is not open!";
    }       

    else if (scmd == "stop") {

        DevicesLid[x_lid].adcCtrlThr.stop.store(true);
        while(!DevicesLid[x_lid].adcCtrlThr.isStoped.load());
                usleep(100);
        releaseAdc(x_lid);
        printf("<SRV> ADC-%d released ..\n", x_lid);
    
    }         

    else if (scmd == "spi-reg") {
        if (request.contains("expr")) { // от expression
            std::string es = request["expr"];
            if (!parsSpiCommand(es)) {
                response["error"] = "spi-reg : parser error!!";
                return response;
            }
            calcSpi(x_lid, response);
        }

    } 
    else if (scmd == "release") {
       // Остановка всех рабочих потоков
       for (size_t i = 1; i < MAX_LID_DEVICES; i++) {
            if (DevicesLid[i].device.isOpen() && !DevicesLid[i].adcCtrlThr.isStoped.load()) {
                DevicesLid[i].adcCtrlThr.stop.store(true);
                bool wt = false;
                while(!wt) {
                    wt = DevicesLid[i].adcCtrlThr.isStoped.load();
                    usleep(100);
                }
                releaseAdc(i);
                printf("<SRV> ADC-%d released ..\n", i);
            }         
        }
    } else if (scmd == "list-parameters") {
        ListParametersAdc(x_lid);

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

void AdcControl::calcSpi(int lid, json& resp)
{
    S32 status;
    U32 mode = BRDcapt_SHARED;
    if(!DevicesLid[lid].device.isOpen()) {
        BRDC_printf(_BRDC("Device LID=(%d) d'nt opened\n"), lid);
        resp["error"] = "Device d'nt opened !!";
        return;
    }
    BRD_Handle hSrv = BRD_capture(DevicesLid[lid].device.handle(), 0, &mode, _BRDC("REG0"), 5000);
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
    printf("<SRV> Sync %s: %s  \n", scmd.c_str(), sfile.c_str());

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