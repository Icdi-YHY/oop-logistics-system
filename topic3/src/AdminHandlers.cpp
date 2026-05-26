#include "../include/Server.h"
#include "../include/Protocol.h"
#include <sstream>

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
    return buildDataResponse("All users", data.str());
}

std::string Server::handleAdminShowPackages()
{
    std::stringstream data;
    for (const auto& pkg : dataManager_.getPackages()) {
        std::string st = pkg->IsSigned() ? "Signed" : (pkg->IsWaitingSign() ? "In transit" : "Pending pickup");
        data << pkg->GetId() << "|" << pkg->GetSender() << "|" << pkg->GetReceiver() << "|"
             << pkg->GetSendTime() << "|" << pkg->GetReceiveTime() << "|"
             << st << "|" << pkg->GetDescription() << "|" << pkg->GetCourierId() << "\n";
    }
    return buildDataResponse("All packages", data.str());
}

std::string Server::handleAdminShowBalance()
{
    return buildOkResponse("Company balance: " + std::to_string(dataManager_.getAdmin().GetBalance()) + " yuan");
}

std::string Server::handleAdminAddCourier(const std::string& name, const std::string& phone, const std::string& pwd)
{
    int id = dataManager_.getNextCourierId();
    Courier newCourier(id, name, phone, pwd, 0.0);
    dataManager_.addCourier(newCourier);
    return buildOkResponse("Courier added, ID: " + std::to_string(id));
}

std::string Server::handleAdminRemoveCourier(int id)
{
    Courier* c = dataManager_.findCourier(id);
    if (c == nullptr) return buildErrResponse("Courier not found");
    dataManager_.removeCourier(id);
    return buildOkResponse("Courier removed");
}

std::string Server::handleAdminShowCouriers()
{
    std::stringstream data;
    for (const auto& courier : dataManager_.getCouriers()) {
        data << courier.GetId() << "|" << courier.GetName() << "|"
             << courier.GetPhone() << "|" << courier.GetBalance() << "\n";
    }
    return buildDataResponse("All couriers", data.str());
}

std::string Server::handleAdminShowCourierDetail(int id)
{
    Courier* c = dataManager_.findCourier(id);
    if (c == nullptr) return buildErrResponse("Courier not found");

    std::stringstream data;
    data << "ID: " << c->GetId() << " | Name: " << c->GetName()
         << " | Phone: " << c->GetPhone() << " | Balance: " << c->GetBalance() << "\n";

    auto pkgs = dataManager_.findPackagesByCourier(id);
    for (const auto& pkg : pkgs) {
        std::string st = pkg->IsSigned() ? "Signed" : (pkg->IsWaitingSign() ? "In transit" : "Pending pickup");
        data << "  Package " << pkg->GetId() << " | Sender: " << pkg->GetSender()
             << " | Receiver: " << pkg->GetReceiver() << " | Status: " << st << "\n";
    }
    return buildDataResponse("Courier detail", data.str());
}

std::string Server::handleAdminAssignCourier(const std::string& packageId, int courierId)
{
    Package* pkg = dataManager_.findPackage(packageId);
    if (pkg == nullptr) return buildErrResponse("Package not found");
    if (!pkg->IsWaitingCollect()) return buildErrResponse("Package not in pending pickup state");

    Courier* c = dataManager_.findCourier(courierId);
    if (c == nullptr) return buildErrResponse("Courier not found");

    pkg->SetCourierId(courierId);
    return buildOkResponse("Assigned package " + packageId + " to courier " + c->GetName());
}

std::string Server::handleAdminQueryBySender(const std::string& sender)
{
    std::stringstream data;
    for (const auto& pkg : dataManager_.getPackages()) {
        if (pkg->GetSender() == sender) {
            std::string st = pkg->IsSigned() ? "Signed" : (pkg->IsWaitingSign() ? "In transit" : "Pending pickup");
            data << pkg->GetId() << "|" << pkg->GetReceiver() << "|"
                 << pkg->GetSendTime() << "|" << st << "|" << pkg->GetDescription() << "\n";
        }
    }
    return buildDataResponse("Packages by sender: " + sender, data.str());
}

std::string Server::handleAdminQueryByReceiver(const std::string& receiver)
{
    std::stringstream data;
    for (const auto& pkg : dataManager_.getPackages()) {
        if (pkg->GetReceiver() == receiver) {
            std::string st = pkg->IsSigned() ? "Signed" : (pkg->IsWaitingSign() ? "In transit" : "Pending pickup");
            data << pkg->GetId() << "|" << pkg->GetSender() << "|"
                 << pkg->GetSendTime() << "|" << st << "|" << pkg->GetDescription() << "\n";
        }
    }
    return buildDataResponse("Packages by receiver: " + receiver, data.str());
}

std::string Server::handleAdminQueryById(const std::string& id)
{
    Package* pkg = dataManager_.findPackage(id);
    if (pkg == nullptr) return buildErrResponse("Package not found");

    std::string st = pkg->IsSigned() ? "Signed" : (pkg->IsWaitingSign() ? "In transit" : "Pending pickup");
    std::stringstream data;
    data << pkg->GetId() << "|" << pkg->GetSender() << "|" << pkg->GetReceiver() << "|"
         << pkg->GetSendTime() << "|" << pkg->GetReceiveTime() << "|" << st << "|"
         << pkg->GetDescription() << "|" << pkg->GetCourierId();
    return buildDataResponse("Package detail", data.str());
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
            std::string st = pkg->IsSigned() ? "Signed" : (pkg->IsWaitingSign() ? "In transit" : "Pending pickup");
            data << pkg->GetId() << "|" << pkg->GetSender() << "|" << pkg->GetReceiver() << "|"
                 << pkg->GetSendTime() << "|" << pkg->GetReceiveTime() << "|" << st << "\n";
        }
    }
    return buildDataResponse("Query result", data.str());
}
