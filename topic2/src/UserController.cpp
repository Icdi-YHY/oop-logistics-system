#include "../include/UserController.h"
#include "../include/FragilePackage.h"
#include "../include/BookPackage.h"
#include "../include/NormalPackage.h"
#include <iostream>
#include <ctime>
UserController::UserController(DataManager& dm)
    : dataManager_(dm), currentUser_(nullptr)
{
}

bool UserController::login()
{
    std::string username, password;
    std::cout << "\n========== 用户登录 ==========\n";
    std::cout << "用户名: ";
    std::cin >> username;
    std::cout << "密码: ";
    std::cin >> password;

    User* user = dataManager_.findUser(username);
    if (user == nullptr)
    {
        std::cout << "用户名不存在\n";
        return false;
    }
    if (!user->CheckPassword(password))
    {
        std::cout << "密码错误\n";
        return false;
    }
    currentUser_ = user;
    std::cout << "登录成功！欢迎 " << user->GetUsername() << "\n";
    return true;
}

void UserController::logout()
{
    currentUser_ = nullptr;
    std::cout << "已退出登录\n";
}

void UserController::registerUser()
{
    std::string username, name, phonenum, password, address;
    std::cout << "\n========== 用户注册 ==========\n";
    std::cout << "用户名: ";
    std::cin >> username;
    
    if (dataManager_.findUser(username) != nullptr)
    {
        std::cout << "用户名已存在\n";
        return;
    }
    
    std::cout << "姓名: ";
    std::cin >> name;
    std::cout << "密码: ";
    std::cin >> password;
    std::cout << "电话: ";
    std::cin >> phonenum;
    std::cout << "地址: ";
    std::cin >> address;

    User newUser(username, name, phonenum, password, address);
    dataManager_.addUser(newUser);
    dataManager_.saveData();
    std::cout << "注册成功！\n";
}

void UserController::changePassword()
{
    if (currentUser_ == nullptr) return;
    
    std::string oldPwd, newPwd;
    std::cout << "原密码: ";
    std::cin >> oldPwd;
    
    if (!currentUser_->CheckPassword(oldPwd))
    {
        std::cout << "原密码错误\n";
        return;
    }
    
    std::cout << "新密码: ";
    std::cin >> newPwd;
    currentUser_->SetPassword(newPwd);
    dataManager_.saveData();
    std::cout << "密码修改成功\n";
}

void UserController::recharge()
{
    if (currentUser_ == nullptr) return;
    
    double amount;
    std::cout << "充值金额: ";
    std::cin >> amount;
    currentUser_->Recharge(amount);
    dataManager_.saveData();
    std::cout << "充值成功！当前余额: " << currentUser_->GetBalance() << " 元\n";
}

void UserController::showBalance()
{
    if (currentUser_ == nullptr) return;
    std::cout << "当前余额: " << currentUser_->GetBalance() << " 元\n";
}

void UserController::sendPackage()
{
    if (currentUser_ == nullptr) return;
    
    std::string receiverName, description;
    int type;
    double weight;
    int count;

    std::cout << "\n========== 发送快递 ==========\n";
    std::cout << "收件人用户名: ";
    std::cin >> receiverName;

    User* receiver = dataManager_.findUser(receiverName);
    if (receiver == nullptr)
    {
        std::cout << "收件人不存在\n";
        return;
    }

    std::cout << "请选择快递类型:\n";
    std::cout << "1. 易碎品 (8元/kg)\n";
    std::cout << "2. 图书 (2元/本)\n";
    std::cout << "3. 普通快递 (5元/kg)\n";
    std::cout << "请选择: ";
    std::cin >> type;

    double price = 0;
    Package* newPackage = nullptr;
    time_t now = time(nullptr);
    std::string sendTime = ctime(&now);
    sendTime.pop_back();

    std::cout << "物品描述: ";
    std::cin >> description;

    std::string packageId = std::to_string(dataManager_.getNextPackageId());

    switch (type)
    {
    case 1:
        std::cout << "重量(kg): ";
        std::cin >> weight;
        price = 8.0 * weight;
        newPackage = new FragilePackage(packageId, currentUser_->GetUsername(),
                                         receiverName, sendTime, description, weight);
        break;
    case 2:
        std::cout << "数量(本): ";
        std::cin >> count;
        price = 2.0 * count;
        newPackage = new BookPackage(packageId, currentUser_->GetUsername(),
                                      receiverName, sendTime, description, count);
        break;
    case 3:
        std::cout << "重量(kg): ";
        std::cin >> weight;
        price = 5.0 * weight;
        newPackage = new NormalPackage(packageId, currentUser_->GetUsername(),
                                        receiverName, sendTime, description, weight);
        break;
    default:
        std::cout << "无效类型\n";
        return;
    }

    if (currentUser_->GetBalance() < price)
    {
        std::cout << "余额不足！需要 " << price << " 元\n";
        delete newPackage;
        return;
    }

    currentUser_->DeductBalance(price);
    dataManager_.getAdmin().AddCompanyBalance(price);
    dataManager_.addPackage(newPackage);
    dataManager_.saveData();

    std::cout << "发送成功！快递单号: " << packageId << "\n";
    std::cout << "费用: " << price << " 元\n";
    std::cout << "当前余额: " << currentUser_->GetBalance() << " 元\n";
}

