#include "lora.hpp"
#include <cstring>
#include <cstdio>
#include <Arduino.h>

LoRa::LoRa(std::uint32_t uart_num, std::uint32_t reset, std::uint32_t rx, std::uint32_t tx)
    : Communicator(), reset(reset), rx(rx), tx(tx), uart(uart_num) {}

bool LoRa::begin() {
    // Reset module
    pinMode(this->reset, OUTPUT);
    digitalWrite(this->reset, LOW);
    delay(100);
    digitalWrite(this->reset, HIGH);
    Serial.println("(begin) Reset LoRa module");

    this->uart.begin(115200, SERIAL_8N1, this->rx, this->tx);
    Serial.println("(begin) Started LoRa UART");
    while (!this->uart) {}
    // Clear any messages
    // TODO: Check for +READY
    uart.printf("AT\r\n");
    delay(1000);
    Serial.printf("(begin) Received:");
    while (this->uart.available() > 0) Serial.write(this->uart.read());
    Serial.printf("\n");
    return true;
}

bool LoRa::send(std::uint8_t* data, std::size_t len) {
    // this->uart.printf("AT+SEND=0,%ld,%.*s\r\n", len, len, data);
    this->uart.printf("AT+SEND=0,%ld,", 2 * len);
    // Serial.printf("(send) sending 'AT+SEND=0,%ld,", 2 * len);
    for (std::size_t i = 0; i < len; i++) {
        this->uart.printf("%02X", data[i]);
        // Serial.printf("%02X", data[i]);
    }
    this->uart.printf("\r\n");
    // Serial.printf("'\n");

    LoRa::Error err = this->getResponse(nullptr, 1000);
    if (err == LoRa::Success) {
        return true;
    } else {
        Serial.printf("(send) got error: %d\n", err);
        return false;
    }
}

static std::uint8_t fromHex(char c) {
    return c >= '0' && c <= '9' ? c - '0' : c - 'A';
}

bool LoRa::recv(std::uint8_t* data, std::size_t len) {
    String resp;
    LoRa::Error err = this->getResponse(&resp);
    if (err == LoRa::NoResponse) return false;
    if (err != LoRa::OtherResponse) {
        Serial.printf("Receive error: %d\n", err);
        return false;
    }

    // Get length and data offset from packet
    size_t read_len{};
    size_t offset{};
    std::sscanf(resp.c_str(), "+RCV=%*d,%d,%n", &read_len, &offset);
    if (read_len != 2 * len) {
        Serial.printf("(recv) Expected %d bytes but only got %d\n", 2 * len, read_len);
        return false;
    }

    // Decode hex
    std::uint8_t* base = (std::uint8_t*)&resp.c_str()[offset];
    for (std::size_t i = 0; i < len; i++) {
        data[i] = (fromHex(base[2 * i]) << 4) + fromHex(base[2 * i + 1]);
    }

    return true;
}

void LoRa::startRecv() {
    this->uart.printf("AT+MODE=1\r\n");
    LoRa::Error err = this->getResponse();
    if (err != LoRa::Success) {
        Serial.printf("(startRecv) Error setting sleep mode: %d\n", err);
    }
}

bool LoRa::getPacket(Packet* packet) {
    return this->recv((std::uint8_t*)packet, sizeof(*packet));
}

LoRa::Error LoRa::getResponse(String* out, unsigned long timeout) {
    unsigned long start = millis();
    while (this->uart.available() <= 0 && millis() - start < timeout) {}
    if (this->uart.available() <= 0) return LoRa::NoResponse;

    String resp = this->uart.readString();
    const char* str = resp.c_str();
    // Serial.printf("(getResponse) got response '%s'\n", str);
    if (str[1] == 'O') {
        if (resp.equals("+OK\r\n")) return LoRa::Success;
    } else if (str[1] == 'E') {
        if (resp.startsWith("+ERR=")) {
            LoRa::Error err = (LoRa::Error)atoi(&str[5]);
            return err;
        }
    } else {
        if (out) *out = std::move(resp);
        return LoRa::OtherResponse;
    }

    return LoRa::InvalidResponse;
}
