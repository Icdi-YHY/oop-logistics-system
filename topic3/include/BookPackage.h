#ifndef BOOK_PACKAGE_H
#define BOOK_PACKAGE_H

#include "Package.h"

// 图书快递：按数量计费，2元/本
class BookPackage : public Package
{
private:
    int count_;  // 数量(本)

public:
    BookPackage(const std::string& packageId,
                const std::string& sender,
                const std::string& receiver,
                const std::string& sendTime,
                const std::string& description,
                int count);

    int GetCount() const { return count_; }
    double GetPrice() const override;
};

#endif
