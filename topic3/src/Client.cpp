#include "../include/Client.h"
#include "../include/Protocol.h"
#include <iostream>
#include <sstream>
#include <vector>

using namespace Protocol;

Client::Client()
    : sock_(INVALID_SOCKET)
    , connected_(false)
{
}

Client::~Client()
{
    disconnect();
}

bool Client::connect(const std::string& host, int port)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 初始化失败" << std::endl;
        return false;
    }

    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ == INVALID_SOCKET) {
        std::cerr << "创建Socket失败" << std::endl;
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(host.c_str());

    if (::connect(sock_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "连接服务器 " << host << ":" << port << " 失败" << std::endl;
        closesocket(sock_);
        WSACleanup();
        return false;
    }

    connected_ = true;
    return true;
}

void Client::disconnect()
{
    if (connected_) {
        closesocket(sock_);
        WSACleanup();
        connected_ = false;
    }
}

bool Client::sendRequest(const std::string& request)
{
    if (!connected_) return false;
    return send(sock_, request.c_str(), request.length(), 0) != SOCKET_ERROR;
}

std::string Client::receiveResponse()
{
    char buffer[8192];
    memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(sock_, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        return ERR_PREFIX + "|连接断开";
    }
    return std::string(buffer);
}

void Client::showMainMenu() const
{
    std::cout << "\n========== 物流管理系统 ==========" << std::endl;
    std::cout << "1. 用户登录" << std::endl;
    std::cout << "2. 管理员登录" << std::endl;
    std::cout << "3. 快递员登录" << std::endl;
    std::cout << "4. 用户注册" << std::endl;
    std::cout << "0. 退出系统" << std::endl;
    std::cout << "请选择: ";
}

void Client::handleUserLogin()
{
    std::string username, password;
    std::cout << "用户名: ";
    std::cin >> username;
    std::cout << "密码: ";
    std::cin >> password;

    sendRequest(buildRequest(CMD_LOGIN, {LOGIN_TYPE_USER, username, password}));
    std::string response = receiveResponse();

    if (response.find(OK_PREFIX) == 0) {
        std::cout << "登录成功!" << std::endl;
        userMenu();
    } else {
        std::cout << "登录失败: " << response << std::endl;
    }
}

void Client::handleAdminLogin()
{
    std::string username, password;
    std::cout << "用户名: ";
    std::cin >> username;
    std::cout << "密码: ";
    std::cin >> password;

    sendRequest(buildRequest(CMD_LOGIN, {LOGIN_TYPE_ADMIN, username, password}));
    std::string response = receiveResponse();

    if (response.find(OK_PREFIX) == 0) {
        std::cout << "管理员登录成功!" << std::endl;
        adminMenu();
    } else {
        std::cout << "登录失败: " << response << std::endl;
    }
}

void Client::handleCourierLogin()
{
    std::string id, password;
    std::cout << "快递员ID: ";
    std::cin >> id;
    std::cout << "密码: ";
    std::cin >> password;

    sendRequest(buildRequest(CMD_LOGIN, {LOGIN_TYPE_COURIER, id, password}));
    std::string response = receiveResponse();

    if (response.find(OK_PREFIX) == 0) {
        std::cout << "快递员登录成功!" << std::endl;
        courierMenu();
    } else {
        std::cout << "登录失败: " << response << std::endl;
    }
}

void Client::handleUserRegister()
{
    std::string username, name, phone, password, address;
    std::cout << "用户名: ";
    std::cin >> username;
    std::cout << "姓名: ";
    std::cin >> name;
    std::cout << "密码: ";
    std::cin >> password;
    std::cout << "电话: ";
    std::cin >> phone;
    std::cout << "地址: ";
    std::cin >> address;

    sendRequest(buildRequest(CMD_REGISTER, {username, name, phone, password, address}));
    std::cout << receiveResponse() << std::endl;
}

void Client::run()
{
    if (!connected_) {
        std::cout << "未连接到服务器" << std::endl;
        return;
    }

    std::cout << "已连接到服务器!" << std::endl;

    int choice;
    while (true) {
        showMainMenu();
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "输入无效" << std::endl;
            continue;
        }

        switch (choice) {
        case 1: handleUserLogin(); break;
        case 2: handleAdminLogin(); break;
        case 3: handleCourierLogin(); break;
        case 4: handleUserRegister(); break;
        case 0:
            sendRequest(buildRequest(CMD_EXIT));
            disconnect();
            std::cout << "感谢使用，再见!" << std::endl;
            return;
        default:
            std::cout << "无效选择" << std::endl;
        }
    }
}

