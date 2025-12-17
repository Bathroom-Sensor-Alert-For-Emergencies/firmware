#pragma once

#include <cstdint>
#include <Arduino.h>
// #include <SoftwareSerial.h>

class Detector {
public:
    Detector(std::uint32_t otz, std::uint32_t tx, std::uint32_t rx);

    bool begin();
    void update();
    bool isUnresponsive();

private:
    HardwareSerial serial;
    // EspSoftwareSerial::UART serial;
    bool detected;
    std::uint32_t otz;
    std::uint32_t tx;
    std::uint32_t rx;

    void sendHexData(String hexString);
};
