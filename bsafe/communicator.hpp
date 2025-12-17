#pragma once

#include "packet.hpp"
#include <cstdint>

class Communicator {
public:
    DeviceID id;

    Communicator() : id{0} {}

    virtual bool begin();
    bool alarm();
    bool ackAlarm(DeviceID id);
    bool lowPower();
    bool pairSensor();
    bool pairReceiver();
    bool pairResponse();
    bool heartbeat();
    bool disconnect();
    bool sendPacket(Packet packet);
    bool recvPacket(Packet* packet);
    virtual void startRecv() = 0;
    virtual bool getPacket(Packet* packet) = 0;
private:
    virtual bool send(std::uint8_t* data, std::size_t len) = 0;
    virtual bool recv(std::uint8_t* data, std::size_t len) = 0;
};
