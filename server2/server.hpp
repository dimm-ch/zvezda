#pragma once

#include <algorithm>
#include <arpa/inet.h>
#include <array>
#include <cassert>
#include <csignal>
#include <cstring>
#include <errno.h>
#include <future>
#include <ifaddrs.h>
#include <iomanip>
#include <map>
#include <memory>
#include <net/if.h>
#include <net/route.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>

#include "json.hpp"

// #include "adm2if.h"
#include "brd_string.h"
#include "brdapi.h"
#include "gipcy.h"

#include "ctrladc.h"
#include "ctrldac.h"
#include "ctrlreg.h"
#include "ctrlsysmon.h"

#include "../work2/common/dev_util.h"
#include "adc/adc_ctrl.h"
#include "dac/exam_edac.h"
#include "sync/fmc146v_sync.h"

using namespace std::chrono_literals;
using namespace std::string_literals;

using namespace InSys;

using json = nlohmann::json;

int globalDeviceIndex;

static bool is_cancel { false };

enum CmdExcCtrl {
    UNDEF,
    DESC,
    END
};

struct ExcCtrl {
    ExcCtrl()
        : ctrl(CmdExcCtrl::UNDEF)
    {
    }
    CmdExcCtrl ctrl;
    std::string desc;
};

// Отключить интерфейс
void if_down(int fd, ifreq& ifr, std::string ifname)
{
    std::strncpy(ifr.ifr_name, ifname.c_str(), IFNAMSIZ);
    auto err = ioctl(fd, SIOCGIFFLAGS, &ifr);
    if (err < 0) {
        std::fprintf(stderr, "[ERROR] if down get flags \"%s\"\n", strerror(errno));
        throw std::runtime_error { "IOCTL_ERROR in if_down" };
    }
    std::strncpy(ifr.ifr_name, ifname.c_str(), IFNAMSIZ);
    ifr.ifr_flags &= ~IFF_UP;
    err = ioctl(fd, SIOCSIFFLAGS, &ifr);
    if (err < 0) {
        std::fprintf(stderr, "[ERROR] if down set flags \"%s\"\n", strerror(errno));
        std::runtime_error { "IOCTL_ERROR in if_down" };
    }
}

// Включить интерфейс
void if_up(int fd, ifreq& ifr, std::string ifname)
{
    std::strncpy(ifr.ifr_name, ifname.c_str(), IFNAMSIZ);
    auto err = ioctl(fd, SIOCGIFFLAGS, &ifr);
    if (err < 0) {
        std::fprintf(stderr, "[ERROR] if up get flags \"%s\"\n", strerror(errno));
        std::runtime_error { "IOCTL_ERROR in if_up" };
    }
    std::strncpy(ifr.ifr_name, ifname.c_str(), IFNAMSIZ);
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
    err = ioctl(fd, SIOCSIFFLAGS, &ifr);
    if (err < 0) {
        std::fprintf(stderr, "[ERROR] if up set flags \"%s\"\n", strerror(errno));
        std::runtime_error { "IOCTL_ERROR in if_up" };
    }
}

// Установить какой-либо адрес IPv4 (IP, netmask, broadcast) в зависимости от
// req
void set_addr(int fd, std::string ifname, unsigned long ip, unsigned long req)
{
    ifreq ifr {};
    sockaddr_in sin {};

    std::strncpy(ifr.ifr_name, ifname.c_str(), IFNAMSIZ);
    std::memset(&sin, 0, sizeof(sockaddr));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip;
    std::memcpy(&ifr.ifr_addr, &sin, sizeof(sockaddr));
    auto err = ioctl(fd, req, &ifr);
    if (err < 0) {
        std::fprintf(stderr, "[ERROR] set addr \"%s\"\n", strerror(errno));
        throw std::runtime_error { "IOCTL_ERROR in set_addr" };
    }
}

// Добавить в таблицу маршрутизации шлюз по-умолчанию
void add_route(int fd, std::string gateway)
{
    rtentry route {};
    std::memset(&route, 0, sizeof(route));

    auto addr = reinterpret_cast<sockaddr_in*>(&route.rt_gateway);
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(gateway.c_str());

    addr = reinterpret_cast<sockaddr_in*>(&route.rt_dst);
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr("0.0.0.0");

    addr = reinterpret_cast<sockaddr_in*>(&route.rt_genmask);
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = INADDR_ANY;

    route.rt_flags = RTF_UP | RTF_GATEWAY;
    route.rt_metric = 100;

    auto err = ioctl(fd, SIOCADDRT, &route);
    if (err < 0) {
        std::fprintf(stderr, "[ERROR] add route \"%s\"\n", strerror(errno));
        throw std::runtime_error { "IOCTL_ERROR in add_route" };
    }
}

