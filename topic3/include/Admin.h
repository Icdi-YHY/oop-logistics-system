#ifndef ADMIN_H
#define ADMIN_H

#include "Person.h"

class Admin : public Person
{
public:
    Admin(const std::string& username,
          const std::string& name,
          const std::string& password,
          double balance);

    void AddCompanyBalance(double amount);
};

#endif
