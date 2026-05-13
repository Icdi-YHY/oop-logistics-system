#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>

class Package
{
private:
    std::string packageId_;
    std::string sender_;
    std::string receiver_;
    std::string sendTime_;
    std::string receiveTime_;
    int status_;
    std::string description_;

public:
    // 构造函数
    Package(const std::string &packageId,
            const std::string &sender,
            const std::string &receiver,
            const std::string &sendTime,
            const std::string &receiveTime,
            const int &status,
            const std::string &description);

    // Getter方法
    std::string GetId() const;
    std::string GetSender() const;
    std::string GetReceiver() const;
    std::string GetSendTime() const;
    std::string GetReceiveTime() const;
    int GetStatus() const;
    std::string GetDescription() const;

    // 是否未签收
    bool IsWaitingSign() const;

    // 修改为签收
    void Sign(const std::string &currentTime);
};

#endif