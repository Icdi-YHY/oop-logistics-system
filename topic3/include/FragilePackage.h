#ifndef FRAGILE_PACKAGE_H
#define FRAGILE_PACKAGE_H

#include "Package.h"

class FragilePackage : public Package
{
private:
    double weight_;

public:
    FragilePackage(const std::string &packageId,
                   const std::string &sender,
                   const std::string &receiver,
                   const std::string &sendTime,
                   const std::string &description,
                   double weight);
    double GetWeight() const { return weight_; }
    double GetPrice() const override;
};

#endif