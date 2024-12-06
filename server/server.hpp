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
#include "total.h"

using namespace std::chrono_literals;
using namespace std::string_literals;

using namespace InSys;

using json = nlohmann::json;

int globalDeviceIndex;

static bool is_cancel { false };

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

// Изменить IP адрес
void change_ip_addr(std::uint8_t geopos, std::string ifname)
{
    if (geopos > 154)
        throw std::logic_error { "Invalid geopos argument" };

    // неизменяемые в процессе работы параметры
    // TODO: получать реальные значения
    const auto addr = "192.168.5.75";
    const auto netmask = "255.255.255.0";
    const auto broadcast = "192.168.5.255";
    const auto gateway = "192.168.5.254";

    // определение нового адреса
    //   char addr[NI_MAXHOST];
    //   std::sprintf(addr, "192.168.1.%d", 100 + geopos);

    auto fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    ifreq ifr {};

    if_down(fd, ifr, ifname);

    // IP
    sockaddr_in ip;
    ip.sin_family = AF_INET;
    inet_aton(addr, reinterpret_cast<in_addr*>(&ip.sin_addr.s_addr));
    set_addr(fd, ifname, ip.sin_addr.s_addr, SIOCSIFADDR);

    // netmask
    sockaddr_in nm;
    nm.sin_family = AF_INET;
    inet_aton(netmask, reinterpret_cast<in_addr*>(&nm.sin_addr.s_addr));
    set_addr(fd, ifname, nm.sin_addr.s_addr, SIOCSIFNETMASK);

    // broadcast
    sockaddr_in bc;
    bc.sin_family = AF_INET;
    inet_aton(broadcast, reinterpret_cast<in_addr*>(&bc.sin_addr.s_addr));
    set_addr(fd, ifname, bc.sin_addr.s_addr, SIOCSIFBRDADDR);

    if_up(fd, ifr, ifname);

    // my
    std::fprintf(stdout, "IP=0x%X Port=%d\n Mask=0x%X", ip.sin_addr.s_addr, ip.sin_port, nm.sin_addr.s_addr);

    std::this_thread::sleep_for(5s);

    // после смены адреса таблица маршрутизации сбрасывается
    // add_route(fd, gateway);
}

class BRD_Service {
    uint32_t m_Mode {};
    BRD_Handle m_Handle {};
    brd_string m_Name {};

public:
    BRD_Handle getHandle() { return m_Handle; }
    using SharedPtr = std::shared_ptr<BRD_Service>;
    BRD_Service(BRD_Handle deviceHandle, const brd_string& serviceName,
        uint32_t timeout = 10000)
        : m_Mode { BRDcapt_SHARED }
        , m_Handle {}
        , m_Name { serviceName }
    {
        m_Handle = BRD_capture(deviceHandle, 0, &m_Mode, serviceName.c_str(), timeout);
        if (!m_Handle) {
            throw std::runtime_error("bardy: can't capture service " + std::to_string(m_Name));
        }
    }
    ~BRD_Service() noexcept
    {
        if (m_Handle) {
            auto status = BRD_release(m_Handle, 0);
            if (!BRD_errcmp(status, BRDerr_OK)) {
                std::fprintf(stderr, "bardy: can't release service %s (%d)\n",
                    std::to_string(m_Name).c_str(), status);
            }
        }
    }
    brd_string getName() const noexcept { return m_Name; }
    template <U32 Command, typename ValueType>
    int command(ValueType& value) noexcept
    {
        return BRD_ctrl(m_Handle, 0, Command, &value);
    }
};
using BRD_ServiceList = std::vector<BRD_Service::SharedPtr>;

