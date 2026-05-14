#include "../include/LogisticsSystem.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <vector>

// 构造函数
LogisticsSystem::LogisticsSystem()
    : admin_("admin", "物流公司管理员", "admin123", 0.0),
      currentUser_(nullptr),
      nextPackageId_(1)
{
    loadData();
}

// 辅助方法
User *LogisticsSystem::findUser(const std::string &username)
{
    for (auto &user : users_)
    {
        if (user.GetUsername() == username)
        {
            return &user;
        }
    }
    return nullptr;
}
std::string LogisticsSystem::generatePackageId()
{
    return std::to_string(nextPackageId_++);
}
// 辅助函数：打印快递信息
void LogisticsSystem::printPackage(const Package &pkg) const
{
    std::cout << "----------------------------\n";
    std::cout << "快递单号: " << pkg.GetId() << "\n";
    std::cout << "寄件人: " << pkg.GetSender() << "\n";
    std::cout << "收件人: " << pkg.GetReceiver() << "\n";
    std::cout << "寄件时间: " << pkg.GetSendTime() << "\n";
    if (pkg.GetStatus() == 1)
    {
        std::cout << "签收时间: " << pkg.GetReceiveTime() << "\n";
        std::cout << "状态: 已签收\n";
    }
    else
    {
        std::cout << "状态: 待签收\n";
    }
    std::cout << "物品描述: " << pkg.GetDescription() << "\n";
}

// 主入口
void LogisticsSystem::Run()
{
    int choice;
    while (true)
    {
        std::cout << "\n========== 物流管理系统 ==========\n";
        std::cout << "1. 用户注册\n";
        std::cout << "2. 用户登录\n";
        std::cout << "3. 管理员登录\n";
        std::cout << "0. 退出系统\n";
        std::cout << "请选择: ";
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            userRegister();
            break;
        case 2:
            if (userLogin())
            {
                userMenu();
            }
            break;
        case 3:
            adminLogin();
            break;

        case 0:
            saveData();
            std::cout << "感谢使用，再见\n";
            return;
        default:
            std::cout << "输入无效，请重新输入\n";
        }
    }
}

// 数据持久化
void LogisticsSystem::loadData()
{ // 1. 加载用户数据
    std::ifstream userFile("users.txt");
    if (userFile.is_open())
    {
        std::string line;
        while (std::getline(userFile, line))
        {
            std::stringstream ss(line);
            std::string username, name, phonenum, password, address, balanceStr;

            std::getline(ss, username, '|');
            std::getline(ss, name, '|');
            std::getline(ss, phonenum, '|');
            std::getline(ss, password, '|');
            std::getline(ss, balanceStr, '|');
            std::getline(ss, address, '|');

            double balance = std::stod(balanceStr);
            User user(username, name, phonenum, password, balance, address);
            users_.push_back(user);
        }
        userFile.close();
        std::cout << "加载用户数据成功\n";
    }

    // 2. 加载快递数据
    std::ifstream packageFile("packages.txt");
    if (packageFile.is_open())
    {
        std::string line;
        while (std::getline(packageFile, line))
        {
            std::stringstream ss(line);
            std::string packageId, sender, receiver, sendTime, receiveTime, statusStr, description;

            std::getline(ss, packageId, '|');
            std::getline(ss, sender, '|');
            std::getline(ss, receiver, '|');
            std::getline(ss, sendTime, '|');
            std::getline(ss, receiveTime, '|');
            std::getline(ss, statusStr, '|');
            std::getline(ss, description, '|');

            int status = std::stoi(statusStr);
            Package pkg(packageId, sender, receiver, sendTime, receiveTime, status, description);
            packages_.push_back(pkg);
        }
        packageFile.close();
        std::cout << "加载快递数据成功\n";
    }

    // 3. 加载管理员数据
    std::ifstream adminFile("admin.txt");
    if (adminFile.is_open())
    {
        std::string line;
        if (std::getline(adminFile, line))
        {
            std::stringstream ss(line);
            std::string username, name, password, balanceStr;

            std::getline(ss, username, '|');
            std::getline(ss, name, '|');
            std::getline(ss, password, '|');
            std::getline(ss, balanceStr, '|');

            double balance = std::stod(balanceStr);
            // 重新创建 admin_（使用现有对象）
            admin_ = Admin(username, name, password, balance);
        }
        adminFile.close();
        std::cout << "加载管理员数据成功\n";
    }

    // 4. 加载下一个快递单号
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
}
void LogisticsSystem::saveData() const
{ // 保存用户数据
    std::ofstream userFile("users.txt");
    if (userFile.is_open())
    {
        for (const auto &user : users_)
        {
            userFile << user.GetUsername() << "|"
                     << user.GetName() << "|"
                     << user.GetPhonenum() << "|"
                     << user.GetPassword() << "|" // 注意：实际应加密
                     << user.GetBalance() << "|"
                     << user.GetAddress() << "\n";
        }
        userFile.close();
    }

    // 保存快递数据
    std::ofstream packageFile("packages.txt");
    if (packageFile.is_open())
    {
        for (const auto &pkg : packages_)
        {
            packageFile << pkg.GetId() << "|"
                        << pkg.GetSender() << "|"
                        << pkg.GetReceiver() << "|"
                        << pkg.GetSendTime() << "|"
                        << pkg.GetReceiveTime() << "|"
                        << pkg.GetStatus() << "|"
                        << pkg.GetDescription() << "\n";
        }
        packageFile.close();
    }

    // 保存管理员数据（nextPackageId 等后续再处理）
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

    std::cout << "数据已保存\n";
}

