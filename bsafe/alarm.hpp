#pragma once
#include "packet.hpp"

enum class AlarmType {
    Heartbeat,
    Alarm,
    LowPower,
};

struct Alarm {
    AlarmType type{};
    DeviceID id{};

    Alarm(AlarmType type, DeviceID id) : type{type}, id{id} {}

    const char* name() {
        using enum AlarmType;
        switch (this->type) {
            case Heartbeat:
                return "Disconnect";
            case Alarm:
                return "Alarm";
            case LowPower:
                return "Low power";
        }
    }
};
