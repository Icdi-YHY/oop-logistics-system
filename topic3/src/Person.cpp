#include "../include/Person.h"

Person::Person(const std::string& username,
               const std::string& name,
               const std::string& password,
               double balance)
    : username_(username)
    , name_(name)
    , password_(password)
    , balance_(balance)
{
}

std::string Person::GetUsername() const { return username_; }
std::string Person::GetName() const { return name_; }
double Person::GetBalance() const { return balance_; }
std::string Person::GetPassword() const { return password_; }

bool Person::CheckPassword(const std::string& pwd) const
{
    return password_ == pwd;
}

void Person::SetPassword(const std::string& newpwd)
{
    password_ = newpwd;
}

void Person::AddBalance(double amount)
{
    if (amount > 0) {
        balance_ += amount;
    }
}

bool Person::DeductBalance(double amount)
{
    if (amount > 0 && balance_ >= amount) {
        balance_ -= amount;
        return true;
    }
    return false;
}
