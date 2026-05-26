#ifndef USER_H
#define USER_H

#include "Person.h"

class User : public Person
{
private:
    std::string phonenum_;  // 电话号码
    std::string address_;   // 地址

public:
    // 构造函数（不含余额，余额默认为0）
    User(const std::string& username,
         const std::string& name,
         const std::string& phonenum,
         const std::string& password,
         const std::string& address);

    // 构造函数（含余额，用于从文件加载数据）
    User(const std::string& username,
         const std::string& name,
         const std::string& phonenum,
         const std::string& password,
         const std::string& address,
         double balance);

    std::string GetPhonenum() const;
    std::string GetAddress() const;
    void Recharge(double amount);
};

#endif
