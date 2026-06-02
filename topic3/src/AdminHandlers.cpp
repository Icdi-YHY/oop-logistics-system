#include "../include/Server.h"
#include "../include/Protocol.h"
#include <sstream>
#include <iomanip>

using namespace Protocol;

// ==================== 管理员功能 ====================

std::string Server::handleAdminShowUsers()
{
    std::stringstream data;
    for (const auto& user : dataManager_.getUsers()) {
        data << user.GetUsername() << "|" << user.GetName() << "|"
             << user.GetPhonenum() << "|" << user.GetAddress() << "|"
             << user.GetBalance() << "\n";
    }
    return buildDataResponse("所有用户", data.str());
}

std::string Server::handleAdminShowPackages()
{
    std::stringstream data;
    for (const auto& pkg : dataManager_.getPackages()) {
        std::string st = pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "运输中" : "待取件");
        data << pkg->GetId() << "|" << pkg->GetSender() << "|" << pkg->GetReceiver() << "|"
             << pkg->GetSendTime() << "|" << pkg->GetReceiveTime() << "|"
             << st << "|" << pkg->GetDescription() << "|" << pkg->GetCourierId() << "\n";
    }
    return buildDataResponse("所有包裹", data.str());
}

std::string Server::handleAdminShowBalance()
{
    std::ostringstream balMsg;
    balMsg << "公司余额：" << std::fixed << std::setprecision(1) << dataManager_.getAdmin().GetBalance() << " 元";
    return buildOkResponse(balMsg.str());
}

std::string Server::handleAdminAddCourier(const std::string& name, const std::string& phone, const std::string& pwd)
{
    int id = dataManager_.getNextCourierId();
    Courier newCourier(id, name, phone, pwd, 0.0);
    dataManager_.addCourier(newCourier);
    return buildOkResponse("快递员已添加，ID：" + std::to_string(id));
}

std::string Server::handleAdminRemoveCourier(int id)
{
    Courier* c = dataManager_.findCourier(id);
    if (c == nullptr) return buildErrResponse("未找到快递员");
    dataManager_.removeCourier(id);
    return buildOkResponse("快递员已移除");
}

std::string Server::handleAdminShowCouriers()
{
    std::stringstream data;
    for (const auto& courier : dataManager_.getCouriers()) {
        data << courier.GetId() << "|" << courier.GetName() << "|"
             << courier.GetPhone() << "|" << courier.GetBalance() << "\n";
    }
    return buildDataResponse("所有快递员", data.str());
}

std::string Server::handleAdminShowCourierDetail(int id)
{
    Courier* c = dataManager_.findCourier(id);
    if (c == nullptr) return buildErrResponse("未找到快递员");

    std::stringstream data;
    data << "ID：" << c->GetId() << " | 姓名：" << c->GetName()
         << " | 电话：" << c->GetPhone() << " | 余额：" << c->GetBalance() << "\n";

    auto pkgs = dataManager_.findPackagesByCourier(id);
    for (const auto& pkg : pkgs) {
        std::string st = pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "运输中" : "待取件");
        data << "  包裹 " << pkg->GetId() << " | 寄件人：" << pkg->GetSender()
             << " | 收件人：" << pkg->GetReceiver() << " | 状态：" << st << "\n";
    }
    return buildDataResponse("快递员详情", data.str());
}

std::string Server::handleAdminAssignCourier(const std::string& packageId, int courierId)
{
    Package* pkg = dataManager_.findPackage(packageId);
    if (pkg == nullptr) return buildErrResponse("未找到包裹");
    if (!pkg->IsWaitingCollect()) return buildErrResponse("包裹不在待取件状态");

    Courier* c = dataManager_.findCourier(courierId);
    if (c == nullptr) return buildErrResponse("未找到快递员");

    pkg->SetCourierId(courierId);
    return buildOkResponse("已将包裹 " + packageId + " 分配给快递员 " + c->GetName());
}

std::string Server::handleAdminQueryBySender(const std::string& sender)
{
    std::stringstream data;
    for (const auto& pkg : dataManager_.getPackages()) {
        if (pkg->GetSender() == sender) {
            std::string st = pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "运输中" : "待取件");
            data << pkg->GetId() << "|" << pkg->GetReceiver() << "|"
                 << pkg->GetSendTime() << "|" << st << "|" << pkg->GetDescription() << "\n";
        }
    }
    return buildDataResponse("按寄件人查询：" + sender, data.str());
}

std::string Server::handleAdminQueryByReceiver(const std::string& receiver)
{
    std::stringstream data;
    for (const auto& pkg : dataManager_.getPackages()) {
        if (pkg->GetReceiver() == receiver) {
            std::string st = pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "运输中" : "待取件");
            data << pkg->GetId() << "|" << pkg->GetSender() << "|"
                 << pkg->GetSendTime() << "|" << st << "|" << pkg->GetDescription() << "\n";
        }
    }
    return buildDataResponse("按收件人查询：" + receiver, data.str());
}

std::string Server::handleAdminQueryById(const std::string& id)
{
    Package* pkg = dataManager_.findPackage(id);
    if (pkg == nullptr) return buildErrResponse("未找到包裹");

    std::string st = pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "运输中" : "待取件");
    std::stringstream data;
    data << pkg->GetId() << "|" << pkg->GetSender() << "|" << pkg->GetReceiver() << "|"
         << pkg->GetSendTime() << "|" << pkg->GetReceiveTime() << "|" << st << "|"
         << pkg->GetDescription() << "|" << pkg->GetCourierId();
    return buildDataResponse("包裹详情", data.str());
}

std::string Server::handleAdminQueryByTime(int timeType, int dateType,
                                           const std::string& param1, const std::string& param2)
{
    auto& packages = dataManager_.getPackages();
    std::stringstream data;

    for (const auto& pkg : packages) {
        std::string refDate;
        if (timeType == 1) {
            refDate = formatDate(pkg->GetSendTime());
        } else if (timeType == 2) {
            if (!pkg->IsSigned()) continue;
            refDate = formatDate(pkg->GetReceiveTime());
        }

        bool include = false;
        if (dateType == 1) {
            include = (refDate == param1);
        } else if (dateType == 2) {
            include = (refDate.substr(0, 7) == param1);
        } else if (dateType == 3) {
            include = (refDate >= param1 && refDate <= param2);
        }

        if (include) {
            std::string st = pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "运输中" : "待取件");
            data << pkg->GetId() << "|" << pkg->GetSender() << "|" << pkg->GetReceiver() << "|"
                 << pkg->GetSendTime() << "|" << pkg->GetReceiveTime() << "|" << st << "\n";
        }
    }
    return buildDataResponse("查询结果", data.str());
}
