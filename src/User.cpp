#include "../include/User.h"

// 构造函数
User ::User(const std::string &username,
            const std::string &name,
            const std::string &phonenum,
            const std::string &password,
            const double &balance,
            const std::string &address)
    : username_(username),
      name_(name),
      phonenum_(phonenum),
      password_(password),
      balance_(balance),
      address_(address) {}

// Getter方法
std::string User::GetUsername() const
{
    return username_;
}

std::string User::GetName() const
{
    return name_;
}
std::string User::GetPhonenum() const
{
    return phonenum_;
}
double User::GetBalance() const
{
    return balance_;
}
std::string User::GetAddress() const
{
    return address_;
}

// 检查密码是否相同
bool User::CheckPassword(const std::string &pwd) const
{
    return pwd == password_;
}

// 修改密码
void User::SetPassword(const std::string &newpwd)
{
    password_ = newpwd;
}

// 充值余额
void User::Recharge(double amount)
{
    if (amount > 0)
    {
        balance_ += amount;
    }
}

// 扣费
bool User::DeductBalance(double amount)
{
    if (amount > 0 && balance_ >= amount)
    {
        balance_ -= amount;
        return true;
    }
    return false;
}