#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>
#include <string>

class Client
{
private:
    SOCKET sock_;
    bool connected_;
    std::string host_;
    int port_;

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    bool sendRequest(const std::string& request);
    std::string receiveResponse();
    void printResponse(const std::string& response) const;

    // 主菜单
    void showMainMenu() const;
    void handleUserLogin();
    void handleAdminLogin();
    void handleCourierLogin();
    void handleUserRegister();

    // 用户菜单
    void userMenu();
    void userSendPackage();
    void userReceivePackage();
    void userQueryPackages();
    void userBalance();
    void userRecharge();
    void userChangePwd();

    // 管理员菜单
    void adminMenu();
    void adminShowUsers();
    void adminShowPackages();
    void adminShowBalance();
    void adminAddCourier();
    void adminRemoveCourier();
    void adminShowCouriers();
    void adminShowCourierDetail();
    void adminAssignCourier();

    // 快递员菜单
    void courierMenu();
    void courierMyTasks();
    void courierCollect();
    void courierMyRecords();
    void courierBalance();
    void courierChangePwd();

public:
    Client();
    ~Client();
    bool connect(const std::string& host, int port);
    bool reconnect(int maxRetries = 5, int delaySec = 2);
    void disconnect();
    void run();
};

#endif
