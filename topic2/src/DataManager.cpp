#include "../include/DataManager.h"
#include "../include/FragilePackage.h"
#include "../include/BookPackage.h"
#include "../include/NormalPackage.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

DataManager::DataManager()
    : admin_("admin", "系统管理员", "admin123", 0.0), nextPackageId_(1), nextCourierId_(1)
{
    loadData();
}

DataManager::~DataManager()
{
    for (auto pkg : packages_)
    {
        delete pkg;
    }
    packages_.clear();
}

// ========== 加载/保存 ==========

void DataManager::loadData()
{
    // 先清空内存中的旧数据
    users_.clear();
    couriers_.clear();
    for (auto pkg : packages_)
        delete pkg;
    packages_.clear();

    // 加载用户数据
    std::ifstream userFile("users.txt");
    if (userFile.is_open())
    {
        std::string line;
        while (std::getline(userFile, line))
        {
            if (line.empty())
                continue;

            std::stringstream ss(line);
            std::string username, name, phonenum, password, address, balanceStr;
            std::getline(ss, username, '|');
            std::getline(ss, name, '|');
            std::getline(ss, phonenum, '|');
            std::getline(ss, password, '|');
            std::getline(ss, balanceStr, '|');
            std::getline(ss, address, '|');

            if (username.empty())
                continue;

            double balance = balanceStr.empty() ? 0.0 : std::stod(balanceStr);
            User user(username, name, phonenum, password, address);
            users_.push_back(user);
        }
        userFile.close();
        std::cout << "加载用户数据成功\n";
    }

    // 加载快递员数据
    std::ifstream courierFile("couriers.txt");
    if (courierFile.is_open())
    {
        std::string line;
        while (std::getline(courierFile, line))
        {
            if (line.empty())
                continue;

            std::stringstream ss(line);
            std::string idStr, name, phone, password, balanceStr;
            std::getline(ss, idStr, '|');
            std::getline(ss, name, '|');
            std::getline(ss, phone, '|');
            std::getline(ss, password, '|');
            std::getline(ss, balanceStr, '|');

            if (idStr.empty())
                continue;

            int id = std::stoi(idStr);
            double balance = balanceStr.empty() ? 0.0 : std::stod(balanceStr);
            Courier courier(id, name, phone, password, balance);
            couriers_.push_back(courier);
            if (id >= nextCourierId_)
                nextCourierId_ = id + 1;
        }
        courierFile.close();
        std::cout << "加载快递员数据成功\n";
    }

    // 加载包裹数据
    std::ifstream packageFile("packages.txt");
    if (packageFile.is_open())
    {
        std::string line;
        while (std::getline(packageFile, line))
        {
            if (line.empty())
                continue;

            std::stringstream ss(line);
            std::string packageId, sender, receiver, sendTime, receiveTime, statusStr, typeStr, detail, description, courierIdStr;
            std::getline(ss, packageId, '|');
            std::getline(ss, sender, '|');
            std::getline(ss, receiver, '|');
            std::getline(ss, sendTime, '|');
            std::getline(ss, receiveTime, '|');
            std::getline(ss, statusStr, '|');
            std::getline(ss, typeStr, '|');
            std::getline(ss, detail, '|');
            std::getline(ss, description, '|');
            std::getline(ss, courierIdStr, '|');

            if (packageId.empty())
                continue;

            int status = statusStr.empty() ? 0 : std::stoi(statusStr);
            int type = typeStr.empty() ? 3 : std::stoi(typeStr);
            int courierId = courierIdStr.empty() ? 0 : std::stoi(courierIdStr);

            Package *pkg = nullptr;
            if (type == 1) // 易碎品
            {
                double weight = detail.empty() ? 0.0 : std::stod(detail);
                pkg = new FragilePackage(packageId, sender, receiver, sendTime, description, weight);
            }
            else if (type == 2) // 图书
            {
                int count = detail.empty() ? 0 : std::stoi(detail);
                pkg = new BookPackage(packageId, sender, receiver, sendTime, description, count);
            }
            else // 普通快递
            {
                double weight = detail.empty() ? 0.0 : std::stod(detail);
                pkg = new NormalPackage(packageId, sender, receiver, sendTime, description, weight);
            }
            pkg->SetStatus(status);
            pkg->SetCourierId(courierId);
            if (!receiveTime.empty())
                pkg->Sign(receiveTime);
            packages_.push_back(pkg);

            int id = std::stoi(packageId);
            if (id >= nextPackageId_)
                nextPackageId_ = id + 1;
        }
        packageFile.close();
        std::cout << "加载包裹数据成功\n";
    }

    // 加载管理员数据
    std::ifstream adminFile("admin.txt");
    if (adminFile.is_open())
    {
        std::string line;
        if (std::getline(adminFile, line) && !line.empty())
        {
            std::stringstream ss(line);
            std::string username, name, password, balanceStr;
            std::getline(ss, username, '|');
            std::getline(ss, name, '|');
            std::getline(ss, password, '|');
            std::getline(ss, balanceStr, '|');

            double balance = balanceStr.empty() ? 0.0 : std::stod(balanceStr);
            admin_ = Admin(username, name, password, balance);
        }
        adminFile.close();
        std::cout << "加载管理员数据成功\n";
    }

    // 加载下一个快递单号
    std::ifstream idFile("next_id.txt");
    if (idFile.is_open())
    {
        std::string line;
        if (std::getline(idFile, line))
        {
            nextPackageId_ = std::stoi(line);
        }
        idFile.close();
        std::cout << "加载单号数据成功\n";
    }

    // 加载下一个快递员ID
    std::ifstream courierIdFile("next_courier_id.txt");
    if (courierIdFile.is_open())
    {
        std::string line;
        if (std::getline(courierIdFile, line))
        {
            nextCourierId_ = std::stoi(line);
        }
        courierIdFile.close();
    }
}

