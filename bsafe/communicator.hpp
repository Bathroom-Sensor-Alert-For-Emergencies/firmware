#pragma once

#include "packet.hpp"
#include <cstdint>

class Communicator {
public:
    Communicator() : id{0} {}

    virtual bool begin();
    bool alarm();
    bool ackAlarm(DeviceID id);
    bool lowPower();
    bool pair();
    bool heartbeat();
    bool sendPacket(Packet packet);
    bool recvPacket(Packet* packet);
    virtual void listen(void (*on_recv)(Packet));
private:
    DeviceID id;

    virtual bool send(std::uint8_t* data, std::size_t len) = 0;
    virtual bool recv(std::uint8_t* data, std::size_t len) = 0;
};
