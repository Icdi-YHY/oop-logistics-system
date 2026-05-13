#include "../include/Package.h"

Package::Package(const std::string &packageId,
                 const std::string &sender,
                 const std::string &receiver,
                 const std::string &sendTime,
                 const std::string &receiveTime,
                 const int &status,
                 const std::string &description)
    : packageId_(packageId),
      sender_(sender),
      receiver_(receiver),
      sendTime_(sendTime),
      receiveTime_(receiveTime),
      status_(status),
      description_(description)
{
}

// Getter方法
std::string Package::GetId() const
{
    return packageId_;
}
std::string Package::GetSender() const
{
    return sender_;
}

std::string Package::GetReceiver() const
{
    return receiver_;
}
std::string Package::GetSendTime() const
{
    return sendTime_;
}
std::string Package::GetReceiveTime() const
{
    return receiveTime_;
}
int Package::GetStatus() const
{
    return status_;
}
std::string Package::GetDescription() const
{
    return description_;
}

// 是否未签收
bool Package::IsWaitingSign() const
{
    return status_ == 0;
}

// 修改为签收
void Package::Sign(const std::string &currentTime)
{
    if (status_ == 0)
    {
        status_ = 1;
        receiveTime_ = currentTime;
    }
}