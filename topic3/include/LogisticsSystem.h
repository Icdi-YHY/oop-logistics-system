#ifndef LOGISTICS_SYSTEM_H
#define LOGISTICS_SYSTEM_H

#include "DataManager.h"
#include "UserController.h"
#include "AdminController.h"
#include "CourierController.h"

class LogisticsSystem
{
private:
    DataManager dataManager_;
    UserController userCtrl_;
    AdminController adminCtrl_;
    CourierController courierCtrl_;

public:
    LogisticsSystem();
    void run();  // 改为 run，不是 Run
};

#endif