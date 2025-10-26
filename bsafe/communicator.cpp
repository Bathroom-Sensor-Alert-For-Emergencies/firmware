#pragma once

#include "packet.hpp"
#include "communicator.hpp"

using enum PacketType;

bool Communicator::alarm() {
    return sendPacket(Packet{alarm, this->id});
}

bool Communicator::ackAlarm(DeviceID id) {
    return sendPacket(Packet{ack_alarm, id});
}

bool Communicator::lowPower() {
    return sendPacket(Packet{low_power, this->id});
}

bool Communicator::pair() {
    return sendPacket(Packet{pair, this->id});
}

bool Communicator::heartbeat() {
    return sendPacket(Packet{heartbeat, this->id});
}

bool Communicator::sendPacket(Packet packet) {
    return send((std::uint8_t*)&packet, sizeof(packet));
}

bool Communicator::recvPacket(Packet* packet) {
    return recv((std::uint8_t*)packet, sizeof(*packet));
}
