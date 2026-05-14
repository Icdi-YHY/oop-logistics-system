#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <vector>
#include <string>
#include "User.h"
#include "Admin.h"
#include "Courier.h"
#include "Package.h"

class DataManager
{
private:
    std::vector<User> users_;
    std::vector<Courier> couriers_;
    std::vector<Package*> packages_;  // 多态指针
    Admin admin_;
    int nextPackageId_;
    int nextCourierId_;

public:
    DataManager();
    ~DataManager();

    // 加载/保存
    void loadData();
    void saveData() const;

    // 用户相关
    std::vector<User>& getUsers();
    User* findUser(const std::string& username);
    void addUser(const User& user);

    // 快递员相关
    std::vector<Courier>& getCouriers();
    Courier* findCourier(int id);
    Courier* findCourierByName(const std::string& name);
    void addCourier(const Courier& courier);
    void removeCourier(int id);
    int getNextCourierId();

    // 包裹相关
    std::vector<Package*>& getPackages();
    Package* findPackage(const std::string& packageId);
    void addPackage(Package* package);
    std::vector<Package*> findPackagesByCourier(int courierId);
    std::vector<Package*> findPackagesBySender(const std::string& sender);
    std::vector<Package*> findPackagesByReceiver(const std::string& receiver);
    std::vector<Package*> findWaitingCollectPackages();

    // 管理员相关
    Admin& getAdmin();
    int getNextPackageId();
};

#endif