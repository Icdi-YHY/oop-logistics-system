#ifndef ADMIN_CONTROLLER_H
#define ADMIN_CONTROLLER_H

#include "DataManager.h"

class AdminController
{
private:
    DataManager& dataManager_;
    bool isLoggedIn_;

    // 格式化日期辅助函数
    static std::string formatDate(const std::string& timeStr);
    // 打印快递详细信息
    void printPackageFull(const Package* pkg) const;
    // 各查询子功能
    void showAllPackagesAll() const;
    void showPackagesBySender() const;
    void showPackagesByReceiver() const;
    void showPackagesById() const;
    void showPackagesByTime();
    void showPackagesByTimeFilter(bool isSendTime);
    void queryByExactDate(bool isSendTime, const char* timeLabel);
    void queryByYearMonth(bool isSendTime, const char* timeLabel);
    void queryByDateRange(bool isSendTime, const char* timeLabel);

public:
    AdminController(DataManager& dm);

    bool login();
    void logout();
    void showAllUsers();
    void showAllPackages();
    void showCompanyBalance();
    void addCourier();
    void removeCourier();
    void showAllCouriers();
    void showCourierDetail();
    void assignCourierToPackage();
    void showMenu();

    bool isLoggedIn() const;
};

#endif