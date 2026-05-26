#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <windows.h>
#include <string>
#include "DataManager.h"

class Server
{
private:
    SOCKET listenSocket_;
    int port_;
    DataManager dataManager_;
    CRITICAL_SECTION cs_;
    volatile bool running_;
    int nextClientId_;

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void handleClient(SOCKET clientSocket, int clientId);

    // 请求分发（主入口 + 按角色分派）
    std::string processRequest(const std::string& request, int& userType, std::string& sessionUsername);
    std::string dispatchUnauthCommand(const std::string& cmd, const std::vector<std::string>& params,
                                      int& userType, std::string& sessionUsername);
    std::string dispatchUserCommand(const std::string& cmd, const std::vector<std::string>& params,
                                    const std::string& sessionUsername);
    std::string dispatchAdminCommand(const std::string& cmd, const std::vector<std::string>& params);
    std::string dispatchCourierCommand(const std::string& cmd, const std::vector<std::string>& params,
                                       const std::string& sessionUsername);

    bool needsSave(const std::string& cmd) const;

    // 用户相关 handler
    std::string handleLogin(const std::string& type, const std::string& user, const std::string& pwd,
                            int& userType, std::string& sessionUsername);
    std::string handleRegister(const std::string& username, const std::string& name,
                               const std::string& phone, const std::string& pwd, const std::string& addr);
    std::string handleSendPackage(const std::string& sender, const std::string& receiver,
                                  int type, double weightOrCount, const std::string& desc);
    std::string handleReceivePackage(const std::string& username, const std::string& indices);
    std::string handleQuerySent(const std::string& username);
    std::string handleQueryReceived(const std::string& username);
    std::string handleQueryById(const std::string& packageId);
    std::string handleQueryByTime(const std::string& username, const std::string& timeType,
                                  const std::string& dateType, const std::string& dateParam, bool isSent);
    std::string handleBalanceQuery(const std::string& username);
    std::string handleRecharge(const std::string& username, double amount);
    std::string handleChangePwd(const std::string& username, const std::string& oldPwd, const std::string& newPwd);

    // 管理员相关 handler
    std::string handleAdminShowUsers();
    std::string handleAdminShowPackages();
    std::string handleAdminShowBalance();
    std::string handleAdminAddCourier(const std::string& name, const std::string& phone, const std::string& pwd);
    std::string handleAdminRemoveCourier(int id);
    std::string handleAdminShowCouriers();
    std::string handleAdminShowCourierDetail(int id);
    std::string handleAdminAssignCourier(const std::string& packageId, int courierId);
    std::string handleAdminQueryBySender(const std::string& sender);
    std::string handleAdminQueryByReceiver(const std::string& receiver);
    std::string handleAdminQueryById(const std::string& id);
    std::string handleAdminQueryByTime(int timeType, int dateType, const std::string& param1, const std::string& param2);

    // 快递员相关 handler
    std::string handleCourierMyTasks(int courierId);
    std::string handleCourierCollect(int courierId, const std::string& indices);
    std::string handleCourierMyRecords(int courierId);
    std::string handleCourierBalance(int courierId);
    std::string handleCourierChangePwd(int courierId, const std::string& oldPwd, const std::string& newPwd);

    static std::string formatDate(const std::string& timeStr);

public:
    Server(int port);
    ~Server();
    void start();
    void stop();
};

#endif
