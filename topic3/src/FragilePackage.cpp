#include "../include/FragilePackage.h"

FragilePackage::FragilePackage(const std::string& packageId,
                               const std::string& sender,
                               const std::string& receiver,
                               const std::string& sendTime,
                               const std::string& description,
                               double weight)
    : Package(packageId, sender, receiver, sendTime, description)
    , weight_(weight)
{
}

double FragilePackage::GetPrice() const
{
    return 8.0 * weight_;
}