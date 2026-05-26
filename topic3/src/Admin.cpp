#include "../include/Admin.h"

Admin::Admin(const std::string& username,
             const std::string& name,
             const std::string& password,
             double balance)
    : Person(username, name, password, balance)
{
}

void Admin::AddCompanyBalance(double amount)
{
    AddBalance(amount);
}
