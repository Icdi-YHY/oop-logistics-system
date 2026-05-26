#ifndef FRAGILE_PACKAGE_H
#define FRAGILE_PACKAGE_H

#include "Package.h"

// 易碎品快递：按重量计费，8元/kg
class FragilePackage : public Package
{
private:
    double weight_;  // 重量(kg)

public:
    FragilePackage(const std::string& packageId,
                   const std::string& sender,
                   const std::string& receiver,
                   const std::string& sendTime,
                   const std::string& description,
                   double weight);

    double GetWeight() const { return weight_; }
    double GetPrice() const override;
};

#endif
