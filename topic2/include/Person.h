#ifndef PERSON_H
#define PERSON_H

#include <string>

class Person
{
protected:  // 改为 protected，让子类可以访问
    std::string username_;
    std::string name_;
    std::string password_;
    double balance_;

public:
    // 构造函数
    Person(const std::string& username,
           const std::string& name,
           const std::string& password,
           double balance);

    // Getter方法（const）
    std::string GetUsername() const;
    std::string GetName() const;
    double GetBalance() const;
    std::string GetPassword() const;

    // 检查密码
    bool CheckPassword(const std::string& pwd) const;

    // 修改密码
    void SetPassword(const std::string& newpwd);

    // 增加余额
    void AddBalance(double amount);

    // 扣费（余额不足返回false）
    bool DeductBalance(double amount);
};

#endif