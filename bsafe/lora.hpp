#pragma once

#include "communicator.hpp"
#include <SoftwareSerial.h>
#include <cstdint>
#include <Arduino.h>

class LoRa : public Communicator {
public:
    enum Error {
        Success = 0,
        MissingEnter = 1,
        MissingAT = 2,
        MissingEquals = 3,
        UnknownCommand = 4,
        TxTimeout = 10,
        RxTimeout = 11,
        CrcError = 12,
        TxDataTooLong = 13,
        UnknownError = 15,
        NoResponse = 253, // Defined by us
        OtherResponse = 254,
        InvalidResponse = 255,
    };

    LoRa(std::uint32_t uart_num, std::uint32_t reset, std::uint32_t rx, std::uint32_t tx);

    bool begin() override;
    void startRecv() override;
    bool getPacket(Packet* packet) override;
    bool send(std::uint8_t* data, std::size_t len) override;
    bool recv(std::uint8_t* data, std::size_t len) override;
    Error getResponse(String* resp = nullptr, unsigned long timeout = 0);

private:
    HardwareSerial uart;
    std::uint32_t reset;
    std::uint32_t rx;
    std::uint32_t tx;
};
