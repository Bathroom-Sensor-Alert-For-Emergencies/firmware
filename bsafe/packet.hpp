#pragma once
#include <cstdint>

enum class PacketType : std::uint8_t {
    // Invalid = 0,
    Alarm, // Sensor notifies receivers when it detects an unresponsive client
    AckAlarm, // Receiver acknowledges an alarm
    LowPower, // Sensor notifies receivers of low power
    PairSensor, // Sensor alerts all nodes when joining network, requests responses from all sensors. Ignore id field
    PairReceiver, // Receiver requests responses from all sensors. Ignore id field
    PairResponse, // Sensor response to pair packet
    Heartbeat, // Sensor sends periodically. Receivers display error if didn't receive from all sensors
    Disconnect, // Sensors must send before turning off to prevent erroneous alarms
};

// Maximum of 256 sensors
using DeviceID = std::uint8_t;

struct Packet {
    PacketType type{};
    DeviceID id{};

    Packet() = default;
    Packet(PacketType type, DeviceID id) : type{type}, id{id} {}

    const char* name() {
        using enum PacketType;
        switch (this->type) {
            case Alarm:
                return "Alarm";
            case AckAlarm:
                return "AckAlarm";
            case LowPower:
                return "LowPower";
            case PairSensor:
                return "PairSensor";
            case PairReceiver:
                return "PairReceiver";
            case PairResponse:
                return "PairResponse";
            case Heartbeat:
                return "Heartbeat";
            case Disconnect:
                return "Disconnect";
            default:
                return "Invalid";
        }
    }
};