// Получить имя интерфейса, с которым можно работать
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

class CommandExecutor {
protected:
public:
    using UniquePtr = std::unique_ptr<CommandExecutor>;
    CommandExecutor()
    {
    }
    virtual ~CommandExecutor() noexcept = default;

    static json fromClient(const std::string& str)
    {
        return json::parse(str.begin(), str.end());
    }

    static std::string toClient(const json& data)
    {
        std::stringstream ss {};
        ss << data;
        return ss.str();
    }

    virtual json execute(const json&) = 0;

    int getDeviceIndex(const json& json_tree)
    {
        const auto option = "device";
        return json_tree.contains(option) ? static_cast<int>(json_tree[option]) : 0;
    }

    bool isCommandError(json& json_tree, int status,
        const std::string message = "Command Error")
    {
        if (!BRD_errcmp(status, BRDerr_OK)) {
            json_tree["error"] = message;
            return true;
        }
        return false;
    }
};

class ServerExecutor {
    using CommandMap = std::map<std::string, CommandExecutor::UniquePtr>;
    CommandMap commandMap {};
    int _server {};
    size_t _socket {};
    std::exception_ptr threadError {};

public:
    ServerExecutor(std::string ip = {}, uint16_t port = 7777)
    {
        // сервер
        _server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        // FIXME: нужно принимать соединения только на текущем интерфейсе
        // !!! Временно входящие соединения принимаем с любого интерфейса
        sockaddr_in server_addr {};
        server_addr.sin_family = AF_INET; // UDP, TCP, etc.
        server_addr.sin_port = htons(port);
        if (ip.empty()) {
            server_addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
        } else {
            server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
        }
        // my
        std::fprintf(stdout, "ServerExecutor: ip=%s ip_in=0x%X \n", ip.c_str(), server_addr.sin_addr.s_addr);
        if (bind(_server, reinterpret_cast<sockaddr*>(&server_addr),
                sizeof(server_addr))
            == -1) {
            // if_down();
            // if_up();
            //_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (bind(_server, reinterpret_cast<sockaddr*>(&server_addr),
                    sizeof(server_addr))
                == -1)
                throw std::runtime_error("ERROR: bind function failed");
        }
        if (listen(_server, SOMAXCONN) == -1)
            throw std::runtime_error("ERROR: listen function failed");
    }
    ~ServerExecutor() noexcept
    {
        if (_server) {
            close(_server);
            commandMap.clear();
        }
    }
    template <typename CommandExecutorType>
    void add_command()
    {
        commandMap.emplace(CommandExecutorType::command(),
            std::make_unique<CommandExecutorType>());
    }
    void start()
    {
        std::thread th([this] {
            // ждём запрос соединения от клиента
            sockaddr_in client_addr {}; // адрес и порт клиента
            socklen_t client_addr_len = sizeof(client_addr);
            while (true) {
                try {
                m1:
                    std::fprintf(stdout, "\nServer Listening...\n");
                    std::fflush(stdout);

                    _socket = accept(_server, reinterpret_cast<sockaddr*>(&client_addr),
                        &client_addr_len);
                    if (_socket == 0) {
                        continue;
                    } else {
                        // обработка соединения
                        int result {}; // результат операций с сокетом
                        int len {}; // фактическое количество принятых байт
                        std::array<uint8_t, 4096> buffer {}; // буфер приёма данных

                        fprintf(stdout, "Connect from %s:%d socket:%zd\n",

                            inet_ntoa(client_addr.sin_addr),
                            ntohs(client_addr.sin_port), _socket);

                        // установка тайм-аута 2 секунды на приём и передачу
                        timeval timeout { .tv_sec = 2, .tv_usec = 0 };
                        result = setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO,
                            reinterpret_cast<char*>(&timeout), sizeof(timeout));
                        result |= setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO,
                            reinterpret_cast<char*>(&timeout), sizeof(timeout));
                        if (result)
                            throw std::runtime_error(
                                "ERROR: setsockopt() timeout is not setting");
                        // получение и отправка данных
                        while (true) {
                            buffer.fill('\0');
                            len = recv(_socket, reinterpret_cast<char*>(buffer.data()),
                                buffer.size(), 0);
                            if (len <= 0) {
                                printf("Packet data lenght = %d, client disconnected ..\n", len);
                                // shutdown(_socket, SHUT_RDWR);
                                // close(_socket);
                                goto m1;
                                break; // тайм-аут ожидания данных или закрытый сокет
                            }
                            std::printf("---------------------------------------------------\n");
                            std::printf("<Recive command>: %s\n", buffer.data());

                            json jsonRequestTree {};
                            // формируем ответ с неизвестной коммандой
                            json jsonResponseTree {};
                            try {
                                // разбираем JSON строку от клиента в дерево запроса
                                jsonRequestTree = CommandExecutor::fromClient(std::to_string(buffer.data()));
                            } catch (const json::exception& e) {
                                jsonResponseTree["error"] = e.what();
                                // формируем из дерева запроса строку JSON для клиента
                                auto jsonStringToClient = CommandExecutor::toClient(jsonResponseTree);
                                send(_socket,
                                    reinterpret_cast<const char*>(jsonStringToClient.data()),
                                    jsonStringToClient.size(), 0);
                                // std::printf("<Send>: %s\n", jsonStringToClient.c_str());
                                continue;
                            }

                            // получаем команду которая поступила от клиента
                            jsonResponseTree["error"] = "unknown parameter";
                            for (const auto& [key, obj] : commandMap) {
                                if (jsonRequestTree.contains(key)) {
                                    jsonResponseTree = obj->execute(jsonRequestTree);
                                }
                            }
                            // формируем из дерева запроса строку JSON для клиента
                            auto jsonStringToClient = CommandExecutor::toClient(jsonResponseTree);
                            send(_socket,
                                reinterpret_cast<const char*>(jsonStringToClient.data()),
                                jsonStringToClient.size(), 0);
                            // std::printf("<Send>: %s\n", jsonStringToClient.c_str());
                        }
                    }
                } catch (ExcCtrl ec) {
                    fprintf(stdout, "<SRV> Exception - %s", ec.desc.c_str());
                    if (ec.ctrl != END) { };
                } catch (...) {
                    threadError = std::current_exception();
                }
                if (_socket) {
                    // разрываем соединение и закрываем сокет
                    fprintf(stdout, "Close connection %s:%d socket:%zd\n",
                        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
                        _socket);
                    shutdown(_socket, SHUT_RDWR);
                    close(_socket);
                }
            }
        });
        th.detach();
    }
    std::exception_ptr error() { return threadError; }
};

