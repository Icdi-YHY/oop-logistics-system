#include "../include/UserController.h"
#include "../include/FragilePackage.h"
#include "../include/BookPackage.h"
#include "../include/NormalPackage.h"
#include <iostream>
#include <ctime>
#include <sstream>
#include <map>
#include <cctype>

UserController::UserController(DataManager &dm)
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

    User *user = dataManager_.findUser(username);
    if (user == nullptr)
    {
        std::cout << "用户名不存在！\n";
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
    std::cout << "注册成功！请登入。\n";
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

    User *receiver = dataManager_.findUser(receiverName);
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
    if (!(std::cin >> type))
    {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "输入非法";
        return;
    }

    double price = 0;
    Package *newPackage = nullptr;
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
    auto &packages = dataManager_.getPackages();
    std::vector<int> waitingIndices;
    int index = 0;

    for (const auto &pkg : packages)
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

// ========== 日期格式化 ==========

std::string UserController::formatDate(const std::string &timeStr)
{
    if (timeStr.empty()) return "";

    if (timeStr.length() >= 10 && isdigit(timeStr[0]) && timeStr[4] == '-' && timeStr[7] == '-')
        return timeStr.substr(0, 10);

    std::map<std::string, std::string> monthMap = {
        {"Jan", "01"}, {"Feb", "02"}, {"Mar", "03"}, {"Apr", "04"},
        {"May", "05"}, {"Jun", "06"}, {"Jul", "07"}, {"Aug", "08"},
        {"Sep", "09"}, {"Oct", "10"}, {"Nov", "11"}, {"Dec", "12"}};

    std::istringstream iss(timeStr);
    std::string weekday, month, day, time, year;
    iss >> weekday >> month >> day >> time >> year;

    if (day.length() == 1) day = "0" + day;
    return year + "-" + monthMap[month] + "-" + day;
}

// ========== 查询主菜单 ==========

void UserController::queryPackages()
{
    if (currentUser_ == nullptr) return;

    int choice;
    std::cout << "\n========== 查询快递 ==========\n";
    std::cout << "1. 我发出的\n";
    std::cout << "2. 我接收的\n";
    std::cout << "3. 按单号查询\n";
    std::cout << "4. 按时间查询\n";
    std::cout << "请选择: ";
    if (!(std::cin >> choice))
    {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "输入非法";
        return;
    }

    switch (choice)
    {
    case 1: querySent(); break;
    case 2: queryReceived(); break;
    case 3: queryById(); break;
    case 4: queryByTimeMenu(); break;
    default: std::cout << "无效选择\n";
    }
}

// ========== 我发出的 ==========

void UserController::querySent() const
{
    for (const auto &pkg : dataManager_.getPackages())
    {
        if (pkg->GetSender() == currentUser_->GetUsername())
            std::cout << "单号: " << pkg->GetId() << " | 收件人: " << pkg->GetReceiver()
                      << " | 状态: " << (pkg->IsSigned() ? "已签收" :
                          (pkg->IsWaitingSign() ? "待签收" : "待揽收")) << "\n";
    }
}

// ========== 我接收的 ==========

void UserController::queryReceived() const
{
    for (const auto &pkg : dataManager_.getPackages())
    {
        if (pkg->GetReceiver() == currentUser_->GetUsername())
            std::cout << "单号: " << pkg->GetId() << " | 寄件人: " << pkg->GetSender()
                      << " | 状态: " << (pkg->IsSigned() ? "已签收" :
                          (pkg->IsWaitingSign() ? "待签收" : "待揽收")) << "\n";
    }
}

// ========== 按单号查询 ==========

void UserController::queryById() const
{
    std::string id;
    std::cout << "快递单号: ";
    std::cin >> id;
    Package *pkg = dataManager_.findPackage(id);
    if (pkg)
        std::cout << "单号: " << pkg->GetId() << " | 寄件人: " << pkg->GetSender()
                  << " | 收件人: " << pkg->GetReceiver()
                  << " | 状态: " << (pkg->IsSigned() ? "已签收" :
                      (pkg->IsWaitingSign() ? "待签收" : "待揽收")) << "\n";
    else
        std::cout << "未找到\n";
}

// ========== 按时间查询入口 ==========

void UserController::queryByTimeMenu()
{
    int queryType;
    std::cout << "\n========== 按时间查询 ==========\n";
    std::cout << "1. 查询我发出的快递\n";
    std::cout << "2. 查询我接收的快递\n";
    std::cout << "请选择: ";

    if (!(std::cin >> queryType))
    {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "输入非法\n";
        return;
    }

    if (queryType == 1)
        queryByTimeFilter(true);
    else if (queryType == 2)
        queryByTimeFilter(false);
    else
        std::cout << "无效选择\n";
}

// ========== 按时间查询具体实现 ==========

void UserController::queryByTimeFilter(bool isSent)
{
    int timeChoice;
    std::cout << "\n1. 按具体日期查询\n";
    std::cout << "2. 按年月查询\n";
    std::cout << "3. 按时间段查询\n";
    std::cout << "请选择: ";

    if (!(std::cin >> timeChoice))
    {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "输入非法\n";
        return;
    }

    auto &packages = dataManager_.getPackages();
    const char *label = isSent ? "发出" : "接收";
    bool found = false;

    if (timeChoice == 1)
    {
        std::string date;
        std::cout << "请输入日期 (格式: YYYY-MM-DD): ";
        std::cin >> date;
        std::cout << "\n========== " << date << " 我" << label << "的快递 ==========\n";

        for (const auto &pkg : packages)
        {
            std::string matchField = isSent ? pkg->GetSender() : pkg->GetReceiver();
            if (matchField != currentUser_->GetUsername()) continue;
            if (formatDate(pkg->GetSendTime()) == date)
            {
                std::cout << "单号: " << pkg->GetId() << " | "
                          << (isSent ? "收件人: " : "寄件人: ")
                          << (isSent ? pkg->GetReceiver() : pkg->GetSender())
                          << " | 寄件时间: " << pkg->GetSendTime()
                          << " | 状态: " << (pkg->IsSigned() ? "已签收" :
                              (pkg->IsWaitingSign() ? "待签收" : "待揽收")) << "\n";
                found = true;
            }
        }
        if (!found) std::cout << "未找到 " << date << " " << label << "的快递\n";
    }
    else if (timeChoice == 2)
    {
        std::string yearMonth;
        std::cout << "请输入年月 (格式: YYYY-MM): ";
        std::cin >> yearMonth;
        std::cout << "\n========== " << yearMonth << " 我" << label << "的快递 ==========\n";

        for (const auto &pkg : packages)
        {
            std::string matchField = isSent ? pkg->GetSender() : pkg->GetReceiver();
            if (matchField != currentUser_->GetUsername()) continue;
            if (formatDate(pkg->GetSendTime()).substr(0, 7) == yearMonth)
            {
                std::cout << "单号: " << pkg->GetId() << " | "
                          << (isSent ? "收件人: " : "寄件人: ")
                          << (isSent ? pkg->GetReceiver() : pkg->GetSender())
                          << " | 寄件时间: " << pkg->GetSendTime()
                          << " | 状态: " << (pkg->IsSigned() ? "已签收" :
                              (pkg->IsWaitingSign() ? "待签收" : "待揽收")) << "\n";
                found = true;
            }
        }
        if (!found) std::cout << "未找到 " << yearMonth << " " << label << "的快递\n";
    }
    else if (timeChoice == 3)
    {
        std::string startDate, endDate;
        std::cout << "请输入开始日期 (格式: YYYY-MM-DD): ";
        std::cin >> startDate;
        std::cout << "请输入结束日期 (格式: YYYY-MM-DD): ";
        std::cin >> endDate;
        std::cout << "\n========== " << startDate << " 至 " << endDate
                  << " 我" << label << "的快递 ==========\n";

        for (const auto &pkg : packages)
        {
            std::string matchField = isSent ? pkg->GetSender() : pkg->GetReceiver();
            if (matchField != currentUser_->GetUsername()) continue;
            std::string sendDate = formatDate(pkg->GetSendTime());
            if (sendDate >= startDate && sendDate <= endDate)
            {
                std::cout << "单号: " << pkg->GetId() << " | "
                          << (isSent ? "收件人: " : "寄件人: ")
                          << (isSent ? pkg->GetReceiver() : pkg->GetSender())
                          << " | 寄件时间: " << pkg->GetSendTime()
                          << " | 状态: " << (pkg->IsSigned() ? "已签收" :
                              (pkg->IsWaitingSign() ? "待签收" : "待揽收")) << "\n";
                found = true;
            }
        }
        if (!found) std::cout << "未找到 " << startDate << " 至 " << endDate << " " << label << "的快递\n";
    }
    else
        std::cout << "无效选择\n";
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
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "输入非法";
            continue;
        }

        switch (choice)
        {
        case 1: showBalance(); break;
        case 2: recharge(); break;
        case 3: sendPackage(); break;
        case 4: receivePackage(); break;
        case 5: queryPackages(); break;
        case 6: changePassword(); break;
        case 0: logout(); return;
        default: std::cout << "无效选择\n";
        }
    }
}

User *UserController::getCurrentUser() const { return currentUser_; }
bool UserController::isLoggedIn() const { return currentUser_ != nullptr; }