#pragma once
#include <LiquidCrystal.h>
#include <deque>
#include <cstdint>
#include "alarm.hpp"
#include "packet.hpp"

class LCD {
public:
    LCD(int rs, int en, int d4, int d5, int d6, int d7);

    void begin();
    void clear();
    void pair();
    void emplace(AlarmType type, DeviceID id);
    void enqueue(Alarm alarm);
    void dequeue();

private:
    LiquidCrystal lcd;
    std::deque<Alarm> alarms;
};
