#ifndef NORMAL_PACKAGE_H
#define NORMAL_PACKAGE_H

#include "Package.h"

class NormalPackage : public Package
{
private:
    double weight_;

public:
    NormalPackage(const std::string &packageId,
                  const std::string &sender,
                  const std::string &receiver,
                  const std::string &sendTime,
                  const std::string &description,
                  double weight);
    double GetWeight() const { return weight_; }
    double GetPrice() const override;
};

#endif