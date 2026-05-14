#ifndef USER_H
#define USER_H

#include "Person.h"

class User : public Person
{
private:
    std::string phonenum_;
    std::string address_;

public:
    // 构造函数
    User(const std::string& username,
         const std::string& name,
         const std::string& phonenum,
         const std::string& password,
         const std::string& address);

    // Getter方法
    std::string GetPhonenum() const;
    std::string GetAddress() const;

    // 充值余额
    void Recharge(double amount);
};

#endif