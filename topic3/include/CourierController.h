#ifndef COURIER_CONTROLLER_H
#define COURIER_CONTROLLER_H

#include "DataManager.h"

class CourierController
{
private:
    DataManager& dataManager_;
    Courier* currentCourier_;

public:
    CourierController(DataManager& dm);

    bool login();
    void logout();
    void showMyTasks();
    void collectPackage();
    void queryMyRecords();
    void changePassword();
    void showBalance();
    void showMenu();  // 添加这一行

    Courier* getCurrentCourier() const;
    bool isLoggedIn() const;
};

#endif