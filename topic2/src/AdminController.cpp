#include "../include/AdminController.h"
#include <iostream>
#include <algorithm>

AdminController::AdminController(DataManager &dm)
    : dataManager_(dm), isLoggedIn_(false)
{
}

bool AdminController::login()
{
    std::string username, password;
    std::cout << "\n========== 管理员登录 ==========\n";
    std::cout << "用户名: ";
    std::cin >> username;
    std::cout << "密码: ";
    std::cin >> password;

    Admin &admin = dataManager_.getAdmin();
    if (username == admin.GetUsername() && admin.CheckPassword(password))
    {
        isLoggedIn_ = true;
        std::cout << "登录成功\n";
        return true;
    }
    else
    {
        std::cout << "用户名或密码错误\n";
        return false;
    }
}

void AdminController::logout()
{
    isLoggedIn_ = false;
    std::cout << "已退出管理员登录\n";
}

void AdminController::showAllUsers()
{
    std::cout << "\n========== 所有用户 ==========\n";
    for (const auto &user : dataManager_.getUsers())
    {
        std::cout << "----------------------------\n";
        std::cout << "用户名: " << user.GetUsername() << "\n";
        std::cout << "姓名: " << user.GetName() << "\n";
        std::cout << "电话: " << user.GetPhonenum() << "\n";
        std::cout << "地址: " << user.GetAddress() << "\n";
        std::cout << "余额: " << user.GetBalance() << " 元\n";
    }
    std::cout << "----------------------------\n";
}

void AdminController::showAllPackages()
{
    std::cout << "\n========== 所有快递 ==========\n";
    for (const auto &pkg : dataManager_.getPackages())
    {
        std::cout << "----------------------------\n";
        std::cout << "快递单号: " << pkg->GetId() << "\n";
        std::cout << "寄件人: " << pkg->GetSender() << "\n";
        std::cout << "收件人: " << pkg->GetReceiver() << "\n";
        std::cout << "寄件时间: " << pkg->GetSendTime() << "\n";
        if (pkg->GetStatus() == 2)
        {
            std::cout << "签收时间: " << pkg->GetReceiveTime() << "\n";
            std::cout << "状态: 已签收\n";
        }
        else if (pkg->GetStatus() == 1)
        {
            std::cout << "状态: 待签收\n";
        }
        else
        {
            std::cout << "状态: 待揽收\n";
        }
        std::cout << "物品描述: " << pkg->GetDescription() << "\n";
    }
    std::cout << "----------------------------\n";
}

void AdminController::showCompanyBalance()
{
    std::cout << "公司余额: " << dataManager_.getAdmin().GetBalance() << " 元\n";
}

void AdminController::addCourier()
{
    std::string name, phone, password;
    std::cout << "姓名: ";
    std::cin >> name;
    std::cout << "电话: ";
    std::cin >> phone;
    std::cout << "密码: ";
    std::cin >> password;

    int id = dataManager_.getNextCourierId();
    Courier newCourier(id, name, phone, password, 0.0);
    dataManager_.addCourier(newCourier);
    dataManager_.saveData();
    std::cout << "添加快递员成功！ID: " << id << "\n";
}

void AdminController::removeCourier()
{
    int id;
    showAllCouriers();
    std::cout << "请输入要删除的快递员ID: ";
    if (!(std::cin >> id))
    {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "输入非法";
        return;
    }

    dataManager_.removeCourier(id);
    dataManager_.saveData();
    std::cout << "删除成功\n";
}

void AdminController::showAllCouriers()
{
    std::cout << "\n========== 所有快递员 ==========\n";
    for (const auto &courier : dataManager_.getCouriers())
    {

        std::cout << "----------------------------\n";
        std::cout << "用户名: " << courier.GetId() << "\n";
        std::cout << "姓名: " << courier.GetName() << "\n";
        std::cout << "电话: " << courier.GetPhone() << "\n";
        std::cout << "余额: " << courier.GetBalance() << " 元\n";
    }
    std::cout << "----------------------------\n";
}

