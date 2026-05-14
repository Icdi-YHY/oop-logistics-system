#include "../include/NormalPackage.h"

NormalPackage::NormalPackage(const std::string& packageId,
                             const std::string& sender,
                             const std::string& receiver,
                             const std::string& sendTime,
                             const std::string& description,
                             double weight)
    : Package(packageId, sender, receiver, sendTime, description)
    , weight_(weight)
{
}

double NormalPackage::GetPrice() const
{
    return 5.0 * weight_;
}