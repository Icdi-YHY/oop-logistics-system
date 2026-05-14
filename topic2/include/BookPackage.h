#ifndef BOOK_PACKAGE_H
#define BOOK_PACKAGE_H

#include "Package.h"

class BookPackage : public Package
{
private:
    int count_;

public:
    BookPackage(const std::string &packageId,
                const std::string &sender,
                const std::string &receiver,
                const std::string &sendTime,
                const std::string &description,
                int count);
    int GetCount() const { return count_; }
    double GetPrice() const override;
};

#endif