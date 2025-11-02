#pragma once
#include <cstdint>

enum class PacketType : std::uint8_t {
    Alarm,
    AckAlarm,
    LowPower,
    Pair,
    Heartbeat,
};

using DeviceID = std::uint8_t;

struct Packet {
    PacketType type{};
    DeviceID id{};

    Packet() = default;
    Packet(PacketType type, DeviceID id) : type{type}, id{id} {}
};