void Client::printResponse(const std::string& response) const
{
    if (response.find(ERR_PREFIX) == 0) {
        std::cout << "错误: " << response.substr(ERR_PREFIX.length() + 1) << std::endl;
        return;
    }

    std::istringstream iss(response);
    std::string line;
    bool firstLine = true;
    while (std::getline(iss, line)) {
        if (line == END_MARKER) break;
        if (firstLine) {
            size_t pos = line.find('|');
            if (pos != std::string::npos) {
                std::cout << line.substr(pos + 1) << std::endl;
            } else {
                std::cout << line << std::endl;
            }
            firstLine = false;
        } else {
            std::cout << "  " << line << std::endl;
        }
    }
}

// ==================== 用户菜单 ====================

void Client::userMenu()
{
    int choice;
    while (true) {
        std::cout << "\n========== 用户菜单 ==========" << std::endl;
        std::cout << "1. 查询余额" << std::endl;
        std::cout << "2. 充值" << std::endl;
        std::cout << "3. 发送快递" << std::endl;
        std::cout << "4. 接收快递" << std::endl;
        std::cout << "5. 查询快递" << std::endl;
        std::cout << "6. 修改密码" << std::endl;
        std::cout << "0. 退出登录" << std::endl;
        std::cout << "请选择: ";
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "输入无效" << std::endl;
            continue;
        }

        switch (choice) {
        case 1: userBalance(); break;
        case 2: userRecharge(); break;
        case 3: userSendPackage(); break;
        case 4: userReceivePackage(); break;
        case 5: userQueryPackages(); break;
        case 6: userChangePwd(); break;
        case 0:
            sendRequest(buildRequest(CMD_LOGOUT));
            receiveResponse();
            return;
        default:
            std::cout << "无效选择" << std::endl;
        }
    }
}

void Client::userSendPackage()
{
    std::string receiver, desc;
    int type;
    double weight;
    int count;

    std::cout << "收件人用户名: ";
    std::cin >> receiver;

    std::cout << "快递类型 (1=易碎品8元/kg, 2=图书2元/本, 3=普通5元/kg): ";
    if (!(std::cin >> type)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "输入无效" << std::endl;
        return;
    }

    std::string detail;
    if (type == 2) {
        std::cout << "图书数量(本): ";
        std::cin >> count;
        detail = std::to_string(count);
    } else {
        std::cout << "重量(kg): ";
        std::cin >> weight;
        detail = std::to_string(weight);
    }

    std::cout << "物品描述: ";
    std::cin >> desc;

    sendRequest(buildRequest(CMD_SEND_PACKAGE, {receiver, std::to_string(type), detail, desc}));
    printResponse(receiveResponse());
}

void Client::userReceivePackage()
{
    sendRequest(buildRequest(CMD_RECEIVE_PACKAGE, {}));
    std::string response = receiveResponse();

    if (response.find(ERR_PREFIX) == 0) {
        std::cout << "错误: " << response.substr(ERR_PREFIX.length() + 1) << std::endl;
        return;
    }

    printResponse(response);

    std::cout << "请输入要签收的快递编号(逗号分隔, 0取消): ";
    std::string indices;
    std::cin >> indices;

    if (indices == "0" || indices.empty()) return;

    sendRequest(buildRequest(CMD_RECEIVE_PACKAGE, {indices}));
    printResponse(receiveResponse());
}

void Client::userQueryPackages()
{
    int choice;
    std::cout << "\n1. 我发出的快递" << std::endl;
    std::cout << "2. 我接收的快递" << std::endl;
    std::cout << "3. 按快递单号查询" << std::endl;
    std::cout << "4. 按寄件时间查询" << std::endl;
    std::cout << "5. 按签收时间查询" << std::endl;
    std::cout << "请选择: ";
    if (!(std::cin >> choice)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        return;
    }

    std::string response;
    switch (choice) {
    case 1:
        sendRequest(buildRequest(CMD_QUERY_SENT));
        break;
    case 2:
        sendRequest(buildRequest(CMD_QUERY_RECEIVED));
        break;
    case 3: {
        std::string id;
        std::cout << "快递单号: ";
        std::cin >> id;
        sendRequest(buildRequest(CMD_QUERY_BY_ID, {id}));
        break;
    }
    case 4:
    case 5: {
        int dateType;
        std::cout << "时间类型 (1=具体日期, 2=按年月, 3=时间段): ";
        std::cin >> dateType;

        std::string param;
        if (dateType == 1) {
            std::cout << "日期 (格式: YYYY-MM-DD): ";
            std::cin >> param;
        } else if (dateType == 2) {
            std::cout << "年月 (格式: YYYY-MM): ";
            std::cin >> param;
        } else if (dateType == 3) {
            std::string start, end;
            std::cout << "开始日期 (YYYY-MM-DD): ";
            std::cin >> start;
            std::cout << "结束日期 (YYYY-MM-DD): ";
            std::cin >> end;
            param = start + "," + end;
        }
        std::string cmd = (choice == 4) ? CMD_QUERY_BY_TIME_SENT : CMD_QUERY_BY_TIME_RECEIVED;
        sendRequest(buildRequest(cmd, {std::to_string(dateType), param}));
        break;
    }
    default:
        return;
    }
    printResponse(receiveResponse());
}