void DataManager::saveData() const
{
    // 保存用户数据
    std::ofstream userFile("users.txt");
    if (userFile.is_open())
    {
        for (const auto &user : users_)
        {
            userFile << user.GetUsername() << "|"
                     << user.GetName() << "|"
                     << user.GetPhonenum() << "|"
                     << user.GetPassword() << "|"
                     << user.GetBalance() << "|"
                     << user.GetAddress() << "\n";
        }
        userFile.close();
    }

    // 保存快递员数据
    std::ofstream courierFile("couriers.txt");
    if (courierFile.is_open())
    {
        for (const auto &courier : couriers_)
        {
            courierFile << courier.GetId() << "|"
                        << courier.GetName() << "|"
                        << courier.GetPhone() << "|"
                        << courier.GetPassword() << "|"
                        << courier.GetBalance() << "\n";
        }
        courierFile.close();
    }

    // 保存包裹数据
    std::ofstream packageFile("packages.txt");
    if (packageFile.is_open())
    {
        for (const auto &pkg : packages_)
        {
            int type = 3;
            std::string detail = "0";

            FragilePackage *fragile = dynamic_cast<FragilePackage *>(pkg);
            if (fragile != nullptr)
            {
                type = 1;
                detail = std::to_string(fragile->GetWeight());
            }

            BookPackage *book = dynamic_cast<BookPackage *>(pkg);
            if (book != nullptr)
            {
                type = 2;
                detail = std::to_string(book->GetCount());
            }

            NormalPackage *normal = dynamic_cast<NormalPackage *>(pkg);
            if (normal != nullptr && type == 3)
            {
                type = 3;
                detail = std::to_string(normal->GetWeight());
            }

            packageFile << pkg->GetId() << "|"
                        << pkg->GetSender() << "|"
                        << pkg->GetReceiver() << "|"
                        << pkg->GetSendTime() << "|"
                        << pkg->GetReceiveTime() << "|"
                        << pkg->GetStatus() << "|"
                        << type << "|"
                        << detail << "|"
                        << pkg->GetDescription() << "|"
                        << pkg->GetCourierId() << "\n";
        }
        packageFile.close();
    }

    // 保存管理员数据
    std::ofstream adminFile("admin.txt");
    if (adminFile.is_open())
    {
        adminFile << admin_.GetUsername() << "|"
                  << admin_.GetName() << "|"
                  << admin_.GetPassword() << "|"
                  << admin_.GetBalance() << "\n";
        adminFile.close();
    }

    // 保存下一个快递单号
    std::ofstream idFile("next_id.txt");
    if (idFile.is_open())
    {
        idFile << nextPackageId_ << "\n";
        idFile.close();
    }

    // 保存下一个快递员ID
    std::ofstream courierIdFile("next_courier_id.txt");
    if (courierIdFile.is_open())
    {
        courierIdFile << nextCourierId_ << "\n";
        courierIdFile.close();
    }

    std::cout << "数据已保存\n";
}

