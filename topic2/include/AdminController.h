#ifndef ADMIN_CONTROLLER_H
#define ADMIN_CONTROLLER_H

#include "DataManager.h"

class AdminController
{
private:
    DataManager& dataManager_;
    bool isLoggedIn_;

public:
    AdminController(DataManager& dm);

    bool login();
    void logout();
    void showAllUsers();
    void showAllPackages();
    void showCompanyBalance();
    void addCourier();
    void removeCourier();
    void showAllCouriers();
    void showCourierDetail();
    void assignCourierToPackage();
    void showMenu();  // 添加这一行

    bool isLoggedIn() const;
};

#endif