#include "../include/Admin.h"

Admin::Admin(const std::string &username,
             const std::string &name,
             const std::string &password,
             const double &balance) : username_(username),
                                      name_(name),
                                      password_(password),
                                      balance_(balance)
{
}

// Getter方法
std::string Admin::GetUsername() const
{
    return username_;
}

std::string Admin::GetName() const
{
    return name_;
}

double Admin::GetBalance() const
{
    return balance_;
}
std::string Admin::GetPassword() const
{
    return password_;
}

// 检查密码是否相同
bool Admin ::CheckPassword(const std::string &pwd) const
{
    return pwd == password_;
}

// 修改密码
void Admin ::SetPassword(const std::string &newpwd)
{
    password_ = newpwd;
}

// 增加公司余额（收到快递费时调用）
void Admin ::AddBalance(double amount)
{
    if (amount > 0)
    {
        balance_ += amount;
    }
}