// ========== 用户相关 ==========

std::vector<User> &DataManager::getUsers() { return users_; }

User *DataManager::findUser(const std::string &username)
{
    for (auto &user : users_)
    {
        if (user.GetUsername() == username)
            return &user;
    }
    return nullptr;
}

void DataManager::addUser(const User &user)
{
    users_.push_back(user);
}

// ========== 快递员相关 ==========

std::vector<Courier> &DataManager::getCouriers() { return couriers_; }

Courier *DataManager::findCourier(int id)
{
    for (auto &courier : couriers_)
    {
        if (courier.GetId() == id)
            return &courier;
    }
    return nullptr;
}

Courier *DataManager::findCourierByName(const std::string &name)
{
    for (auto &courier : couriers_)
    {
        if (courier.GetName() == name)
            return &courier;
    }
    return nullptr;
}

void DataManager::addCourier(const Courier &courier)
{
    couriers_.push_back(courier);
}

void DataManager::removeCourier(int id)
{
    auto it = std::remove_if(couriers_.begin(), couriers_.end(),
                             [id](const Courier &c)
                             { return c.GetId() == id; });
    if (it != couriers_.end())
        couriers_.erase(it, couriers_.end());
}

int DataManager::getNextCourierId()
{
    return nextCourierId_++;
}

// ========== 包裹相关 ==========

std::vector<Package *> &DataManager::getPackages() { return packages_; }

Package *DataManager::findPackage(const std::string &packageId)
{
    for (auto pkg : packages_)
    {
        if (pkg->GetId() == packageId)
            return pkg;
    }
    return nullptr;
}

void DataManager::addPackage(Package *package)
{
    packages_.push_back(package);
}

std::vector<Package *> DataManager::findPackagesByCourier(int courierId)
{
    std::vector<Package *> result;
    for (auto pkg : packages_)
    {
        if (pkg->GetCourierId() == courierId)
            result.push_back(pkg);
    }
    return result;
}

std::vector<Package *> DataManager::findPackagesBySender(const std::string &sender)
{
    std::vector<Package *> result;
    for (auto pkg : packages_)
    {
        if (pkg->GetSender() == sender)
            result.push_back(pkg);
    }
    return result;
}

std::vector<Package *> DataManager::findPackagesByReceiver(const std::string &receiver)
{
    std::vector<Package *> result;
    for (auto pkg : packages_)
    {
        if (pkg->GetReceiver() == receiver)
            result.push_back(pkg);
    }
    return result;
}

std::vector<Package *> DataManager::findWaitingCollectPackages()
{
    std::vector<Package *> result;
    for (auto pkg : packages_)
    {
        if (pkg->IsWaitingCollect())
            result.push_back(pkg);
    }
    return result;
}

// ========== 管理员相关 ==========

Admin &DataManager::getAdmin() { return admin_; }

int DataManager::getNextPackageId()
{
    return nextPackageId_++;
}