void Client::userBalance()
{
    sendRequest(buildRequest(CMD_GET_BALANCE));
    printResponse(receiveResponse());
}

void Client::userRecharge()
{
    double amount;
    std::cout << "充值金额: ";
    std::cin >> amount;
    sendRequest(buildRequest(CMD_RECHARGE, {std::to_string(amount)}));
    printResponse(receiveResponse());
}

void Client::userChangePwd()
{
    std::string oldPwd, newPwd;
    std::cout << "原密码: ";
    std::cin >> oldPwd;
    std::cout << "新密码: ";
    std::cin >> newPwd;
    sendRequest(buildRequest(CMD_CHANGE_PWD, {oldPwd, newPwd}));
    printResponse(receiveResponse());
}

// ==================== 管理员菜单 ====================

void Client::adminMenu()
{
    int choice;
    while (true) {
        std::cout << "\n========== 管理员菜单 ==========" << std::endl;
        std::cout << "1. 查看所有用户" << std::endl;
        std::cout << "2. 查看所有快递" << std::endl;
        std::cout << "3. 查看公司余额" << std::endl;
        std::cout << "4. 添加快递员" << std::endl;
        std::cout << "5. 删除快递员" << std::endl;
        std::cout << "6. 查看所有快递员" << std::endl;
        std::cout << "7. 查看快递员详情" << std::endl;
        std::cout << "8. 分配揽收任务" << std::endl;
        std::cout << "0. 退出登录" << std::endl;
        std::cout << "请选择: ";
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "输入无效" << std::endl;
            continue;
        }

        switch (choice) {
        case 1: adminShowUsers(); break;
        case 2: adminShowPackages(); break;
        case 3: adminShowBalance(); break;
        case 4: adminAddCourier(); break;
        case 5: adminRemoveCourier(); break;
        case 6: adminShowCouriers(); break;
        case 7: adminShowCourierDetail(); break;
        case 8: adminAssignCourier(); break;
        case 0:
            sendRequest(buildRequest(CMD_LOGOUT));
            receiveResponse();
            return;
        default:
            std::cout << "无效选择" << std::endl;
        }
    }
}

void Client::adminShowUsers()
{
    sendRequest(buildRequest(CMD_ADMIN_SHOW_USERS));
    printResponse(receiveResponse());
}

void Client::adminShowPackages()
{
    int choice;
    std::cout << "\n1. 查看所有快递" << std::endl;
    std::cout << "2. 按寄件人查询" << std::endl;
    std::cout << "3. 按收件人查询" << std::endl;
    std::cout << "4. 按快递单号查询" << std::endl;
    std::cout << "5. 按时间查询" << std::endl;
    std::cout << "请选择: ";
    if (!(std::cin >> choice)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        return;
    }

    switch (choice) {
    case 1:
        sendRequest(buildRequest(CMD_ADMIN_SHOW_PACKAGES));
        break;
    case 2: {
        std::string sender;
        std::cout << "寄件人用户名: ";
        std::cin >> sender;
        sendRequest(buildRequest(CMD_ADMIN_QUERY_BY_SENDER, {sender}));
        break;
    }
    case 3: {
        std::string receiver;
        std::cout << "收件人用户名: ";
        std::cin >> receiver;
        sendRequest(buildRequest(CMD_ADMIN_QUERY_BY_RECEIVER, {receiver}));
        break;
    }
    case 4: {
        std::string id;
        std::cout << "快递单号: ";
        std::cin >> id;
        sendRequest(buildRequest(CMD_ADMIN_QUERY_BY_ID, {id}));
        break;
    }
    case 5: {
        int timeType, dateType;
        std::cout << "1. 按寄件时间 2. 按签收时间: ";
        std::cin >> timeType;
        std::cout << "时间类型 (1=具体日期 2=按年月 3=时间段): ";
        std::cin >> dateType;

        std::string param1, param2;
        if (dateType == 1) {
            std::cout << "日期 (YYYY-MM-DD): ";
            std::cin >> param1;
        } else if (dateType == 2) {
            std::cout << "年月 (YYYY-MM): ";
            std::cin >> param1;
        } else if (dateType == 3) {
            std::cout << "开始日期 (YYYY-MM-DD): ";
            std::cin >> param1;
            std::cout << "结束日期 (YYYY-MM-DD): ";
            std::cin >> param2;
        }
        sendRequest(buildRequest(CMD_ADMIN_QUERY_BY_TIME,
                    {std::to_string(timeType), std::to_string(dateType), param1, param2}));
        break;
    }
    default:
        return;
    }
    printResponse(receiveResponse());
}

