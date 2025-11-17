#include "radio.hpp"
#include "packet.hpp"
#include <Arduino.h>

Radio::Radio(std::uint32_t cs, std::uint32_t gdo0, std::uint32_t gdo2)
    : Communicator(),
      radio{new Module(cs, gdo0, RADIOLIB_NC, gdo2)} {}

bool Radio::begin() {
    int err = this->radio.begin();
    return err == RADIOLIB_ERR_NONE;
}

bool Radio::send(std::uint8_t* data, std::size_t len) {
    return this->radio.transmit(data, len) == RADIOLIB_ERR_NONE;
}

bool Radio::recv(std::uint8_t* data, std::size_t len) {
    return this->radio.receive(data, len) == RADIOLIB_ERR_NONE;
}

volatile bool ready_flag = false;

void Radio::startRecv() {
    this->radio.setPacketReceivedAction([] {
        ready_flag = true;
    });
    int err = this->radio.startReceive();
    if (err != RADIOLIB_ERR_NONE) Serial.printf("Error starting receive: %d\n", err);
}

bool Radio::getPacket(Packet* packet) {
    if (!ready_flag) return false;

    ready_flag = false;
    int err = this->radio.readData((std::uint8_t*)packet, sizeof(*packet));
    if (err != RADIOLIB_ERR_NONE) Serial.printf("Error reading data: %d\n", err);
    this->radio.finishReceive();
    return err != RADIOLIB_ERR_NONE;
}

void Radio::listen(void (*on_recv)(Packet)) {
    Radio::current = this;
    Radio::on_recv = on_recv;
    this->radio.setPacketReceivedAction([] {
        Packet packet;
        // TODO: Error handling?
        Radio::current->radio.readData((std::uint8_t*)&packet, sizeof(packet));
        Radio::current->radio.finishReceive();
        Radio::on_recv(packet);
        Radio::current->radio.startReceive();
    });
    this->radio.startReceive();
    // TODO: Low power listening
    // this->radio.sleep();
}
