#include "../include/LogisticsSystem.h"
#include <iostream>

LogisticsSystem::LogisticsSystem()
    : dataManager_()
    , userCtrl_(dataManager_)
    , adminCtrl_(dataManager_)
    , courierCtrl_(dataManager_)
{
    dataManager_.loadData();
}

void LogisticsSystem::run()
{
    int choice;
    while (true)
    {
        std::cout << "\n========== 物流管理系统 ==========\n";
        std::cout << "1. 用户登录\n";
        std::cout << "2. 管理员登录\n";
        std::cout << "3. 快递员登录\n";
        std::cout << "4. 用户注册\n";
        std::cout << "0. 退出系统\n";
        std::cout << "请选择: ";
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            if (userCtrl_.login())
                userCtrl_.showMenu();
            break;
        case 2:
            if (adminCtrl_.login())
                adminCtrl_.showMenu();
            break;
        case 3:
            if (courierCtrl_.login())
                courierCtrl_.showMenu();
            break;
        case 4:
            userCtrl_.registerUser();
            break;
        case 0:
            dataManager_.saveData();
            std::cout << "感谢使用，再见\n";
            return;
        default:
            std::cout << "无效选择\n";
        }
    }
}