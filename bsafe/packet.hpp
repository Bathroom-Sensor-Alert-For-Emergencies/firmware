#pragma once
#include <cstdint>

enum class PacketType : std::uint8_t {
    // Invalid = 0,
    Alarm = 1, // Sensor notifies receivers when it detects an unresponsive client
    AckAlarm = 2, // Receiver acknowledges an alarm
    LowPower = 3, // Sensor notifies receivers of low power
    PairSensor = 4, // Sensor alerts all nodes when joining network, requests responses from all sensors. Ignore id field
    PairReceiver = 5, // Receiver requests responses from all sensors. Ignore id field
    PairResponse = 6, // Sensor response to pair packet
    Heartbeat = 7, // Sensor sends periodically. Receivers display error if didn't receive from all sensors
    Disconnect = 8, // Sensors must send before turning off to prevent erroneous alarms
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