// 用户菜单
void LogisticsSystem::userMenu()
{
    int choice;
    while (true)
    {
        std::cout << "\n========== 用户菜单 ==========\n";
        std::cout << "1. 修改密码\n";
        std::cout << "2. 查询余额\n";
        std::cout << "3. 充值\n";
        std::cout << "4. 发送快递\n";
        std::cout << "5. 接收快递\n";
        std::cout << "6. 查询快递\n";
        std::cout << "0. 退出登录\n";
        std::cout << "请选择: ";
        std::cin >> choice;

        switch (choice)
        {
        case 1:
        {
            std::string oldPwd, newPwd;
            std::cout << "请输入旧密码: ";
            std::cin >> oldPwd;

            if (!currentUser_->CheckPassword(oldPwd))
            {
                std::cout << "原密码错误！\n";
                break;
            }

            std::cout << "请输入新密码: ";
            std::cin >> newPwd;

            currentUser_->SetPassword(newPwd);
            saveData();
            std::cout << "密码修改成功！\n";
            break;
        }
        case 2:
            std::cout << "当前余额: " << currentUser_->GetBalance() << " 元\n";
            break;
        case 3:
        {
            double amount;
            std::cout << "请输入充值金额: ";
            std::cin >> amount;
            if (amount > 0)
            {
                currentUser_->Recharge(amount);
                saveData();
                std::cout << "充值成功！当前余额: " << currentUser_->GetBalance() << " 元\n";
            }
            else
            {
                std::cout << "充值金额必须大于0\n";
            }
            break;
        }
        case 4:
            sendPackage();
            break;
        case 5:
            receivePackage();
            break;
        case 6:
            queryPackages();
            break;
        case 0:
            currentUser_ = nullptr;
            std::cout << "已退出登录\n";
            return;
        default:
            std::cout << "无效选择，请重新输入\n";
        }
    }
}

