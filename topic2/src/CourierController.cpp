#include "../include/CourierController.h"
#include <iostream>

CourierController::CourierController(DataManager &dm)
    : dataManager_(dm), currentCourier_(nullptr)
{
}

bool CourierController::login()
{
    int id;
    std::string password;
    std::cout << "\n========== 快递员登录 ==========\n";
    std::cout << "快递员ID: ";
    std::cin >> id;
    while (std::cin.fail())
    {
        std::cin.clear();             // 1. 清除错误标志
        std::cin.ignore(10000, '\n'); // 清空缓冲区
        std::cout << "请输入快递员ID：";
        std::cin >> id;
    }
    std::cout << "密码: ";
    std::cin >> password;

    Courier *courier = dataManager_.findCourier(id);
    if (courier == nullptr)
    {
        std::cout << "快递员不存在\n";
        return false;
    }
    if (!courier->CheckPassword(password))
    {
        std::cout << "密码错误\n";
        return false;
    }
    currentCourier_ = courier;
    std::cout << "登录成功！欢迎 " << courier->GetName() << "\n";
    return true;
}

void CourierController::logout()
{
    currentCourier_ = nullptr;
    std::cout << "已退出登录\n";
}

void CourierController::showMyTasks()
{
    if (currentCourier_ == nullptr)
        return;

    std::cout << "\n我的待揽收任务:\n";
    bool found = false;
    for (const auto &pkg : dataManager_.getPackages())
    {
        if (pkg->GetCourierId() == currentCourier_->GetId() && pkg->IsWaitingCollect())
        {
            std::cout << "单号: " << pkg->GetId() << " | 寄件人: " << pkg->GetSender()
                      << " | 收件人: " << pkg->GetReceiver()
                      << " | 描述: " << pkg->GetDescription() << "\n";
            found = true;
        }
    }
    if (!found)
        std::cout << "暂无待揽收任务\n";
}

void CourierController::collectPackage()
{
    if (currentCourier_ == nullptr)
        return;

    std::vector<Package *> myTasks;
    for (auto pkg : dataManager_.getPackages())
    {
        if (pkg->GetCourierId() == currentCourier_->GetId() && pkg->IsWaitingCollect())
        {
            myTasks.push_back(pkg);
        }
    }

    if (myTasks.empty())
    {
        std::cout << "没有待揽收任务\n";
        return;
    }

    std::cout << "\n待揽收任务:\n";
    for (size_t i = 0; i < myTasks.size(); i++)
    {
        std::cout << i << ". 单号: " << myTasks[i]->GetId()
                  << " | 寄件人: " << myTasks[i]->GetSender()
                  << " | 收件人: " << myTasks[i]->GetReceiver() << "\n";
    }

    std::vector<int> selected;
    std::cout << "请输入要揽收的编号（多个用空格隔开，回车结束）: ";
    int num;
    while (std::cin >> num)
    {
        selected.push_back(num);
        if (std::cin.get() == '\n')
            break;
    }

    Admin &admin = dataManager_.getAdmin();
    double totalCommission = 0;

    for (int idx : selected)
    {
        if (idx >= 0 && idx < (int)myTasks.size())
        {
            Package *pkg = myTasks[idx];
            double price = pkg->GetPrice();
            double commission = price * 0.5;

            admin.DeductBalance(commission);
            currentCourier_->AddEarnings(commission);
            pkg->SetStatus(1); // 待揽收 -> 待签收
            totalCommission += commission;

            std::cout << "已揽收: " << pkg->GetId() << " 佣金: " << commission << " 元\n";
        }
    }

    dataManager_.saveData();
    std::cout << "揽收完成！总佣金: " << totalCommission << " 元\n";
    std::cout << "当前余额: " << currentCourier_->GetBalance() << " 元\n";
}

void CourierController::queryMyRecords()
{
    if (currentCourier_ == nullptr)
        return;

    std::cout << "\n我的揽收记录:\n";
    bool found = false;
    for (const auto &pkg : dataManager_.getPackages())
    {
        if (pkg->GetCourierId() == currentCourier_->GetId())
        {
            std::cout << "单号: " << pkg->GetId() << " | 寄件人: " << pkg->GetSender()
                      << " | 收件人: " << pkg->GetReceiver()
                      << " | 状态: " << (pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "待签收" : "待揽收")) << "\n";
            found = true;
        }
    }
    if (!found)
        std::cout << "暂无揽收记录\n";
}

void CourierController::changePassword()
{
    if (currentCourier_ == nullptr)
        return;

    std::string oldPwd, newPwd;
    std::cout << "原密码: ";
    std::cin >> oldPwd;

    if (!currentCourier_->CheckPassword(oldPwd))
    {
        std::cout << "原密码错误\n";
        return;
    }

    std::cout << "新密码: ";
    std::cin >> newPwd;
    currentCourier_->SetPassword(newPwd);
    dataManager_.saveData();
    std::cout << "密码修改成功\n";
}

void CourierController::showBalance()
{
    if (currentCourier_ == nullptr)
        return;
    std::cout << "当前余额: " << currentCourier_->GetBalance() << " 元\n";
}

void CourierController::showMenu()
{
    if (currentCourier_ == nullptr)
        return;

    int choice;
    while (true)
    {
        std::cout << "\n========== 快递员菜单 ==========\n";
        std::cout << "1. 查看我的任务\n";
        std::cout << "2. 揽收快递\n";
        std::cout << "3. 查看我的揽收记录\n";
        std::cout << "4. 查询余额\n";
        std::cout << "5. 修改密码\n";
        std::cout << "0. 退出登录\n";
        std::cout << "请选择: ";
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            showMyTasks();
            break;
        case 2:
            collectPackage();
            break;
        case 3:
            queryMyRecords();
            break;
        case 4:
            showBalance();
            break;
        case 5:
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

Courier *CourierController::getCurrentCourier() const { return currentCourier_; }
bool CourierController::isLoggedIn() const { return currentCourier_ != nullptr; }