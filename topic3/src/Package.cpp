#include "../include/Package.h"

Package::Package(const std::string& packageId,
                 const std::string& sender,
                 const std::string& receiver,
                 const std::string& sendTime,
                 const std::string& description)
    : packageId_(packageId)
    , sender_(sender)
    , receiver_(receiver)
    , sendTime_(sendTime)
    , receiveTime_("")
    , status_(0)
    , description_(description)
    , courierId_(0)
{
}

std::string Package::GetId() const { return packageId_; }
std::string Package::GetSender() const { return sender_; }
std::string Package::GetReceiver() const { return receiver_; }
std::string Package::GetSendTime() const { return sendTime_; }
std::string Package::GetReceiveTime() const { return receiveTime_; }
int Package::GetStatus() const { return status_; }
std::string Package::GetDescription() const { return description_; }
int Package::GetCourierId() const { return courierId_; }

void Package::SetStatus(int status) { status_ = status; }
void Package::SetCourierId(int courierId) { courierId_ = courierId; }
void Package::SetReceiveTime(const std::string& time) { receiveTime_ = time; }

void Package::Sign(const std::string& receiveTime)
{
    if (status_ == 1) {
        status_ = 2;
        receiveTime_ = receiveTime;
    }
}

bool Package::IsWaitingCollect() const { return status_ == 0; }
bool Package::IsWaitingSign() const { return status_ == 1; }
bool Package::IsSigned() const { return status_ == 2; }
