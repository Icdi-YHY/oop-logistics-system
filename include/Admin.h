#ifndef ADMIN_H
#define ADMIN_H

#include <string>
class Admin
{
private:
    std::string username_;
    std::string name_;
    std::string password_;
    double balance_;

public:
    // 构造函数
    Admin();

    // Getter方法
    std::string GetUsername() const;
    std::string GetName() const;
    double GetBalance() const;

    // 查看所有用户
    void ViewAllUsers() const {};

    // 查看所有快递
    void ViewAllPackages() const {};

    // 按条件查询快递
    void QueryPackage() const {};
};

#endif