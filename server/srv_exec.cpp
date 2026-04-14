#include "srv_exec.hpp"

#include <array>
#include <chrono>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <thread>

#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "json.hpp"

#include "server.hpp"

ServerExecutor::ServerExecutor(std::string ip, uint16_t port) {
    _server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

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
    if (bind(_server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == -1) {
        if (bind(_server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == -1)
            throw std::runtime_error("ERROR: bind function failed");
    }
    if (listen(_server, SOMAXCONN) == -1)
        throw std::runtime_error("ERROR: listen function failed");
}

ServerExecutor::~ServerExecutor() noexcept {
    if (_server) {
        close(_server);
        commandMap.clear();
    }
}

void ServerExecutor::start() {
    std::thread th([this] {
        using clock = std::chrono::steady_clock;
        
        while (true) { // ждём запрос соединения от клиента
            sockaddr_in client_addr {}; // адрес и порт клиента
            socklen_t client_addr_len = sizeof(client_addr);

            try {
                std::fprintf(stdout, "\nServer Listening...\n");
                std::fflush(stdout);

                _socket = accept(_server, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);

                if (_socket < 0) { // ошибка accept — логируем и переходим к следующей итерации
                    std::perror("accept");
                    continue;
                }

                std::fprintf(stdout, "Connect from %s:%d socket:%zd\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), static_cast<ssize_t>(_socket));
                
                timeval timeout { .tv_sec = 2, .tv_usec = 0 };

                int result = 0;
                result = setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));
                result |= setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));
                if (result != 0) {
                    throw std::runtime_error("ERROR: setsockopt() timeout is not setting");
                }

                // время последнего полученного пакета
                auto last_data_time = clock::now();

                int len {};   // фактическое количество принятых байт
                std::array<uint8_t, 4096> buffer {}; // буфер приёма данных

                // получение и отправка данных
                while (true) {
                    buffer.fill('\0');
                    len = ::recv(_socket, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0);
                    if (len > 0) {
                        // пришли данные
                        last_data_time = clock::now();

                        std::printf("---------------------------------------------------\n");
                        std::printf("<Receive command>: %.*s\n", len, reinterpret_cast<char*>(buffer.data()));

                        json jsonRequestTree {};
                        json jsonResponseTree {};

                        try {
                            // собираем строку из буфера
                            std::string requestStr(reinterpret_cast<char*>(buffer.data()), static_cast<std::size_t>(len));
                            // разбираем JSON строку от клиента в дерево запроса
                            //jsonRequestTree = CommandExecutor::fromClient(requestStr);
                            jsonRequestTree = json::parse(requestStr.begin(), requestStr.end());
                        }
                        catch (const json::exception& e) {
                            jsonResponseTree["error"] = e.what();
                            //auto jsonStringToClient = CommandExecutor::toClient(jsonResponseTree);
                            auto jsonStringToClient = jsonResponseTree.dump();
                            ::send(_socket, reinterpret_cast<const char*>(jsonStringToClient.data()), jsonStringToClient.size(), 0);
                            continue;
                        }

                        // получаем команду, которая поступила от клиента
                        jsonResponseTree["error"] = "unknown parameter";
                        for (const auto& [key, obj] : commandMap) {
                            if (jsonRequestTree.contains(key)) {
                                jsonResponseTree = obj->execute(jsonRequestTree);
                            }
                        }

                        // формируем ответ клиенту
                        //auto jsonStringToClient = CommandExecutor::toClient(jsonResponseTree);
                        auto jsonStringToClient = jsonResponseTree.dump();
                        ::send(_socket, reinterpret_cast<const char*>(jsonStringToClient.data()), jsonStringToClient.size(), 0);
                    }
                    else if (len == 0) { // клиент корректно закрыл соединение
                        std::printf("Client closed connection (len = 0)\n");
                        break;
                    }
                    else { // len < 0 — ошибка
                        if (errno == EAGAIN || errno == EWOULDBLOCK) { // истёк SO_RCVTIMEO — данных не было 2 секунды
                            auto now  = clock::now();
                            auto idle = std::chrono::duration_cast<std::chrono::seconds>(now - last_data_time).count();

                            // если с момента последнего успешного приёма прошёл час — рвём соединение
                            if (idle > 30) {
                                std::printf("Idle timeout > 30 sec, closing connection\n");
                                break;
                            }

                            // иначе просто ждём дальше
                            continue;
                        }
                        else {
                            // ошибка recv
                            std::perror("recv");
                            break;
                        }
                    }
                } // while (true) при обслуживании одного клиента
            }
            catch (ExcCtrl& ec) {
                std::fprintf(stdout, "<SRV> Exception - %s\n", ec.desc.c_str());
                if (ec.ctrl != END) {
                    // здесь можно что-то сделать, если нужно
                }
            }
            catch (const std::exception& e) {
                std::fprintf(stdout, "<SRV> std::exception: %s\n", e.what());
                threadError = std::current_exception();
            }
            catch (...) {
                std::fprintf(stdout, "<SRV> unknown exception\n");
                threadError = std::current_exception();
            }

            // разрываем соединение и закрываем сокет, если он был успешно открыт
            if (_socket >= 0) {
                std::fprintf(stdout, "Close connection %s:%d socket:%zd\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), static_cast<ssize_t>(_socket));

                ::shutdown(_socket, SHUT_RDWR);
                ::close(_socket);
                _socket = -1;
            }
        } // while(true) — ждём новых клиентов
    });

    th.detach();
}

std::exception_ptr ServerExecutor::error() {
    return threadError;
}