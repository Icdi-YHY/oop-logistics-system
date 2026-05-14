#ifndef USER_H
#define USER_H

#include <string>

class User
{
private:
    std::string username_;
    std::string name_;
    std::string phonenum_;
    std::string password_;
    double balance_;
    std::string address_;

public:
    // 构造函数
    User(const std::string &username,
         const std::string &name,
         const std::string &phonenum,
         const std::string &password,
         const double &balance,
         const std::string &address);

    // Getter方法
    std::string GetUsername() const;
    std::string GetName() const;
    std::string GetPhonenum() const;
    double GetBalance() const;
    std::string GetAddress() const;
    std::string GetPassword() const;

    // 检查密码是否相同
    bool CheckPassword(const std::string &pwd) const;

    // 修改密码
    void SetPassword(const std::string &newpwd);

    // 充值余额
    void Recharge(double amount);

    // 扣费
    bool DeductBalance(double amount);
};

#endif