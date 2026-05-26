#include "../include/Server.h"
#include "../include/Protocol.h"
#include "../include/FragilePackage.h"
#include "../include/BookPackage.h"
#include "../include/NormalPackage.h"
#include <sstream>
#include <ctime>

using namespace Protocol;

// ==================== 登录处理 ====================

std::string Server::handleLogin(const std::string& type, const std::string& user,
                                const std::string& pwd, int& userType, std::string& sessionUsername)
{
    if (type == LOGIN_TYPE_USER) {
        User* u = dataManager_.findUser(user);
        if (u == nullptr) return buildErrResponse("User not found");
        if (!u->CheckPassword(pwd)) return buildErrResponse("Wrong password");
        userType = 1;
        sessionUsername = user;
        return buildOkResponse("User login success, welcome " + u->GetName());
    }
    else if (type == LOGIN_TYPE_ADMIN) {
        Admin& admin = dataManager_.getAdmin();
        if (user != admin.GetUsername()) return buildErrResponse("Admin not found");
        if (!admin.CheckPassword(pwd)) return buildErrResponse("Wrong password");
        userType = 2;
        sessionUsername = user;
        return buildOkResponse("Admin login success");
    }
    else if (type == LOGIN_TYPE_COURIER) {
        int courierId = std::stoi(user);
        Courier* c = dataManager_.findCourier(courierId);
        if (c == nullptr) return buildErrResponse("Courier not found");
        if (!c->CheckPassword(pwd)) return buildErrResponse("Wrong password");
        userType = 3;
        sessionUsername = user;
        return buildOkResponse("Courier login success, welcome " + c->GetName());
    }
    return buildErrResponse("Invalid login type");
}

// ==================== 注册处理 ====================

std::string Server::handleRegister(const std::string& username, const std::string& name,
                                   const std::string& phone, const std::string& pwd, const std::string& addr)
{
    if (dataManager_.findUser(username) != nullptr)
        return buildErrResponse("Username already exists");
    User newUser(username, name, phone, pwd, addr);
    dataManager_.addUser(newUser);
    return buildOkResponse("Registration success");
}

// ==================== 发送快递 ====================

std::string Server::handleSendPackage(const std::string& sender, const std::string& receiver,
                                      int type, double weightOrCount, const std::string& desc)
{
    User* recvUser = dataManager_.findUser(receiver);
    if (recvUser == nullptr) return buildErrResponse("Receiver not found");

    User* sendUser = dataManager_.findUser(sender);
    if (sendUser == nullptr) return buildErrResponse("Sender not found");

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
        return buildErrResponse("Invalid package type");
    }

    if (sendUser->GetBalance() < price) {
        delete newPackage;
        return buildErrResponse("Insufficient balance, need " + std::to_string(price) + " yuan");
    }

    sendUser->DeductBalance(price);
    dataManager_.getAdmin().AddCompanyBalance(price);
    dataManager_.addPackage(newPackage);

    std::stringstream result;
    result << "Package sent! ID: " << packageId << ", Fee: " << price
           << " yuan, Balance: " << sendUser->GetBalance() << " yuan";
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
            return buildOkResponse("No packages waiting for sign");
        std::stringstream data;
        for (int idx : waitingIndices) {
            data << idx << "|" << packages[idx]->GetId() << "|"
                 << packages[idx]->GetSender() << "|"
                 << packages[idx]->GetDescription() << "\n";
        }
        return buildDataResponse("Pending packages", data.str());
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

    return buildOkResponse("Signed " + std::to_string(count) + " packages");
}

// ==================== 用户查询 ====================

std::string Server::handleQuerySent(const std::string& username)
{
    auto& packages = dataManager_.getPackages();
    std::stringstream data;
    for (const auto& pkg : packages) {
        if (pkg->GetSender() == username) {
            std::string st = pkg->IsSigned() ? "Signed" : (pkg->IsWaitingSign() ? "In transit" : "Pending pickup");
            data << pkg->GetId() << "|" << pkg->GetReceiver() << "|"
                 << pkg->GetSendTime() << "|" << st << "\n";
        }
    }
    return buildDataResponse("Sent packages", data.str());
}

std::string Server::handleQueryReceived(const std::string& username)
{
    auto& packages = dataManager_.getPackages();
    std::stringstream data;
    for (const auto& pkg : packages) {
        if (pkg->GetReceiver() == username) {
            std::string st = pkg->IsSigned() ? "Signed" : (pkg->IsWaitingSign() ? "In transit" : "Pending pickup");
            data << pkg->GetId() << "|" << pkg->GetSender() << "|"
                 << pkg->GetSendTime() << "|" << st << "\n";
        }
    }
    return buildDataResponse("Received packages", data.str());
}

std::string Server::handleQueryById(const std::string& packageId)
{
    Package* pkg = dataManager_.findPackage(packageId);
    if (pkg == nullptr) return buildErrResponse("Package not found");

    std::string st = pkg->IsSigned() ? "Signed" : (pkg->IsWaitingSign() ? "In transit" : "Pending pickup");
    std::stringstream data;
    data << pkg->GetId() << "|" << pkg->GetSender() << "|" << pkg->GetReceiver() << "|"
         << pkg->GetSendTime() << "|" << pkg->GetReceiveTime() << "|" << st << "|"
         << pkg->GetDescription();
    return buildDataResponse("Package detail", data.str());
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
            std::string st = pkg->IsSigned() ? "Signed" : (pkg->IsWaitingSign() ? "In transit" : "Pending pickup");
            data << pkg->GetId() << "|" << pkg->GetSender() << "|" << pkg->GetReceiver() << "|"
                 << pkg->GetSendTime() << "|" << st << "\n";
        }
    }

    return buildDataResponse("Query result", data.str());
}

// ==================== 余额和密码管理 ====================

std::string Server::handleBalanceQuery(const std::string& username)
{
    User* u = dataManager_.findUser(username);
    if (u == nullptr) return buildErrResponse("User not found");
    return buildOkResponse("Balance: " + std::to_string(u->GetBalance()) + " yuan");
}

std::string Server::handleRecharge(const std::string& username, double amount)
{
    if (amount <= 0) return buildErrResponse("Amount must be positive");
    User* u = dataManager_.findUser(username);
    if (u == nullptr) return buildErrResponse("User not found");
    u->Recharge(amount);
    return buildOkResponse("Recharged " + std::to_string(amount)
                           + " yuan, balance: " + std::to_string(u->GetBalance()) + " yuan");
}

std::string Server::handleChangePwd(const std::string& username, const std::string& oldPwd, const std::string& newPwd)
{
    User* u = dataManager_.findUser(username);
    if (u == nullptr) return buildErrResponse("User not found");
    if (!u->CheckPassword(oldPwd)) return buildErrResponse("Wrong old password");
    u->SetPassword(newPwd);
    return buildOkResponse("Password changed");
}