// 用户功能
void LogisticsSystem::userRegister()
{
    std::string username, name, phonenum, password, address;
    std::cout << "\n========== 用户注册 ==========\n";

    while (true)
    {
        std::cout << "用户名：";
        std::cin >> username;

        if (findUser(username) != nullptr)
        {
            std::cout << "\n用户已存在，请重新输入\n";
        }
        else
        {
            break;
        }
    }
    std::cout << "姓名：";
    std::cin >> name;

    std::cout << "密码：";
    std::cin >> password;

    std::cout << "电话：";
    std::cin >> phonenum;

    std::cout << "地址：";
    std::cin >> address;

    User newUser(username, name, phonenum, password, 0.0, address);
    users_.push_back(newUser);

    // 保存到文件中
    saveData();

    std::cout << "\n注册成功！请登入。\n";
}
bool LogisticsSystem::userLogin()
{
    std::string username, password;
    std::cout << "\n========== 用户登录 ==========\n";
    std::cout << "用户名：";
    std::cin >> username;
    std::cout << "密码：";
    std::cin >> password;

    // 查找用户
    User *user = findUser(username);

    if (user == nullptr)
    {
        std::cout << "用户名不存在！\n";
        return false;
    }

    if (!user->CheckPassword(password))
    {
        std::cout << "密码错误！\n";
        return false;
    }

    // 登录成功，设置当前用户
    currentUser_ = user;
    std::cout << "登录成功！欢迎 " << user->GetUsername() << "\n";
    return true;
}
void LogisticsSystem::adminLogin()
{
    std::string username, password;

    std::cout << "\n========== 管理员登录 ==========\n";
    std::cout << "用户名: ";
    std::cin >> username;
    std::cout << "密码: ";
    std::cin >> password;

    // 验证管理员账号
    if (username == admin_.GetUsername() && admin_.CheckPassword(password))
    {
        std::cout << "登录成功！欢迎 " << admin_.GetName() << "\n";

        // 管理员菜单
        int choice;
        while (true)
        {
            std::cout << "\n========== 管理员菜单 ==========\n";
            std::cout << "1. 查看所有用户\n";
            std::cout << "2. 查看所有快递\n";
            std::cout << "3. 查看公司余额\n";
            std::cout << "0. 退出登录\n";
            std::cout << "请选择: ";
            std::cin >> choice;

            switch (choice)
            {
            case 1:
                viewAllUsers();
                break;
            case 2:
                viewAllPackages();
                break;
            case 3:
                std::cout << "公司账户余额: " << admin_.GetBalance() << " 元\n";
                break;
            case 0:
                std::cout << "已退出管理员登录\n";
                return;
            default:
                std::cout << "无效选择，请重新输入\n";
            }
        }
    }
    else
    {
        std::cout << "用户名或密码错误！\n";
    }
}
void LogisticsSystem::sendPackage()
{
    std::string receiverName, description;

    std::cout << "\n========== 发送快递 ==========\n";

    // 1. 输入收件人用户名
    std::cout << "收件人用户名: ";
    std::cin >> receiverName;

    // 2. 检查收件人是否存在
    User *receiver = findUser(receiverName);
    if (receiver == nullptr)
    {
        std::cout << "收件人不存在！\n";
        return;
    }

    // 3. 输入物品描述
    std::cout << "物品描述: ";
    std::cin >> description;

    // 4. 检查余额是否足够（15元）
    if (currentUser_->GetBalance() < 15)
    {
        std::cout << "余额不足！请先充值。\n";
        return;
    }

    // 5. 扣费
    currentUser_->DeductBalance(15);

    // 6. 增加公司余额
    admin_.AddBalance(15);

    // 7. 获取当前时间
    time_t now = time(nullptr);
    std::string sendTime = ctime(&now);
    sendTime.pop_back(); // 去掉末尾的换行符

    // 8. 生成快递单号
    std::string packageId = generatePackageId();

    // 9. 创建快递
    Package newPackage(packageId,
                       currentUser_->GetUsername(),
                       receiverName,
                       sendTime,
                       "",
                       0,
                       description);

    // 10. 添加到列表
    packages_.push_back(newPackage);

    // 11. 保存数据
    saveData();

    // 12. 显示结果
    std::cout << "发送成功！\n";
    std::cout << "快递单号: " << packageId << "\n";
    std::cout << "扣费: 15 元\n";
    std::cout << "当前余额: " << currentUser_->GetBalance() << " 元\n";
}
void LogisticsSystem::receivePackage()
{
    std::cout << "\n========== 接收快递 ==========\n";

    // 1. 找出当前用户所有未签收的快递
    std::vector<int> waitingPackages;
    int index = 0;

    for (const auto &pkg : packages_)
    {
        if (pkg.GetReceiver() == currentUser_->GetUsername() && pkg.IsWaitingSign())
        {
            waitingPackages.push_back(index);
            std::cout << index << ". 单号: " << pkg.GetId()
                      << " | 寄件人: " << pkg.GetSender()
                      << " | 描述: " << pkg.GetDescription()
                      << " | 寄件时间: " << pkg.GetSendTime() << "\n";
        }
        index++;
    }

    // 2. 如果没有待签收快递
    if (waitingPackages.empty())
    {
        std::cout << "没有待签收的快递。\n";
        return;
    }

    // 3. 用户选择要签收的快递
    std::cout << "\n请输入要签收的快递编号（多个用空格隔开，回车结束）: ";

    std::vector<int> selected;
    int num;
    while (std::cin >> num)
    {
        selected.push_back(num);
        if (std::cin.get() == '\n')
            break;
    }

    // 4. 签收选中的快递
    time_t now = time(nullptr);
    std::string receiveTime = ctime(&now);
    receiveTime.pop_back(); // 去掉换行符

    for (int idx : selected)
    {
        if (idx >= 0 && idx < (int)packages_.size())
        {
            packages_[idx].Sign(receiveTime);
            std::cout << "已签收: " << packages_[idx].GetId() << "\n";
        }
    }

    // 5. 保存数据
    saveData();

    std::cout << "退出签收！\n";
}
void LogisticsSystem::queryPackages() const
{
    int choice;
    std::cout << "\n========== 查询快递 ==========\n";
    std::cout << "1. 我发出的快递\n";
    std::cout << "2. 我接收的快递\n";
    std::cout << "3. 按快递单号查询\n";
    std::cout << "请选择: ";
    std::cin >> choice;

    switch (choice)
    {
    case 1:
    {
        std::cout << "\n========== 我发出的快递 ==========\n";
        bool found = false;
        for (const auto &pkg : packages_)
        {
            if (pkg.GetSender() == currentUser_->GetUsername())
            {
                 printPackage(pkg);
                found = true;
            }
        }
        if (!found)
            std::cout << "暂无发出的快递\n";
        break;
    }
    case 2:
    {
        std::cout << "\n========== 我接收的快递 ==========\n";
        bool found = false;
        for (const auto &pkg : packages_)
        {
            if (pkg.GetReceiver() == currentUser_->GetUsername())
            {
                printPackage(pkg);
                found = true;
            }
        }
        if (!found)
            std::cout << "暂无接收的快递\n";
        break;
    }
    case 3:
    {
        std::string packageId;
        std::cout << "请输入快递单号: ";
        std::cin >> packageId;

        bool found = false;
        for (const auto &pkg : packages_)
        {
            if (pkg.GetId() == packageId)
            {
                printPackage(pkg);
                found = true;
                break;
            }
        }
        if (!found)
            std::cout << "未找到该快递\n";
        break;
    }
    default:
        std::cout << "无效选择\n";
    }
}

// 管理员功能
void LogisticsSystem::viewAllUsers() const
{
    std::cout << "\n========== 所有用户信息 ==========\n";

    if (users_.empty())
    {
        std::cout << "暂无用户\n";
        return;
    }

    for (const auto &user : users_)
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
void LogisticsSystem::viewAllPackages() const
{
    std::cout << "\n========== 所有快递信息 ==========\n";

    if (packages_.empty())
    {
        std::cout << "暂无快递\n";
        return;
    }

    for (const auto &pkg : packages_)
    {
        std::cout << "----------------------------\n";
        std::cout << "快递单号: " << pkg.GetId() << "\n";
        std::cout << "寄件人: " << pkg.GetSender() << "\n";
        std::cout << "收件人: " << pkg.GetReceiver() << "\n";
        std::cout << "寄件时间: " << pkg.GetSendTime() << "\n";
        if (pkg.GetStatus() == 1)
        {
            std::cout << "签收时间: " << pkg.GetReceiveTime() << "\n";
            std::cout << "状态: 已签收\n";
        }
        else
        {
            std::cout << "状态: 待签收\n";
        }
        std::cout << "物品描述: " << pkg.GetDescription() << "\n";
    }
    std::cout << "----------------------------\n";
}