void Client::adminShowBalance()
{
    sendRequest(buildRequest(CMD_ADMIN_SHOW_BALANCE));
    printResponse(receiveResponse());
}

void Client::adminAddCourier()
{
    std::string name, phone, password;
    std::cout << "姓名: ";
    std::cin >> name;
    std::cout << "电话: ";
    std::cin >> phone;
    std::cout << "密码: ";
    std::cin >> password;
    sendRequest(buildRequest(CMD_ADMIN_ADD_COURIER, {name, phone, password}));
    printResponse(receiveResponse());
}

void Client::adminRemoveCourier()
{
    int id;
    std::cout << "快递员ID: ";
    std::cin >> id;
    sendRequest(buildRequest(CMD_ADMIN_REMOVE_COURIER, {std::to_string(id)}));
    printResponse(receiveResponse());
}

void Client::adminShowCouriers()
{
    sendRequest(buildRequest(CMD_ADMIN_SHOW_COURIERS));
    printResponse(receiveResponse());
}

void Client::adminShowCourierDetail()
{
    int id;
    std::cout << "快递员ID: ";
    std::cin >> id;
    sendRequest(buildRequest(CMD_ADMIN_SHOW_COURIER_DETAIL, {std::to_string(id)}));
    printResponse(receiveResponse());
}

void Client::adminAssignCourier()
{
    sendRequest(buildRequest(CMD_ADMIN_SHOW_PACKAGES));
    std::string response = receiveResponse();
    printResponse(response);

    std::string packageId;
    int courierId;
    std::cout << "要分配的快递单号: ";
    std::cin >> packageId;

    sendRequest(buildRequest(CMD_ADMIN_SHOW_COURIERS));
    response = receiveResponse();
    printResponse(response);

    std::cout << "快递员ID: ";
    std::cin >> courierId;

    sendRequest(buildRequest(CMD_ADMIN_ASSIGN_COURIER, {packageId, std::to_string(courierId)}));
    printResponse(receiveResponse());
}

// ==================== 快递员菜单 ====================

void Client::courierMenu()
{
    int choice;
    while (true) {
        std::cout << "\n========== 快递员菜单 ==========" << std::endl;
        std::cout << "1. 查看我的任务" << std::endl;
        std::cout << "2. 揽收快递" << std::endl;
        std::cout << "3. 我的揽收记录" << std::endl;
        std::cout << "4. 查询余额" << std::endl;
        std::cout << "5. 修改密码" << std::endl;
        std::cout << "0. 退出登录" << std::endl;
        std::cout << "请选择: ";
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "输入无效" << std::endl;
            continue;
        }

        switch (choice) {
        case 1: courierMyTasks(); break;
        case 2: courierCollect(); break;
        case 3: courierMyRecords(); break;
        case 4: courierBalance(); break;
        case 5: courierChangePwd(); break;
        case 0:
            sendRequest(buildRequest(CMD_LOGOUT));
            receiveResponse();
            return;
        default:
            std::cout << "无效选择" << std::endl;
        }
    }
}

void Client::courierMyTasks()
{
    sendRequest(buildRequest(CMD_COURIER_MY_TASKS));
    printResponse(receiveResponse());
}

void Client::courierCollect()
{
    sendRequest(buildRequest(CMD_COURIER_COLLECT, {}));
    std::string response = receiveResponse();
    printResponse(response);

    std::cout << "请输入要揽收的编号(逗号分隔, 0取消): ";
    std::string indices;
    std::cin >> indices;

    if (indices == "0" || indices.empty()) return;

    sendRequest(buildRequest(CMD_COURIER_COLLECT, {indices}));
    printResponse(receiveResponse());
}

void Client::courierMyRecords()
{
    sendRequest(buildRequest(CMD_COURIER_MY_RECORDS));
    printResponse(receiveResponse());
}

void Client::courierBalance()
{
    sendRequest(buildRequest(CMD_COURIER_BALANCE));
    printResponse(receiveResponse());
}

void Client::courierChangePwd()
{
    std::string oldPwd, newPwd;
    std::cout << "原密码: ";
    std::cin >> oldPwd;
    std::cout << "新密码: ";
    std::cin >> newPwd;
    sendRequest(buildRequest(CMD_COURIER_CHANGE_PWD, {oldPwd, newPwd}));
    printResponse(receiveResponse());
}
