#include "lora.hpp"
#include <cstring>
#include <cstdio>
#include <Arduino.h>

LoRa::LoRa(std::uint32_t uart_num, std::uint32_t reset)
    : Communicator(), reset(reset), uart(uart_num) {}

bool LoRa::begin() {
    // Reset module
    pinMode(this->reset, OUTPUT);
    digitalWrite(this->reset, LOW);
    delay(100);
    digitalWrite(this->reset, HIGH);

    this->uart.begin(115200);
    while (!this->uart) {}
    return true;
}

bool LoRa::send(std::uint8_t* data, std::size_t len) {
    this->uart.printf("AT+SEND=0,%ld,%.*s\r\n", len, len, data);
    LoRa::Error err = this->getResponse();
    if (err == LoRa::Success) {
        return true;
    } else {
        Serial.printf("Error sending SEND command: %d\n", err);
        return false;
    }
}

bool LoRa::recv(std::uint8_t* data, std::size_t len) {
    if (this->uart.available() <= 0) return false;
    String resp;
    LoRa::Error err = this->getResponse(&resp);
    if (err != LoRa::OtherResponse) {
        Serial.printf("Receive error: %d\n", err);
        return false;
    }

    size_t read_len{};
    size_t offset{};
    std::sscanf(resp.c_str(), "+RCV=%*d,%d,%n", &read_len, &offset);
    if (read_len != len) {
        Serial.printf("Expected %d bytes but only got %d\n", len, read_len);
        return false;
    }
    std::memcpy(data, (uint8_t*)&resp.c_str()[offset], len);
    return true;
}

void LoRa::startRecv() {
    this->uart.printf("AT+MODE=1\r\n");
    LoRa::Error err = this->getResponse();
    if (err != LoRa::Success) {
        Serial.printf("Error setting sleep mode: %d\n", err);
    }
}

bool LoRa::getPacket(Packet* packet) {
    return this->recv((std::uint8_t*)packet, sizeof(*packet));
}

LoRa::Error LoRa::getResponse(String* out) {
    if (!this->uart.available()) return LoRa::NoResponse;

    String resp = this->uart.readString();
    const char* ptr = resp.c_str();
    if (resp[1] == 'O') {
        if (resp.equals("+OK\r\n")) return LoRa::Success;
    } else if (ptr[1] == 'E') {
        if (resp.startsWith("+ERR=")) {
            LoRa::Error err = (LoRa::Error)atoi(&ptr[5]);
            return err;
        }
    } else {
        if (out) *out = std::move(resp);
        return LoRa::OtherResponse;
    }

    return LoRa::InvalidResponse;
}
