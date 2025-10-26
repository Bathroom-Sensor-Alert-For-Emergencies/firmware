#pragma once
#include <cstdint>

enum class PacketType : std::uint8_t {
    alarm,
    ack_alarm,
    low_power,
    pair,
    heartbeat,
};

using DeviceID = std::uint8_t;

struct Packet {
    PacketType type{};
    DeviceID id{};

    Packet() = default;
    Packet(PacketType type, DeviceID id) : type{type}, id{id} {}
};
