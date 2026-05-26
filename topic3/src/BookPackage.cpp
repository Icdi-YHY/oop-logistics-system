#include "../include/BookPackage.h"

BookPackage::BookPackage(const std::string& packageId,
                         const std::string& sender,
                         const std::string& receiver,
                         const std::string& sendTime,
                         const std::string& description,
                         int count)
    : Package(packageId, sender, receiver, sendTime, description)
    , count_(count)
{
}

double BookPackage::GetPrice() const
{
    return 2.0 * count_;
}
