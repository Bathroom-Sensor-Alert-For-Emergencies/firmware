#include "radio.hpp"
#include "packet.hpp"
#include <Arduino.h>

Radio::Radio(std::uint32_t cs, std::uint32_t gdo0, std::uint32_t gdo2)
    : Communicator(),
      radio{new Module(cs, gdo0, RADIOLIB_NC, gdo2)} {}

bool Radio::begin() {
    int err = this->radio.begin();
    if (err != RADIOLIB_ERR_NONE) {
        Serial.println("Error initializing radio");
        return false;
    }

    err = this->radio.setPreambleLength(128);
    if (err != RADIOLIB_ERR_NONE) {
        Serial.println("Error setting preamble length");
        return false;
    }

    return true;
}

bool Radio::send(std::uint8_t* data, std::size_t len) {
    int err = this->radio.transmit(data, len);
    if (err != RADIOLIB_ERR_NONE) {
        Serial.printf("Error sending data: %d\n", err);
        return false;
    }

    return true;
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
    if (err != RADIOLIB_ERR_NONE) {
        Serial.printf("Error reading data: %d\n", err);
        return false;
    }

    err = this->radio.finishReceive();
    if (err != RADIOLIB_ERR_NONE) {
        Serial.printf("Error cleaning up: %d\n", err);
    }
    return err == RADIOLIB_ERR_NONE;
}
