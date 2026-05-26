#ifndef NORMAL_PACKAGE_H
#define NORMAL_PACKAGE_H

#include "Package.h"

// 普通快递：按重量计费，5元/kg
class NormalPackage : public Package
{
private:
    double weight_;  // 重量(kg)

public:
    NormalPackage(const std::string& packageId,
                  const std::string& sender,
                  const std::string& receiver,
                  const std::string& sendTime,
                  const std::string& description,
                  double weight);

    double GetWeight() const { return weight_; }
    double GetPrice() const override;
};

#endif