void AdminController::showCourierDetail()
{
    int id;
    std::cout << "请输入快递员ID: ";
    if (!(std::cin >> id))
    {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "输入非法";
        return;
    }

    Courier *courier = dataManager_.findCourier(id);
    if (courier == nullptr)
    {
        std::cout << "快递员不存在\n";
        return;
    }

    std::cout << "ID: " << courier->GetId() << "\n";
    std::cout << "姓名: " << courier->GetName() << "\n";
    std::cout << "电话: " << courier->GetPhone() << "\n";
    std::cout << "余额: " << courier->GetBalance() << "\n";

    std::cout << "\n揽收记录:\n";
    auto packages = dataManager_.findPackagesByCourier(id);
    for (const auto &pkg : packages)
    {
        std::cout << "  单号: " << pkg->GetId() << " | 寄件人: " << pkg->GetSender()
                  << " | 收件人: " << pkg->GetReceiver()
                  << " | 状态: " << (pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "待签收" : "待揽收")) << "\n";
    }
}

void AdminController::assignCourierToPackage()
{
    auto waitingPackages = dataManager_.findWaitingCollectPackages();
    if (waitingPackages.empty())
    {
        std::cout << "没有待揽收的快递\n";
        return;
    }

    std::cout << "\n待揽收快递:\n";
    for (size_t i = 0; i < waitingPackages.size(); i++)
    {
        std::cout << i << ". 单号: " << waitingPackages[i]->GetId()
                  << " | 寄件人: " << waitingPackages[i]->GetSender()
                  << " | 收件人: " << waitingPackages[i]->GetReceiver() << "\n";
    }

    int pkgIndex;
    std::cout << "请选择快递编号: ";
    if (!(std::cin >> pkgIndex))
    {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "输入非法";
        return;
    }

    if (pkgIndex < 0 || pkgIndex >= (int)waitingPackages.size())
    {
        std::cout << "无效选择\n";
        return;
    }

    showAllCouriers();
    int courierId;
    std::cout << "请选择快递员ID: ";
    if (!(std::cin >> courierId))
    {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "输入非法";
        return;
    }

    Courier *courier = dataManager_.findCourier(courierId);
    if (courier == nullptr)
    {
        std::cout << "快递员不存在\n";
        return;
    }

    waitingPackages[pkgIndex]->SetCourierId(courierId);
    dataManager_.saveData();
    std::cout << "分配成功！快递员 " << courier->GetName() << " 将揽收此快递\n";
}

void AdminController::showMenu()
{
    if (!isLoggedIn_)
        return;

    int choice;
    while (true)
    {
        std::cout << "\n========== 管理员菜单 ==========\n";
        std::cout << "1. 查看所有用户\n";
        std::cout << "2. 查看所有快递\n";
        std::cout << "3. 查看公司余额\n";
        std::cout << "4. 添加快递员\n";
        std::cout << "5. 删除快递员\n";
        std::cout << "6. 查看所有快递员\n";
        std::cout << "7. 查看快递员详情\n";
        std::cout << "8. 分配揽收任务\n";
        std::cout << "0. 退出登录\n";
        std::cout << "请选择: ";
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "输入非法";
            continue;
        }
        switch (choice)
        {
        case 1:
            showAllUsers();
            break;
        case 2:
            showAllPackages();
            break;
        case 3:
            showCompanyBalance();
            break;
        case 4:
            addCourier();
            break;
        case 5:
            removeCourier();
            break;
        case 6:
            showAllCouriers();
            break;
        case 7:
            showCourierDetail();
            break;
        case 8:
            assignCourierToPackage();
            break;
        case 0:
            logout();
            return;
        default:
            std::cout << "无效选择\n";
        }
    }
}

bool AdminController::isLoggedIn() const { return isLoggedIn_; }