#include "../include/User.h"

User::User(const std::string& username,
           const std::string& name,
           const std::string& phonenum,
           const std::string& password,
           const std::string& address)
    : Person(username, name, password, 0.0)
    , phonenum_(phonenum)
    , address_(address)
{
}

// 含余额的构造函数（用于从文件加载数据）
User::User(const std::string& username,
           const std::string& name,
           const std::string& phonenum,
           const std::string& password,
           const std::string& address,
           double balance)
    : Person(username, name, password, balance)
    , phonenum_(phonenum)
    , address_(address)
{
}

std::string User::GetPhonenum() const { return phonenum_; }
std::string User::GetAddress() const { return address_; }

void User::Recharge(double amount)
{
    AddBalance(amount);
}
