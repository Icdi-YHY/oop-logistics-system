#include "../include/Courier.h"

Courier::Courier(int id,
                 const std::string& name,
                 const std::string& phone,
                 const std::string& password,
                 double balance)
    : Person("courier" + std::to_string(id), name, password, balance)
    , id_(id)
    , phone_(phone)
{
}

int Courier::GetId() const { return id_; }
std::string Courier::GetPhone() const { return phone_; }

void Courier::AddEarnings(double amount)
{
    AddBalance(amount);
}
