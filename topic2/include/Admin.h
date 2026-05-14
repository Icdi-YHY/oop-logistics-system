#ifndef ADMIN_H
#define ADMIN_H

#include "Person.h"

class Admin : public Person
{
public:
    // 构造函数
    Admin(const std::string& username,
          const std::string& name,
          const std::string& password,
          double balance);

    // 增加公司余额（收到快递费时调用）
    void AddCompanyBalance(double amount);
};

#endif