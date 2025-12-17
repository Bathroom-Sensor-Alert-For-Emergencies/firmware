#pragma once

#include "communicator.hpp"
#include <RadioLib.h>
#include <cstdint>

class Radio : public Communicator {
public:
    Radio(std::uint32_t cs, std::uint32_t gdo0, std::uint32_t gdo2);

    bool begin() override;
    void startRecv() override;
    bool getPacket(Packet* packet) override;
private:
    CC1101 radio;

    bool send(std::uint8_t* data, std::size_t len) override;
    bool recv(std::uint8_t* data, std::size_t len) override;
};