class BRD_Device {
    BRD_Handle m_Handle {};
    BRD_Info m_Info {};
    BRD_ServiceList m_ServiceList {};

public:
    using SharedPtr = std::shared_ptr<BRD_Device>;
    BRD_Device(uint32_t lid, const BRD_Info& info)
        : m_Handle { BRD_open(lid, BRDopen_SHARED, NULL) }
        , m_Info { info }
    {
        if (!m_Handle) {
            throw std::runtime_error("bardy: can't open device");
        }
        std::vector<BRD_ServList> serviceInfoList {};
        const int servicesMax { 16 };
        serviceInfoList.resize(servicesMax);
        uint32_t itemReal {};
        auto status = BRD_serviceList(m_Handle, 0, serviceInfoList.data(),
            servicesMax, &itemReal);
        if (!BRD_errcmp(status, BRDerr_OK)) {
            throw std::runtime_error("bardy: no services found");
        }
        serviceInfoList.resize(itemReal);
        int seviceIdx {};
        for (auto serviceInfo : serviceInfoList) {
            auto serviceName = std::basic_string<BRDCHAR>(
                std::begin(serviceInfo.name), std::end(serviceInfo.name));
            serviceName.resize(BRDC_strlen(serviceName.c_str()));
            try {
                auto service = std::make_shared<BRD_Service>(m_Handle, serviceName);
                m_ServiceList.emplace_back(std::move(service));
                std::fprintf(stdout, "service_%d: %s\n", seviceIdx,
                    serviceName.c_str());
                ++seviceIdx;
            } catch (const std::exception& e) {
                std::fprintf(stderr, "\n%s\n", e.what());
            }
        }
    }
    ~BRD_Device() noexcept
    {
        if (m_Handle) {
            m_ServiceList.clear();
            auto status = BRD_close(m_Handle);
            if (!BRD_errcmp(status, BRDerr_OK)) {
                std::fprintf(stderr, "bardy: can't close device (%d)\n", status);
            }
        }
    }
    BRD_Info getInfo() const noexcept { return m_Info; }
    BRD_ServiceList const& serviceList() const noexcept { return m_ServiceList; }
    BRD_Service::SharedPtr findService(const brd_string& serviceName) const
    {
        for (auto& service : m_ServiceList) {
            if (serviceName.compare(service->getName()) == 0) {
                return service;
            }
        }
        throw std::runtime_error("bardy: no services found");
    }
};
using BRD_DeviceList = std::vector<BRD_Device::SharedPtr>;

class BRD_DeviceStore {

    BRD_DeviceList m_DeviceList {};

public:
    BRD_DeviceStore(const brd_string& iniFileName)
    {
        std::set_terminate([] { BRD_cleanup(); });
        int32_t deviceNums {};
        BRD_displayMode(BRDdm_WARN | BRDdm_ERROR | BRDdm_FATAL | BRDdm_CONSOLE);
        auto status = BRD_init(iniFileName.c_str(), &deviceNums);
        if (!BRD_errcmp(status, BRDerr_OK)) {
            throw std::runtime_error("bardy: init error\n");
        }
        std::vector<uint32_t> lidArray {};
        BRD_LidList lidList {};
        lidArray.resize(deviceNums);
        lidList.item = U32(lidArray.size());
        lidList.pLID = lidArray.data();
        status = BRD_lidList(lidList.pLID, lidList.item, &lidList.itemReal);
        if (!BRD_errcmp(status, BRDerr_OK)) {
            throw std::runtime_error("bardy: can't get lid list\n");
        }
        lidArray.resize(lidList.itemReal);
        lidList.item = U32(lidArray.size());
        int deviceIdx {};
        for (auto& lid : lidArray) {
            BRD_Info brdInfo {};
            brdInfo.size = sizeof(brdInfo);
            BRD_getInfo(lid, &brdInfo);
            auto deviceName = std::to_string(brdInfo.name);
            std::fprintf(stdout,
                "%s\n"
                "[LID:%d]\n"
                "device_%d: %s\n"
                "pid: %d\n"
                "dev:bus: %d:%d\n",
                "------------------", lid, deviceIdx, deviceName.c_str(),
                brdInfo.pid, brdInfo.dev, brdInfo.bus);
            try {
                auto device = std::make_shared<BRD_Device>(lid, brdInfo);
                m_DeviceList.emplace_back(std::move(device));
                ++deviceIdx;

            } catch (const std::exception& e) {
                std::fprintf(stderr, "\n%s\n", e.what());
            }
        }
        globalDeviceIndex = 0;
    }
    ~BRD_DeviceStore() noexcept
    {
        m_DeviceList.clear();
        BRD_cleanup();
    }
    BRD_DeviceList const& deviceList() const noexcept { return m_DeviceList; }
};

