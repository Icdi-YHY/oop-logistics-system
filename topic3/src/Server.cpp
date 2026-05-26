#include "../include/Server.h"
#include "../include/Protocol.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <map>
#include <cctype>

using namespace Protocol;

struct ThreadParam {
    Server* server;
    SOCKET clientSocket;
    int clientId;
};

Server::Server(int port)
    : listenSocket_(INVALID_SOCKET), port_(port), running_(false), nextClientId_(1)
{
    InitializeCriticalSection(&cs_);
    dataManager_.loadData();
}

Server::~Server()
{
    stop();
    DeleteCriticalSection(&cs_);
}

void Server::start()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return;
    }

    listenSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port_);

    if (bind(listenSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(listenSocket_);
        WSACleanup();
        return;
    }

    if (listen(listenSocket_, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        closesocket(listenSocket_);
        WSACleanup();
        return;
    }

    std::cout << "Server started on port " << port_ << std::endl;
    running_ = true;

    while (running_) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket_, (sockaddr*)&clientAddr, &clientAddrSize);

        if (clientSocket == INVALID_SOCKET) {
            if (running_) std::cerr << "Accept failed" << std::endl;
            continue;
        }

        int cid = nextClientId_++;
        std::cout << "[客户端#" << cid << " 连接] " << inet_ntoa(clientAddr.sin_addr) << std::endl;

        ThreadParam* param = new ThreadParam();
        param->server = this;
        param->clientSocket = clientSocket;
        param->clientId = cid;

        HANDLE hThread = CreateThread(NULL, 0,
            [](LPVOID lpParam) -> DWORD {
                ThreadParam* p = (ThreadParam*)lpParam;
                p->server->handleClient(p->clientSocket, p->clientId);
                closesocket(p->clientSocket);
                delete p;
                return 0;
            },
            param, 0, NULL);

        if (hThread != NULL) CloseHandle(hThread);
    }

    closesocket(listenSocket_);
    WSACleanup();
}

void Server::stop()
{
    running_ = false;
}

std::string Server::formatDate(const std::string& timeStr)
{
    if (timeStr.empty()) return "";

    if (timeStr.length() >= 10 && isdigit(timeStr[0]) && timeStr[4] == '-' && timeStr[7] == '-') {
        return timeStr.substr(0, 10);
    }

    std::map<std::string, std::string> monthMap = {
        {"Jan", "01"}, {"Feb", "02"}, {"Mar", "03"}, {"Apr", "04"},
        {"May", "05"}, {"Jun", "06"}, {"Jul", "07"}, {"Aug", "08"},
        {"Sep", "09"}, {"Oct", "10"}, {"Nov", "11"}, {"Dec", "12"}
    };

    std::istringstream iss(timeStr);
    std::string weekday, month, day, time, year;
    iss >> weekday >> month >> day >> time >> year;

    if (day.length() == 1) day = "0" + day;
    return year + "-" + monthMap[month] + "-" + day;
}

void Server::handleClient(SOCKET clientSocket, int clientId)
{
    int userType = 0;
    std::string sessionUsername;

    char buffer[8192];
    while (running_) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            std::cout << "  [#" << clientId << " 断开] " << sessionUsername << std::endl;
            break;
        }

        std::string request(buffer);
        std::istringstream reqStream(request);
        std::string singleRequest;
        while (std::getline(reqStream, singleRequest)) {
            if (singleRequest.empty()) continue;

            std::string logCmd = singleRequest.substr(0, singleRequest.find('|'));
            std::cout << "  [#" << clientId << " 请求] " << sessionUsername << " -> " << logCmd << std::endl;

            std::string response = processRequest(singleRequest, userType, sessionUsername);
            send(clientSocket, response.c_str(), response.length(), 0);

            if (response.find("OK") == 0) {
                std::string msg = response.substr(3, response.find('\n') - 3);
                std::cout << "  [#" << clientId << " 成功] " << msg << std::endl;
            } else if (response.find("ERR") == 0) {
                std::string msg = response.substr(4, response.find('\n') - 4);
                std::cout << "  [#" << clientId << " 失败] " << msg << std::endl;
            }

            if (singleRequest.find(CMD_EXIT) == 0) return;
        }
    }
}

// ==================== 请求分发 ====================

bool Server::needsSave(const std::string& cmd) const
{
    return cmd == CMD_REGISTER || cmd == CMD_SEND_PACKAGE || cmd == CMD_RECEIVE_PACKAGE ||
           cmd == CMD_RECHARGE || cmd == CMD_CHANGE_PWD || cmd == CMD_ADMIN_ADD_COURIER ||
           cmd == CMD_ADMIN_REMOVE_COURIER || cmd == CMD_ADMIN_ASSIGN_COURIER ||
           cmd == CMD_COURIER_COLLECT || cmd == CMD_COURIER_CHANGE_PWD;
}

std::string Server::processRequest(const std::string& request, int& userType, std::string& sessionUsername)
{
    std::istringstream ss(request);
    std::string cmd;
    std::getline(ss, cmd, DELIMITER);

    std::vector<std::string> params;
    std::string param;
    while (std::getline(ss, param, DELIMITER)) {
        params.push_back(param);
    }

    EnterCriticalSection(&cs_);

    std::string response;
    try {
        if (cmd == CMD_LOGOUT) {
            userType = 0;
            sessionUsername = "";
            response = buildOkResponse("Logged out");
        } else if (userType == 1) {
            response = dispatchUserCommand(cmd, params, sessionUsername);
        } else if (userType == 2) {
            response = dispatchAdminCommand(cmd, params);
        } else if (userType == 3) {
            response = dispatchCourierCommand(cmd, params, sessionUsername);
        } else {
            response = dispatchUnauthCommand(cmd, params, userType, sessionUsername);
        }
    } catch (const std::exception& e) {
        response = buildErrResponse(std::string("Server error: ") + e.what());
    } catch (...) {
        response = buildErrResponse("Unknown server error");
    }

    LeaveCriticalSection(&cs_);

    if (needsSave(cmd)) {
        EnterCriticalSection(&cs_);
        dataManager_.saveData();
        LeaveCriticalSection(&cs_);
    }

    return response;
}

