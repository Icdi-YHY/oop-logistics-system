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
    Admin(const std::string &username,
          const std::string &name,
          const std::string &password,
          double balance);

    // Getter方法
    std::string GetUsername() const;
    std::string GetName() const;
    double GetBalance() const;

    // 检查密码是否相同
    bool CheckPassword(const std::string &pwd) const;

    // 修改密码
    void SetPassword(const std::string &newpwd);
    // 查看所有用户
    void ViewAllUsers() const;

    // 查看所有快递
    void ViewAllPackages() const;

    // 按条件查询快递
    void QueryPackage() const;

    // 增加公司余额（收到快递费时调用）
    void AddBalance(double amount);
};

#endif