class CommandExecutor {
protected:
    BRD_DeviceList const& _devices;

public:
    using UniquePtr = std::unique_ptr<CommandExecutor>;
    CommandExecutor(const BRD_DeviceList& devices)
        : _devices { devices }
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
    int getServiceInstance(const json& json_tree)
    {
        const auto option = "instance";
        return json_tree.contains(option) ? static_cast<int>(json_tree[option]) : 0;
    }
    BRD_Service* getService(int index, int instance = 0, const brd_string& name = "REG")
    {
        brd_string serviceName = name + to_brd_string(instance);
        auto service = _devices.at(index)->findService(serviceName).get();
        assert(service != nullptr);
        return service;
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
            == -1)
            throw std::runtime_error("ERROR: bind function failed");

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
    void add_command(const BRD_DeviceList& deviceList)
    {
        commandMap.emplace(CommandExecutorType::command(),
            std::make_unique<CommandExecutorType>(deviceList));
    }
    void start()
    {
        std::thread th([this] {
            // ждём запрос соединения от клиента
            sockaddr_in client_addr {}; // адрес и порт клиента
            socklen_t client_addr_len = sizeof(client_addr);
            std::fprintf(stdout, "\nServer Listening...\n");
            std::fflush(stdout);
            while (true) {
                try {
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
                                printf("Packet data lenght = %d, exit from server\n", len);
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
                            /*
                            if (jsonRequestTree.contains("command")) {
                                auto command = jsonRequestTree["command"].get<std::string>();
                                jsonResponseTree["command"] = command;
                                jsonResponseTree["error"] = "unknown command";
                             1
                                //  отдаем дерево запроса в соответствующую команду и получаем
                                //  дерево результата
                                auto commandExecutor = commandMap.find(command);
                                if (commandExecutor != commandMap.end())
                                    jsonResponseTree = commandExecutor->second.get()->execute(jsonRequestTree);
                            } else
                            */
                            jsonResponseTree["error"] = "unknown parameter";
                            if (jsonRequestTree.contains("com")) {
                                auto command = jsonRequestTree["com"].get<std::string>();
                                jsonResponseTree["com"] = command;
                                // jsonResponseTree["error"] = "unknown command";
                                auto CE_icom = commandMap.find("com");
                                if (CE_icom != commandMap.end()) {
                                    jsonResponseTree = CE_icom->second.get()->execute(jsonRequestTree);
                                    // std::printf("Select com\n");
                                }
                            } else if (jsonRequestTree.contains("reg")) {
                                auto command = jsonRequestTree["reg"].get<std::string>();
                                jsonResponseTree["reg"] = command;
                                // jsonResponseTree["error"] = "unknown command";
                                auto CE_sreg = commandMap.find("reg");
                                if (CE_sreg != commandMap.end()) {
                                    jsonResponseTree = CE_sreg->second.get()->execute(jsonRequestTree);
                                    // std::printf("Select reg\n");
                                }
                            } else if (jsonRequestTree.contains("dac")) {
                                auto command = jsonRequestTree["dac"].get<std::string>();
                                jsonResponseTree["dac"] = command;
                                // jsonResponseTree["error"] = "unknown command";
                                auto commandExecutor = commandMap.find("dac");
                                if (commandExecutor != commandMap.end()) {
                                    jsonResponseTree = commandExecutor->second.get()->execute(jsonRequestTree);
                                    // std::printf("Select dac\n");
                                } else
                                    std::printf("Debug - commandExecutor dac not found ...\n");

                            } else if (jsonRequestTree.contains("adc")) {
                                auto command = jsonRequestTree["adc"].get<std::string>();
                                jsonResponseTree["adc"] = command;
                                // jsonResponseTree["error"] = "unknown command";
                                auto comEx = commandMap.find("adc");
                                if (comEx != commandMap.end()) {
                                    jsonResponseTree = comEx->second.get()->execute(jsonRequestTree);
                                    // std::printf("Select adc\n");
                                }
                            } else if (jsonRequestTree.contains("sync")) {
                                auto command = jsonRequestTree["sync"].get<std::string>();
                                jsonResponseTree["sync"] = command;
                                // jsonResponseTree["error"] = "unknown command";
                                auto comEx = commandMap.find("sync");
                                if (comEx != commandMap.end()) {
                                    jsonResponseTree = comEx->second.get()->execute(jsonRequestTree);
                                    // std::printf("Select sync\n");
                                }
                            } else {
                                jsonResponseTree["error"] = "unknown command";
                            }

                            // формируем из дерева запроса строку JSON для клиента
                            auto jsonStringToClient = CommandExecutor::toClient(jsonResponseTree);
                            send(_socket,
                                reinterpret_cast<const char*>(jsonStringToClient.data()),
                                jsonStringToClient.size(), 0);
                            // std::printf("<Send>: %s\n", jsonStringToClient.c_str());
                        }
                    }
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
    BRD_Service* _sysmon {};

public:
    static std::string command() { return "BRDctrl_SYSMON"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final
    {
        auto deviceIndex = getDeviceIndex(request);
        auto instance = getServiceInstance(request);
        _sysmon = getService(deviceIndex, instance, _BRDC("SYSMON"));

        json response = request;

        BRD_SysMonVal value {};
        auto status = _sysmon->command<BRDctrl_SYSMON_GETTEMP>(value);
        if (isCommandError(response, status, "GetTemp error"))
            return response;
        response["temp"] = value.curv;

        status = _sysmon->command<BRDctrl_SYSMON_GETVCCINT>(value);
        if (isCommandError(response, status, "GetVCCINT error"))
            return response;
        response["vcc_int"] = value.curv;

        status = _sysmon->command<BRDctrl_SYSMON_GETVCCAUX>(value);
        if (isCommandError(response, status, "GetVCCAUX error"))
            return response;
        response["vcc_aux"] = value.curv;

        status = _sysmon->command<BRDctrl_SYSMON_GETVREFP>(value);
        if (isCommandError(response, status, "GetVREFP error"))
            return response;
        response["vref_p"] = value.curv;

        status = _sysmon->command<BRDctrl_SYSMON_GETVREFN>(value);
        if (isCommandError(response, status, "GetVREFN error"))
            return response;
        response["vref_n"] = value.curv;

        status = _sysmon->command<BRDctrl_SYSMON_GETVCCBRAM>(value);
        if (isCommandError(response, status, "GetVCCBRAM error"))
            return response;
        response["vcc_bram"] = value.curv;

        return response;
    }
};

class WriteRegIndExecutor final : public CommandExecutor {
    BRD_Service* _reg {};

public:
    static std::string command() { return "BRDctrl_REG_WRITEIND"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final
    {
        auto deviceIndex = getDeviceIndex(request);
        auto instance = getServiceInstance(request);
        _reg = getService(deviceIndex, instance, _BRDC("REG"));

        json response = request;

        std::string message = "Parameter is missing: ";
        if (!request.contains("tetr")) {
            response["error"] = message + "'tetr'";
            return response;
        } else if (!request.contains("reg")) {
            response["error"] = message + "'reg'";
            return response;
        } else if (!request.contains("val")) {
            response["error"] = message + "'val'";
            return response;
        }

        BRD_Reg reg {};
        reg.tetr = request["tetr"];
        reg.reg = request["reg"];
        reg.val = request["val"];
        auto status = _reg->command<BRDctrl_REG_WRITEIND>(reg);
        if (isCommandError(response, status))
            return response;

        return response;
    }
};

class ReadRegIndExecutor final : public CommandExecutor {
    BRD_Service* _reg {};

public:
    static std::string command() { return "BRDctrl_REG_READIND"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final
    {
        auto deviceIndex = getDeviceIndex(request);
        auto instance = getServiceInstance(request);
        _reg = getService(deviceIndex, instance, _BRDC("REG"));

        json response = request;

        std::string message = "Parameter is missing: ";
        if (!request.contains("tetr")) {
            response["error"] = message + "'tetr'";
            return response;
        } else if (!request.contains("reg")) {
            response["error"] = message + "'reg'";
            return response;
        }

        BRD_Reg reg {};
        reg.tetr = request["tetr"];
        reg.reg = request["reg"];
        auto status = _reg->command<BRDctrl_REG_READIND>(reg);
        if (isCommandError(response, status))
            return response;
        response["val"] = reg.val;
        return response;
    }
};

class WriteRegDirExecutor final : public CommandExecutor {
    BRD_Service* _reg {};

public:
    static std::string command() { return "BRDctrl_REG_WRITEDIR"; }
    using CommandExecutor::CommandExecutor;

    json execute(const json& request) final
    {
        auto deviceIndex = getDeviceIndex(request);
        auto instance = getServiceInstance(request);
        _reg = getService(deviceIndex, instance, _BRDC("REG"));

        json response = request;

        std::string message = "Parameter is missing: ";
        if (!request.contains("tetr")) {
            response["error"] = message + "'tetr'";
            return response;
        } else if (!request.contains("reg")) {
            response["error"] = message + "'reg'";
            return response;
        } else if (!request.contains("val")) {
            response["error"] = message + "'val'";
            return response;
        }

        BRD_Reg reg {};
        reg.tetr = request["tetr"];
        reg.reg = request["reg"];
        reg.val = request["val"];
        auto status = _reg->command<BRDctrl_REG_WRITEDIR>(reg);
        if (isCommandError(response, status))
            return response;

        return response;
    }
};

class ReadRegDirExecutor final : public CommandExecutor {
    BRD_Service* _reg {};

public:
    static std::string command() { return "BRDctrl_REG_READDIR"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final
    {
        auto deviceIndex = getDeviceIndex(request);
        auto instance = getServiceInstance(request);
        _reg = getService(deviceIndex, instance, _BRDC("REG"));

        json response = request;

        std::string message = "Parameter is missing: ";
        if (!request.contains("tetr")) {
            response["error"] = message + "'tetr'";
            return response;
        } else if (!request.contains("reg")) {
            response["error"] = message + "'reg'";
            return response;
        }

        BRD_Reg reg {};
        reg.tetr = request["tetr"];
        reg.reg = request["reg"];
        auto status = _reg->command<BRDctrl_REG_READDIR>(reg);
        if (isCommandError(response, status))
            return response;

        response["val"] = reg.val;
        return response;
    }
};

class WriteSpdExecutor final : public CommandExecutor {
    BRD_Service* _reg {};

public:
    static std::string command() { return "BRDctrl_REG_WRITESPD"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final
    {
        auto deviceIndex = getDeviceIndex(request);
        auto instance = getServiceInstance(request);
        _reg = getService(deviceIndex, instance, _BRDC("REG"));

        json response = request;

        std::string message = "Parameter is missing: ";
        if (!request.contains("dev")) {
            response["error"] = message + "'dev'";
            return response;
        } else if (!request.contains("num")) {
            response["error"] = message + "'num'";
            return response;
        } else if (!request.contains("tetr")) {
            response["error"] = message + "'tetr'";
            return response;
        } else if (!request.contains("reg")) {
            response["error"] = message + "'reg'";
            return response;
        } else if (!request.contains("val")) {
            response["error"] = message + "'val'";
            return response;
        }

        BRD_Spd reg {};
        reg.dev = request["dev"];
        reg.sync = request.contains("is32bits") ? int(request["is32bits"]) : 0;
        reg.num = request["num"];
        reg.sync = request.contains("synchr") ? int(request["synchr"]) : 0;
        reg.tetr = request["tetr"];
        reg.reg = request["reg"];
        reg.val = request["val"];
        auto status = _reg->command<BRDctrl_REG_WRITESPD>(reg);
        if (isCommandError(response, status))
            return response;

        return response;
    }
};

class ReadSpdExecutor final : public CommandExecutor {
    BRD_Service* _reg {};

public:
    static std::string command() { return "BRDctrl_REG_READSPD"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final
    {
        auto deviceIndex = getDeviceIndex(request);
        auto instance = getServiceInstance(request);
        _reg = getService(deviceIndex, instance, _BRDC("REG"));

        json response = request;

        std::string message = "Parameter is missing: ";
        if (!request.contains("dev")) {
            response["error"] = message + "'dev'";
            return response;
        } else if (!request.contains("num")) {
            response["error"] = message + "'num'";
            return response;
        } else if (!request.contains("tetr")) {
            response["error"] = message + "'tetr'";
            return response;
        } else if (!request.contains("reg")) {
            response["error"] = message + "'reg'";
            return response;
        }

        BRD_Spd reg {};
        reg.dev = request["dev"];
        reg.sync = request.contains("is32bits") ? int(request["is32bits"]) : 0;
        reg.num = request["num"];
        reg.sync = request.contains("synchr") ? int(request["synchr"]) : 0;
        reg.tetr = request["tetr"];
        reg.reg = request["reg"];
        auto status = _reg->command<BRDctrl_REG_READSPD>(reg);
        if (isCommandError(response, status))
            return response;

        response["val"] = reg.val;
        return response;
    }
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class FastRegsAccess final : public CommandExecutor {
    BRD_Service* _reg {};
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
    BRD_Service* _reg {};
    int indexDev;
    commandLineParams paramReg;

public:
    static std::string command() { return "com"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final;
    bool isCommand(const json& request, json& response, std::string& cmd, std::string& param, commandLineParams& params);
};

//////////////////////////////////////////////////////////////////////
class AdcControl final : public CommandExecutor {
    BRD_Service* _reg {};
    int indexDev;
    S16 regNum_;
    S16 value_;
    bool write_;
    double freq_clk = 0.0; // частота тактирования ЦАП в МГц
public:
    static std::string command() { return "adc"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final;
    bool parsSpiCommand(const std::string es);
    void calcSpi(json& resp);
    void nco_main_setup(std::size_t chan, double freq, double phase);
    void nco_channel_setup(std::size_t chan, double freq, double phase);
    size_t findTetrad() { return 4; }
};

//////////////////////////////////////////////////////////////////////
class DacControl final : public CommandExecutor {
    BRD_Service* _reg {};
    int indexDev;
    S16 regNum_;
    S16 value_;
    bool write_;

public:
    static std::string command() { return "dac"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final;
    void calcSpi(json& resp);
    size_t findTetrad() { return 7; }
    bool parsSpiCommand(const std::string es);
};

//////////////////////////////////////////////////////////////////////
class FmcSync final : public CommandExecutor {
    BRD_Service* _reg {};
    int indexDev;

public:
    static std::string command() { return "sync"; }
    using CommandExecutor::CommandExecutor;
    json execute(const json& request) final;
};
