// clang-format off

#include "server.hpp"
#include "tool_hub.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <cstdlib>

#include "cmd/sync_cmd.hpp"
#include "cmd/proc_cmd.hpp"
#include "cmd/adc_cmd.hpp"
#include "cmd/dac_cmd.hpp"
#include "srv_exec.hpp"
#include "total.h"

S32 x_DevNum = -1;
FuDevs DevicesLid[MAX_LID_DEVICES];
int x_lid = 1;
BRD_Handle x_handleDevice = 0;

int main() try {
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
    // serverExecutor.add_command<GetSysmonExecutor>();
    serverExecutor.add_command<CommandProcessor>();
    serverExecutor.add_command<DacControl>();
    serverExecutor.add_command<AdcControl>();
    serverExecutor.add_command<FmcSync>();

    // запускаем поток обработки команд
    
    bool ok = ToolHub::inst().init();
    if (!ok) {
        BRDC_printf("Socket init error\n");
        return false;
    }

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
}
catch (const std::exception& e) {
    std::fprintf(stderr, "\n%s\n", e.what());
    return EXIT_FAILURE;
}
catch (...) {
    std::fprintf(stderr, "\nUnexpected exception!\n");
    return EXIT_FAILURE;
}

fs::path getExePath() {
    char buf[PATH_MAX];
    const ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n < 0)
        throw std::runtime_error("readlink(/proc/self/exe) failed");
    buf[n] = '\0';
    return fs::path(buf);
}

std::string get_interface()
{
    ifaddrs* addrs = nullptr;
    getifaddrs(&addrs);

    auto iface = addrs;
    while (iface != nullptr) {
        if (iface->ifa_addr && iface->ifa_addr->sa_family == AF_PACKET) {
            auto status = (IFF_UP | IFF_BROADCAST | IFF_RUNNING);
            if ((iface->ifa_flags & status) == status) {
                std::string host { iface->ifa_name };
                freeifaddrs(addrs);
                return host;
            }
        }

        iface = iface->ifa_next;
    }

    freeifaddrs(addrs);
    return "";
}
