#include "../include/Server.h"
#include "../include/Protocol.h"
#include <sstream>
#include <iomanip>

using namespace Protocol;

// ==================== 快递员功能 ====================

std::string Server::handleCourierMyTasks(int courierId)
{
    std::stringstream data;
    for (const auto& pkg : dataManager_.getPackages()) {
        if (pkg->GetCourierId() == courierId && pkg->IsWaitingCollect()) {
            data << pkg->GetId() << "|" << pkg->GetSender() << "|"
                 << pkg->GetReceiver() << "|" << pkg->GetDescription() << "\n";
        }
    }
    return buildDataResponse("我的任务", data.str());
}

std::string Server::handleCourierCollect(int courierId, const std::string& indices)
{
    auto& packages = dataManager_.getPackages();

    std::vector<Package*> myTasks;
    for (auto pkg : packages) {
        if (pkg->GetCourierId() == courierId && pkg->IsWaitingCollect()) {
            myTasks.push_back(pkg);
        }
    }

    if (indices.empty()) {
        if (myTasks.empty())
            return buildOkResponse("暂无待取件任务");
        std::stringstream data;
        for (size_t i = 0; i < myTasks.size(); i++) {
            data << i << "|" << myTasks[i]->GetId() << "|"
                 << myTasks[i]->GetSender() << "|" << myTasks[i]->GetReceiver() << "\n";
        }
        return buildDataResponse("待取件任务列表", data.str());
    }

    Admin& admin = dataManager_.getAdmin();
    Courier* courier = dataManager_.findCourier(courierId);
    if (courier == nullptr) return buildErrResponse("未找到快递员");

    double totalCommission = 0;
    std::istringstream iss(indices);
    std::string idxStr;
    int count = 0;
    while (std::getline(iss, idxStr, ',')) {
        if (idxStr.empty()) continue;
        int idx = std::stoi(idxStr);
        if (idx >= 0 && idx < (int)myTasks.size()) {
            Package* pkg = myTasks[idx];
            double commission = pkg->GetPrice() * 0.5;
            admin.DeductBalance(commission);
            courier->AddEarnings(commission);
            pkg->SetStatus(1);
            totalCommission += commission;
            count++;
        }
    }

    std::stringstream result;
    result << "已取件 " << count << " 个包裹，佣金：" << std::fixed << std::setprecision(1) << totalCommission
           << " 元，余额：" << std::fixed << std::setprecision(1) << courier->GetBalance() << " 元";
    return buildOkResponse(result.str());
}

std::string Server::handleCourierMyRecords(int courierId)
{
    std::stringstream data;
    for (const auto& pkg : dataManager_.getPackages()) {
        if (pkg->GetCourierId() == courierId) {
            std::string st = pkg->IsSigned() ? "已签收" : (pkg->IsWaitingSign() ? "运输中" : "待取件");
            data << pkg->GetId() << "|" << pkg->GetSender() << "|"
                 << pkg->GetReceiver() << "|" << st << "\n";
        }
    }
    return buildDataResponse("我的记录", data.str());
}

std::string Server::handleCourierBalance(int courierId)
{
    Courier* c = dataManager_.findCourier(courierId);
    if (c == nullptr) return buildErrResponse("未找到快递员");
    std::ostringstream balMsg;
    balMsg << "余额：" << std::fixed << std::setprecision(1) << c->GetBalance() << " 元";
    return buildOkResponse(balMsg.str());
}

std::string Server::handleCourierChangePwd(int courierId, const std::string& oldPwd, const std::string& newPwd)
{
    Courier* c = dataManager_.findCourier(courierId);
    if (c == nullptr) return buildErrResponse("未找到快递员");
    if (!c->CheckPassword(oldPwd)) return buildErrResponse("旧密码错误");
    c->SetPassword(newPwd);
    return buildOkResponse("密码已修改");
}
