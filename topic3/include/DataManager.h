#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <vector>
#include <string>
#include "User.h"
#include "Admin.h"
#include "Courier.h"
#include "Package.h"

// 数据管理器：负责所有数据的持久化存储和内存管理
class DataManager
{
private:
    std::vector<User> users_;         // 所有注册用户
    std::vector<Courier> couriers_;   // 所有快递员
    std::vector<Package*> packages_;  // 所有快递包裹（使用多态指针）
    Admin admin_;                     // 管理员
    int nextPackageId_;               // 下一个快递单号
    int nextCourierId_;               // 下一个快递员ID

public:
    DataManager();
    ~DataManager();

    // 加载和保存数据到文件
    void loadData();
    void saveData();

private:
    // 分模块加载数据
    void loadUsers();
    void loadCouriers();
    void loadPackages();
    void loadAdmin();
    void loadNextIds();

public:
    // 用户管理
    std::vector<User>& getUsers();
    User* findUser(const std::string& username);
    void addUser(const User& user);

    // 快递员管理
    std::vector<Courier>& getCouriers();
    Courier* findCourier(int id);
    Courier* findCourierByName(const std::string& name);
    void addCourier(const Courier& courier);
    void removeCourier(int id);
    int getNextCourierId();

    // 包裹管理
    std::vector<Package*>& getPackages();
    Package* findPackage(const std::string& packageId);
    void addPackage(Package* package);
    std::vector<Package*> findPackagesByCourier(int courierId);
    std::vector<Package*> findPackagesBySender(const std::string& sender);
    std::vector<Package*> findPackagesByReceiver(const std::string& receiver);
    std::vector<Package*> findWaitingCollectPackages();

    // 管理员
    Admin& getAdmin();
    int getNextPackageId();
};

#endif
