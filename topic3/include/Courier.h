#ifndef COURIER_H
#define COURIER_H

#include "Person.h"

class Courier : public Person
{
private:
    int id_;
    std::string phone_;

public:
    Courier(int id,
            const std::string& name,
            const std::string& phone,
            const std::string& password,
            double balance);

    int GetId() const;
    std::string GetPhone() const;
    void AddEarnings(double amount);
};

#endif