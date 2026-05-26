#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>

// 快递状态：0=待揽收 1=待签收 2=已签收
class Package
{
protected:
    std::string packageId_;     // 快递单号
    std::string sender_;        // 寄件人用户名
    std::string receiver_;      // 收件人用户名
    std::string sendTime_;      // 寄件时间
    std::string receiveTime_;   // 签收时间
    int status_;                // 0=待揽收 1=待签收 2=已签收
    std::string description_;   // 物品描述
    int courierId_;             // 揽收快递员ID（0=未分配）

public:
    Package(const std::string& packageId,
            const std::string& sender,
            const std::string& receiver,
            const std::string& sendTime,
            const std::string& description);

    virtual ~Package() = default;

    virtual double GetPrice() const = 0;  // 纯虚函数

    std::string GetId() const;
    std::string GetSender() const;
    std::string GetReceiver() const;
    std::string GetSendTime() const;
    std::string GetReceiveTime() const;
    int GetStatus() const;
    std::string GetDescription() const;
    int GetCourierId() const;

    void SetStatus(int status);
    void SetCourierId(int courierId);
    void SetReceiveTime(const std::string& time);
    void Sign(const std::string& receiveTime);

    bool IsWaitingCollect() const;
    bool IsWaitingSign() const;
    bool IsSigned() const;
};

#endif