void UserController::receivePackage()
{
    if (currentUser_ == nullptr) return;
    
    std::cout << "\n========== 接收快递 ==========\n";
    auto& packages = dataManager_.getPackages();
    std::vector<int> waitingIndices;
    int index = 0;

    for (const auto& pkg : packages)
    {
        if (pkg->GetReceiver() == currentUser_->GetUsername() && pkg->IsWaitingSign())
        {
            waitingIndices.push_back(index);
            std::cout << index << ". 单号: " << pkg->GetId()
                      << " | 寄件人: " << pkg->GetSender()
                      << " | 描述: " << pkg->GetDescription() << "\n";
        }
        index++;
    }

    if (waitingIndices.empty())
    {
        std::cout << "没有待签收的快递\n";
        return;
    }

    std::cout << "请输入要签收的快递编号（多个用空格隔开，回车结束）: ";
    std::vector<int> selected;
    int num;
    while (std::cin >> num)
    {
        selected.push_back(num);
        if (std::cin.get() == '\n') break;
    }

    time_t now = time(nullptr);
    std::string receiveTime = ctime(&now);
    receiveTime.pop_back();

    for (int idx : selected)
    {
        if (idx >= 0 && idx < (int)packages.size())
        {
            packages[idx]->Sign(receiveTime);
            std::cout << "已签收: " << packages[idx]->GetId() << "\n";
        }
    }
    dataManager_.saveData();
}

void UserController::queryPackages()
{
    if (currentUser_ == nullptr) return;
    
    int choice;
    std::cout << "\n========== 查询快递 ==========\n";
    std::cout << "1. 我发出的\n";
    std::cout << "2. 我接收的\n";
    std::cout << "3. 按单号查询\n";
    std::cout << "请选择: ";
    std::cin >> choice;

    auto& packages = dataManager_.getPackages();

    switch (choice)
    {
    case 1:
        for (const auto& pkg : packages)
        {
            if (pkg->GetSender() == currentUser_->GetUsername())
                std::cout << "单号: " << pkg->GetId() << " | 收件人: " << pkg->GetReceiver()
                          << " | 状态: " << (pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "待签收" : "待揽收")) << "\n";
        }
        break;
    case 2:
        for (const auto& pkg : packages)
        {
            if (pkg->GetReceiver() == currentUser_->GetUsername())
                std::cout << "单号: " << pkg->GetId() << " | 寄件人: " << pkg->GetSender()
                          << " | 状态: " << (pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "待签收" : "待揽收")) << "\n";
        }
        break;
    case 3:
    {
        std::string id;
        std::cout << "快递单号: ";
        std::cin >> id;
        Package* pkg = dataManager_.findPackage(id);
        if (pkg)
            std::cout << "单号: " << pkg->GetId() << " | 寄件人: " << pkg->GetSender()
                      << " | 收件人: " << pkg->GetReceiver()
                      << " | 状态: " << (pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "待签收" : "待揽收")) << "\n";
        else
            std::cout << "未找到\n";
        break;
    }
    default:
        std::cout << "无效选择\n";
    }
}

void UserController::showMenu()
{
    if (currentUser_ == nullptr) return;
    
    int choice;
    while (true)
    {
        std::cout << "\n========== 用户菜单 ==========\n";
        std::cout << "1. 查询余额\n";
        std::cout << "2. 充值\n";
        std::cout << "3. 发送快递\n";
        std::cout << "4. 接收快递\n";
        std::cout << "5. 查询快递\n";
        std::cout << "6. 修改密码\n";
        std::cout << "0. 退出登录\n";
        std::cout << "请选择: ";
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            showBalance();
            break;
        case 2:
            recharge();
            break;
        case 3:
            sendPackage();
            break;
        case 4:
            receivePackage();
            break;
        case 5:
            queryPackages();
            break;
        case 6:
            changePassword();
            break;
        case 0:
            logout();
            return;
        default:
            std::cout << "无效选择\n";
        }
    }
}

User* UserController::getCurrentUser() const { return currentUser_; }
bool UserController::isLoggedIn() const { return currentUser_ != nullptr; }