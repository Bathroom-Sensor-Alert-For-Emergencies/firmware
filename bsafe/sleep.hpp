#include "driver/rtc_io.h"
#include "esp_rtc_time.h"
#include "esp_sleep.h"
#include <Arduino.h>
#include <RadioLib.h>

const uint32_t cs = D6;
const uint32_t gdo0 = D0;
const uint32_t gdo2 = D1;
CC1101 radio = new Module(cs, gdo0, RADIOLIB_NC, gdo2);

RTC_DATA_ATTR unsigned long long last_heartbeat_us = 0;
RTC_DATA_ATTR unsigned long long epoch = 0;
// RTC_DATA_ATTR bool first_boot = true;
const unsigned long long CYCLE_TIME_US = 7 * 1000 * 1000;
const unsigned long long WAKE_TIME_US = 250 * 1000;
const unsigned long long SLEEP_TIME_US = CYCLE_TIME_US - WAKE_TIME_US;

volatile bool available = false;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.printf("Woke up, epoch = %lld\n", epoch);

    radio.begin();

    // while (true) {
    //     Serial.printf("Receiving: ");
    //     uint8_t buf[15];
    //     int err = radio.receive(buf, 15);
    //     buf[14] = 0;
    //     // radio.finishReceive();
    //     // radio.startReceive();
    //     if (buf[0] == 1) {
    //         // Sync packet
    //         epoch = *(unsigned long long*)&buf[1];
    //         Serial.printf("Syncing, epoch = %lld\n", epoch);
    //     } else {
    //         // Data packet
    //         Serial.printf("Received '%s', err = %d\n", &buf[1], err);
    //     }
    // }
    //

    radio.setPacketReceivedAction([] {
        available = true;
    });

    radio.startReceive();

    unsigned long long last_wake_us = esp_rtc_get_time_us();
    Serial.printf("Sending heartbeat at %lld, offset = %lld\n", last_wake_us, (last_wake_us - epoch) % CYCLE_TIME_US);

    while (esp_rtc_get_time_us() < (last_wake_us + WAKE_TIME_US)) {
        if (available) {
            available = false;

            // "Hello, world!\0"
            const size_t len = 15;
            uint8_t buf[len];
            int err = radio.readData(buf, len);
            buf[len - 1] = 0;
            radio.finishReceive();
            radio.startReceive();
            if (buf[0] == 1) {
                // Sync packet
                epoch = *(unsigned long long*)&buf[1];
                Serial.printf("Syncing, epoch = %lld\n", epoch);
            } else {
                // Data packet
                Serial.printf("Received '%s', err = %d\n", &buf[1], err);
            }
        }
    }

    radio.sleep();
    unsigned long long offset = (esp_rtc_get_time_us() - epoch) % CYCLE_TIME_US;
    if (epoch == 0) {
        epoch = esp_rtc_get_time_us();
        offset = 0;
    }
    esp_sleep_enable_timer_wakeup(CYCLE_TIME_US - offset);
    Serial.printf("Going to sleep, measured offset of %lld, sleeping for %lld\n", offset, CYCLE_TIME_US - offset);
    esp_deep_sleep_start();
}

void loop() {}
