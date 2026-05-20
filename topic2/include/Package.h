#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>

class Package
{
protected:
    std::string packageId_;
    std::string sender_;
    std::string receiver_;
    std::string sendTime_;
    std::string receiveTime_;
    int status_; // 0=待揽收 1=待签收 2=已签收
    std::string description_;
    int courierId_; // 揽收快递员ID（0=未分配）

public:
    // 构造函数
    Package(const std::string &packageId,
            const std::string &sender,
            const std::string &receiver,
            const std::string &sendTime,
            const std::string &description);

    virtual ~Package() = default;

    // 纯虚函数
    virtual double GetPrice() const = 0;

    // Getter方法
    std::string GetId() const;
    std::string GetSender() const;
    std::string GetReceiver() const;
    std::string GetSendTime() const;
    std::string GetReceiveTime() const;
    int GetStatus() const;
    std::string GetDescription() const;
    int GetCourierId() const;

    // Setter方法
    void SetStatus(int status);
    void SetCourierId(int courierId);
    void SetReceiveTime(const std::string &time);
    void Sign(const std::string &receiveTime);

    // 状态判断
    bool IsWaitingCollect() const; // 待揽收
    bool IsWaitingSign() const;    // 待签收
    bool IsSigned() const;         // 已签收
};

#endif