std::string Server::dispatchUnauthCommand(const std::string& cmd, const std::vector<std::string>& params,
                                          int& userType, std::string& sessionUsername)
{
    if (cmd == CMD_LOGIN && params.size() >= 3) {
        return handleLogin(params[0], params[1], params[2], userType, sessionUsername);
    }
    if (cmd == CMD_REGISTER && params.size() >= 5) {
        return handleRegister(params[0], params[1], params[2], params[3], params[4]);
    }
    if (cmd == CMD_EXIT) {
        return buildOkResponse("Goodbye");
    }
    return buildErrResponse("Please login first");
}

std::string Server::dispatchUserCommand(const std::string& cmd, const std::vector<std::string>& params,
                                        const std::string& sessionUsername)
{
    if (cmd == CMD_SEND_PACKAGE && params.size() >= 4) {
        return handleSendPackage(sessionUsername, params[0],
                                 std::stoi(params[1]), std::stod(params[2]), params[3]);
    }
    if (cmd == CMD_RECEIVE_PACKAGE) {
        return handleReceivePackage(sessionUsername, params.empty() ? "" : params[0]);
    }
    if (cmd == CMD_QUERY_SENT)           return handleQuerySent(sessionUsername);
    if (cmd == CMD_QUERY_RECEIVED)       return handleQueryReceived(sessionUsername);
    if (cmd == CMD_GET_BALANCE)          return handleBalanceQuery(sessionUsername);

    if (cmd == CMD_QUERY_BY_ID && params.size() >= 1)
        return handleQueryById(params[0]);
    if (cmd == CMD_QUERY_BY_TIME_SENT && params.size() >= 2)
        return handleQueryByTime(sessionUsername, "sent", params[0], params[1], true);
    if (cmd == CMD_QUERY_BY_TIME_RECEIVED && params.size() >= 2)
        return handleQueryByTime(sessionUsername, "received", params[0], params[1], false);
    if (cmd == CMD_RECHARGE && params.size() >= 1)
        return handleRecharge(sessionUsername, std::stod(params[0]));
    if (cmd == CMD_CHANGE_PWD && params.size() >= 2)
        return handleChangePwd(sessionUsername, params[0], params[1]);

    return buildErrResponse("Invalid command or insufficient permissions");
}

std::string Server::dispatchAdminCommand(const std::string& cmd, const std::vector<std::string>& params)
{
    if (cmd == CMD_ADMIN_SHOW_USERS)            return handleAdminShowUsers();
    if (cmd == CMD_ADMIN_SHOW_PACKAGES)          return handleAdminShowPackages();
    if (cmd == CMD_ADMIN_SHOW_BALANCE)           return handleAdminShowBalance();
    if (cmd == CMD_ADMIN_SHOW_COURIERS)          return handleAdminShowCouriers();

    if (cmd == CMD_ADMIN_ADD_COURIER && params.size() >= 3)
        return handleAdminAddCourier(params[0], params[1], params[2]);
    if (cmd == CMD_ADMIN_REMOVE_COURIER && params.size() >= 1)
        return handleAdminRemoveCourier(std::stoi(params[0]));
    if (cmd == CMD_ADMIN_SHOW_COURIER_DETAIL && params.size() >= 1)
        return handleAdminShowCourierDetail(std::stoi(params[0]));
    if (cmd == CMD_ADMIN_ASSIGN_COURIER && params.size() >= 2)
        return handleAdminAssignCourier(params[0], std::stoi(params[1]));
    if (cmd == CMD_ADMIN_QUERY_BY_SENDER && params.size() >= 1)
        return handleAdminQueryBySender(params[0]);
    if (cmd == CMD_ADMIN_QUERY_BY_RECEIVER && params.size() >= 1)
        return handleAdminQueryByReceiver(params[0]);
    if (cmd == CMD_ADMIN_QUERY_BY_ID && params.size() >= 1)
        return handleAdminQueryById(params[0]);
    if (cmd == CMD_ADMIN_QUERY_BY_TIME && params.size() >= 3)
        return handleAdminQueryByTime(std::stoi(params[0]), std::stoi(params[1]),
                                      params[2], params.size() >= 4 ? params[3] : "");

    return buildErrResponse("Invalid command or insufficient permissions");
}

std::string Server::dispatchCourierCommand(const std::string& cmd, const std::vector<std::string>& params,
                                           const std::string& sessionUsername)
{
    int courierId = std::stoi(sessionUsername);

    if (cmd == CMD_COURIER_MY_TASKS)        return handleCourierMyTasks(courierId);
    if (cmd == CMD_COURIER_MY_RECORDS)      return handleCourierMyRecords(courierId);
    if (cmd == CMD_COURIER_BALANCE)         return handleCourierBalance(courierId);

    if (cmd == CMD_COURIER_COLLECT)
        return handleCourierCollect(courierId, params.empty() ? "" : params[0]);
    if (cmd == CMD_COURIER_CHANGE_PWD && params.size() >= 2)
        return handleCourierChangePwd(courierId, params[0], params[1]);

    return buildErrResponse("Invalid command or insufficient permissions");
}
