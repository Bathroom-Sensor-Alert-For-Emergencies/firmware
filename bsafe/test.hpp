#include "esp_rtc_time.h"
#include <Arduino.h>
#include <RadioLib.h>

const uint32_t cs = D6;
const uint32_t gdo0 = D5;
const uint32_t gdo2 = D4;
CC1101 radio = new Module(cs, gdo0, RADIOLIB_NC, gdo2);

const unsigned long long CYCLE_TIME_US = 7 * 1000 * 1000;
const unsigned long long WAKE_TIME_US = 250 * 1000;
unsigned long long epoch;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Initialized serial");

    int err = radio.begin();
    Serial.printf("Initialized radio, err = %d\n", err);
}

void loop() {
    if (esp_rtc_get_time_us() > (epoch + CYCLE_TIME_US)) {
        epoch += CYCLE_TIME_US;
    }
    {

        // Send sync packet
        const size_t len = 15; // For hello world message
        uint8_t buf[len];
        buf[0] = 1;
        *(unsigned long long*)&buf[1] = epoch;
        buf[14] = 0;
        int err = radio.transmit(buf, len);
        Serial.printf("Sent sync, epoch = %lld, err = %d\n", epoch, err);

        // Send as many data packets as possible
        unsigned long long start = esp_rtc_get_time_us();
        while (esp_rtc_get_time_us() <= (start + WAKE_TIME_US)) {
            err = radio.transmit((uint8_t*)"\0Hello, world!", len);
            Serial.printf("Sent data, err = %d\n", err);
        }
    }
}