class GetSysmonExecutor final : public CommandExecutor {

public:
    static std::string command() { return "BRDctrl_SYSMON"; }
    json execute(const json& request) final
    {
        json response = request;
        return response;
    }
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class FastRegsAccess final : public CommandExecutor {
    commandLineParams paramReg;
    int indexDev;

public:
    static std::string command() { return "reg"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final;
    bool parsingLine(std::string& line, commandLineParams& params);
};

//////////////////////////////////////////////////////////////////////
class CommandProcessor final : public CommandExecutor {
    int indexDev;
    commandLineParams paramReg;

public:
    static std::string command() { return "com"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final;
    void clearAll();
    bool isCommand(const json& request, json& response, std::string& cmd, std::string& param, commandLineParams& params);
};

//////////////////////////////////////////////////////////////////////
class AdcControl final : public CommandExecutor {
    int indexDev;
    S16 regNum_;
    S16 value_;
    bool write_;

public:
    static std::string command() { return "adc"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final;
    bool parsSpiCommand(const std::string es);
    void calcSpi(int lid, json& resp);

    size_t findTetrad() { return 4; }
};

//////////////////////////////////////////////////////////////////////
class DacControl final : public CommandExecutor {
    int indexDev;
    S16 regNum_;
    S16 value_;
    bool write_;
    std::string error_;

public:
    static std::string command() { return "dac"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final;
    void calcSpi(int lid, json& resp);
    size_t findTetrad() { return 7; }
    void nco_main_setup(int lid, std::size_t chan, double freq, double phase);
    void nco_channel_setup(int lid, std::size_t chan, double freq, double phase);
    bool parsSpiCommand(const std::string es);
};

//////////////////////////////////////////////////////////////////////
class FmcSync final : public CommandExecutor {
    int indexDev;

public:
    static std::string command() { return "sync"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final;
};
