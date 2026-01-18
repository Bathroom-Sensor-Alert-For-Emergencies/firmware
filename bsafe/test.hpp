#include "esp_rtc_time.h"
#include <Arduino.h>
#include <HardwareSerial.h>

HardwareSerial uart(0);
const uint32_t rst = D5;

void setup() {
    Serial.begin(115200);

    uart.begin(115200);
    delay(2000);
    Serial.println("Initialized serial and uart");

    pinMode(rst, OUTPUT);
    digitalWrite(rst, LOW);
    delay(100);
    digitalWrite(rst, HIGH);
    delay(3000);
    Serial.println("Reset LoRa module");

    while (uart.available() > 0) uart.read();
    uart.printf("AT\r\n");
    uart.flush();
    delay(2000);

    if (uart.available() <= 0) {
        Serial.println("No response from LoRa module");
    }

    while (uart.available() > 0) {
        String resp = uart.readString();
        Serial.printf("Got response '%s'\n", resp.c_str());
        // uint8_t byte = uart.read();
        // Serial.printf("0x%02X '%c'\n", byte, isprint(byte) ? byte : '.');
    }
}

void loop() {
    static int i = 0;
    uart.printf("AT+SEND=0,19,Hello, world! (%03d)\r\n", i);
    delay(1000);
    Serial.printf("Sent (%d), ", i);

    auto start = millis();
    while (millis() < (start + 5000)) {
        if (uart.available() > 0) {
            String resp = uart.readString();
            Serial.printf("got response '%s'\n", resp.c_str());
            goto end;
        }
    }

    Serial.println("no response from module");


end:
    i++;
    delay(500);
}
