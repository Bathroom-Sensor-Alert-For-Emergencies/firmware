#pragma once
#include "DFRobot_HumanDetection.h"
#include <cstdint>

class Detector {
public:
    Detector(std::uint8_t uart_num, std::uint32_t rx, std::uint32_t tx);

    bool begin();
    void update();
    bool shouldWarn();
    bool shouldAlarm();

private:
    enum class State {
        Idle,
        Active,
        Unconscious,
        UnconsciousWarned,
        Alarm,
    };

    HardwareSerial uart;
    DFRobot_HumanDetection hu;
    std::uint32_t rx;
    std::uint32_t tx;
    State state;
    int timer;
};
