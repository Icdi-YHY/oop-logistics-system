#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include "DataManager.h"

class UserController
{
private:
    DataManager& dataManager_;
    User* currentUser_;

    // 日期格式化辅助函数
    static std::string formatDate(const std::string& timeStr);
    // 查询子功能
    void querySent() const;
    void queryReceived() const;
    void queryById() const;
    void queryByTimeMenu();
    void queryByTimeFilter(bool isSent);

public:
    UserController(DataManager& dm);

    bool login();
    void logout();
    void registerUser();
    void changePassword();
    void recharge();
    void showBalance();
    void sendPackage();
    void receivePackage();
    void queryPackages();
    void showMenu();

    User* getCurrentUser() const;
    bool isLoggedIn() const;
};

#endif