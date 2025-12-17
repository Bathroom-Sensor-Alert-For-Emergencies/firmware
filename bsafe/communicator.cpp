#pragma once

#include "packet.hpp"
#include "communicator.hpp"

using enum PacketType;

bool Communicator::alarm() {
    return sendPacket(Packet{Alarm, this->id});
}

bool Communicator::ackAlarm(DeviceID id) {
    return sendPacket(Packet{AckAlarm, id});
}

bool Communicator::lowPower() {
    return sendPacket(Packet{LowPower, this->id});
}

bool Communicator::pairSensor() {
    return sendPacket(Packet{PairSensor, this->id});
}

bool Communicator::pairReceiver() {
    return sendPacket(Packet{PairReceiver, this->id});
}

bool Communicator::pairResponse() {
    return sendPacket(Packet{PairResponse, this->id});
}

bool Communicator::heartbeat() {
    return sendPacket(Packet{Heartbeat, this->id});
}

bool Communicator::disconnect() {
    return sendPacket(Packet{Disconnect, this->id});
}

bool Communicator::sendPacket(Packet packet) {
    return send((std::uint8_t*)&packet, sizeof(packet));
}

bool Communicator::recvPacket(Packet* packet) {
    return recv((std::uint8_t*)packet, sizeof(*packet));
}
