#include "../include/Server.h"
#include "../include/Protocol.h"
#include "../include/FragilePackage.h"
#include "../include/BookPackage.h"
#include "../include/NormalPackage.h"
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace Protocol;

// ==================== 登录处理 ====================

std::string Server::handleLogin(const std::string& type, const std::string& user,
                                const std::string& pwd, int& userType, std::string& sessionUsername)
{
    if (type == LOGIN_TYPE_USER) {
        User* u = dataManager_.findUser(user);
        if (u == nullptr) return buildErrResponse("未找到用户");
        if (!u->CheckPassword(pwd)) return buildErrResponse("密码错误");
        // 先检查是否已在线，再设置 session
        {
            std::string key = makeAccountKey(user, 1);
            if (onlineSessions_.find(key) != onlineSessions_.end())
                return buildErrResponse("该账号已在其他客户端登录");
            onlineSessions_[key] = 0;
        }
        userType = 1;
        sessionUsername = user;
        return buildOkResponse("用户登录成功，欢迎 " + u->GetName());
    }
    else if (type == LOGIN_TYPE_ADMIN) {
        Admin& admin = dataManager_.getAdmin();
        if (user != admin.GetUsername()) return buildErrResponse("未找到管理员");
        if (!admin.CheckPassword(pwd)) return buildErrResponse("密码错误");
        // 先检查是否已在线，再设置 session
        {
            std::string key = makeAccountKey(user, 2);
            if (onlineSessions_.find(key) != onlineSessions_.end())
                return buildErrResponse("该账号已在其他客户端登录");
            onlineSessions_[key] = 0;
        }
        userType = 2;
        sessionUsername = user;
        return buildOkResponse("管理员登录成功");
    }
    else if (type == LOGIN_TYPE_COURIER) {
        int courierId = std::stoi(user);
        Courier* c = dataManager_.findCourier(courierId);
        if (c == nullptr) return buildErrResponse("未找到快递员");
        if (!c->CheckPassword(pwd)) return buildErrResponse("密码错误");
        // 先检查是否已在线，再设置 session
        {
            std::string key = makeAccountKey(user, 3);
            if (onlineSessions_.find(key) != onlineSessions_.end())
                return buildErrResponse("该账号已在其他客户端登录");
            onlineSessions_[key] = 0;
        }
        userType = 3;
        sessionUsername = user;
        return buildOkResponse("快递员登录成功，欢迎 " + c->GetName());
    }
    return buildErrResponse("无效的登录类型");
}

// ==================== 注册处理 ====================

std::string Server::handleRegister(const std::string& username, const std::string& name,
                                   const std::string& phone, const std::string& pwd, const std::string& addr)
{
    if (dataManager_.findUser(username) != nullptr)
        return buildErrResponse("用户名已存在");
    User newUser(username, name, phone, pwd, addr);
    dataManager_.addUser(newUser);
    return buildOkResponse("注册成功");
}

// ==================== 发送快递 ====================

std::string Server::handleSendPackage(const std::string& sender, const std::string& receiver,
                                      int type, double weightOrCount, const std::string& desc)
{
    User* recvUser = dataManager_.findUser(receiver);
    if (recvUser == nullptr) return buildErrResponse("未找到收件人");

    User* sendUser = dataManager_.findUser(sender);
    if (sendUser == nullptr) return buildErrResponse("未找到寄件人");

    double price = 0;
    Package* newPackage = nullptr;
    time_t now = time(nullptr);
    std::string sendTime = ctime(&now);
    if (!sendTime.empty() && sendTime.back() == '\n') sendTime.pop_back();

    std::string packageId = std::to_string(dataManager_.getNextPackageId());

    switch (type) {
    case 1:
        price = 8.0 * weightOrCount;
        newPackage = new FragilePackage(packageId, sender, receiver, sendTime, desc, weightOrCount);
        break;
    case 2:
        price = 2.0 * weightOrCount;
        newPackage = new BookPackage(packageId, sender, receiver, sendTime, desc, (int)weightOrCount);
        break;
    case 3:
        price = 5.0 * weightOrCount;
        newPackage = new NormalPackage(packageId, sender, receiver, sendTime, desc, weightOrCount);
        break;
    default:
        return buildErrResponse("无效的包裹类型");
    }

    if (sendUser->GetBalance() < price) {
        delete newPackage;
        std::ostringstream errMsg;
        errMsg << "余额不足，需要 " << std::fixed << std::setprecision(1) << price << " 元";
        return buildErrResponse(errMsg.str());
    }

    sendUser->DeductBalance(price);
    dataManager_.getAdmin().AddCompanyBalance(price);
    dataManager_.addPackage(newPackage);

    std::stringstream result;
    result << "包裹已发送！ID：" << packageId << "，费用：" << std::fixed << std::setprecision(1) << price
           << " 元，余额：" << std::fixed << std::setprecision(1) << sendUser->GetBalance() << " 元";
    return buildOkResponse(result.str());
}

// ==================== 接收/签收快递 ====================

