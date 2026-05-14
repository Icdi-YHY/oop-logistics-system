#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include "DataManager.h"

class UserController
{
private:
    DataManager& dataManager_;
    User* currentUser_;

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
    void showMenu();  // 添加这一行

    User* getCurrentUser() const;
    bool isLoggedIn() const;
};

#endif