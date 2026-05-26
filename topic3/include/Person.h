#ifndef PERSON_H
#define PERSON_H

#include <string>

/**
 * @brief 人员基类
 * 包含所有人员共有的属性：用户名、姓名、密码、账户余额
 */
class Person
{
protected:
    std::string username_;  // 用户名（唯一标识）
    std::string name_;      // 真实姓名
    std::string password_;  // 登录密码
    double balance_;        // 账户余额

public:
    Person(const std::string& username,
           const std::string& name,
           const std::string& password,
           double balance);

    std::string GetUsername() const;
    std::string GetName() const;
    double GetBalance() const;
    std::string GetPassword() const;
    bool CheckPassword(const std::string& pwd) const;
    void SetPassword(const std::string& newpwd);
    void AddBalance(double amount);
    bool DeductBalance(double amount);
};

#endif