std::string Server::handleReceivePackage(const std::string& username, const std::string& indices)
{
    auto& packages = dataManager_.getPackages();
    std::vector<int> waitingIndices;

    for (size_t i = 0; i < packages.size(); i++) {
        if (packages[i]->GetReceiver() == username && packages[i]->IsWaitingSign()) {
            waitingIndices.push_back((int)i);
        }
    }

    if (indices.empty()) {
        if (waitingIndices.empty())
            return buildOkResponse("暂无待签收包裹");
        std::stringstream data;
        for (int idx : waitingIndices) {
            data << idx << "|" << packages[idx]->GetId() << "|"
                 << packages[idx]->GetSender() << "|"
                 << packages[idx]->GetDescription() << "\n";
        }
        return buildDataResponse("待签收包裹", data.str());
    }

    time_t now = time(nullptr);
    std::string receiveTime = ctime(&now);
    if (!receiveTime.empty() && receiveTime.back() == '\n') receiveTime.pop_back();

    std::istringstream iss(indices);
    std::string idxStr;
    int count = 0;
    while (std::getline(iss, idxStr, ',')) {
        if (idxStr.empty()) continue;
        int idx = std::stoi(idxStr);
        if (idx >= 0 && idx < (int)packages.size()) {
            if (packages[idx]->GetReceiver() == username && packages[idx]->IsWaitingSign()) {
                packages[idx]->Sign(receiveTime);
                count++;
            }
        }
    }

    return buildOkResponse("已签收 " + std::to_string(count) + " 个包裹");
}

// ==================== 用户查询 ====================

std::string Server::handleQuerySent(const std::string& username)
{
    auto& packages = dataManager_.getPackages();
    std::stringstream data;
    for (const auto& pkg : packages) {
        if (pkg->GetSender() == username) {
            std::string st = pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "运输中" : "待取件");
            data << pkg->GetId() << "|" << pkg->GetReceiver() << "|"
                 << pkg->GetSendTime() << "|" << st << "\n";
        }
    }
    return buildDataResponse("已寄出的包裹", data.str());
}

std::string Server::handleQueryReceived(const std::string& username)
{
    auto& packages = dataManager_.getPackages();
    std::stringstream data;
    for (const auto& pkg : packages) {
        if (pkg->GetReceiver() == username) {
            std::string st = pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "运输中" : "待取件");
            data << pkg->GetId() << "|" << pkg->GetSender() << "|"
                 << pkg->GetSendTime() << "|" << st << "\n";
        }
    }
    return buildDataResponse("已收到的包裹", data.str());
}

std::string Server::handleQueryById(const std::string& packageId)
{
    Package* pkg = dataManager_.findPackage(packageId);
    if (pkg == nullptr) return buildErrResponse("未找到包裹");

    std::string st = pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "运输中" : "待取件");
    std::stringstream data;
    data << pkg->GetId() << "|" << pkg->GetSender() << "|" << pkg->GetReceiver() << "|"
         << pkg->GetSendTime() << "|" << pkg->GetReceiveTime() << "|" << st << "|"
         << pkg->GetDescription();
    return buildDataResponse("包裹详情", data.str());
}

std::string Server::handleQueryByTime(const std::string& username, const std::string& timeType,
                                      const std::string& dateType, const std::string& dateParam, bool isSent)
{
    auto& packages = dataManager_.getPackages();
    std::stringstream data;

    for (const auto& pkg : packages) {
        bool match = isSent ? (pkg->GetSender() == username) : (pkg->GetReceiver() == username);
        if (!match) continue;

        std::string refDate = formatDate(isSent ? pkg->GetSendTime() : pkg->GetReceiveTime());

        bool include = false;
        if (dateType == "1") {
            include = (refDate == dateParam);
        } else if (dateType == "2") {
            include = (refDate.substr(0, 7) == dateParam);
        } else if (dateType == "3") {
            size_t comma = dateParam.find(',');
            if (comma != std::string::npos) {
                std::string start = dateParam.substr(0, comma);
                std::string end = dateParam.substr(comma + 1);
                include = (refDate >= start && refDate <= end);
            }
        }

        if (include) {
            std::string st = pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "运输中" : "待取件");
            data << pkg->GetId() << "|" << pkg->GetSender() << "|" << pkg->GetReceiver() << "|"
                 << pkg->GetSendTime() << "|" << st << "\n";
        }
    }

    return buildDataResponse("查询结果", data.str());
}

// ==================== 余额和密码管理 ====================

std::string Server::handleBalanceQuery(const std::string& username)
{
    User* u = dataManager_.findUser(username);
    if (u == nullptr) return buildErrResponse("未找到用户");
    std::ostringstream balMsg;
    balMsg << "余额：" << std::fixed << std::setprecision(1) << u->GetBalance() << " 元";
    return buildOkResponse(balMsg.str());
}

std::string Server::handleRecharge(const std::string& username, double amount)
{
    if (amount <= 0) return buildErrResponse("充值金额必须为正数");
    User* u = dataManager_.findUser(username);
    if (u == nullptr) return buildErrResponse("未找到用户");
    u->Recharge(amount);
    std::ostringstream rechMsg;
    rechMsg << "已充值 " << std::fixed << std::setprecision(1) << amount
            << " 元，余额：" << std::fixed << std::setprecision(1) << u->GetBalance() << " 元";
    return buildOkResponse(rechMsg.str());
}

std::string Server::handleChangePwd(const std::string& username, const std::string& oldPwd, const std::string& newPwd)
{
    User* u = dataManager_.findUser(username);
    if (u == nullptr) return buildErrResponse("未找到用户");
    if (!u->CheckPassword(oldPwd)) return buildErrResponse("旧密码错误");
    u->SetPassword(newPwd);
    return buildOkResponse("密码已修改");
}
