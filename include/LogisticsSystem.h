#ifndef LOGISTICS_SYSTEM_H
#define LOGISTICS_SYSTEM_H

#include <vector>
#include <string>
#include "User.h"
#include "Admin.h"
#include "Package.h"

class LogisticsSystem
{
private:
    std::vector<User> users_;
    std::vector<Package> packages_;
    Admin admin_;
    User *currentUser_;
    int nextPackageId_;

    // 辅助方法
    User *findUser(const std::string &username);
    std::string generatePackageId();

public:
    // 构造函数
    LogisticsSystem();

    // 主入口
    void Run();

    // 数据持久化
    void loadData();
    void saveData() const;

    // 用户菜单
    void userMenu();

    // 用户功能
    void userRegister();
    bool userLogin();
    void adminLogin();
    void sendPackage();
    void receivePackage();
    void queryPackages() const;

    // 管理员功能
    void viewAllUsers() const;
    void viewAllPackages() const;
};
#endif