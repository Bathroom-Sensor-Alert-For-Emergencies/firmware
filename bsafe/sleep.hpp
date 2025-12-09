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
const unsigned long long HEARTBEAT_INTERVAL_US = 5 * 1000 * 1000;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Woke up!");

    radio.begin();
    radio.startReceive();

    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("Woke up from timer");
            break;
        case ESP_SLEEP_WAKEUP_VBAT_UNDER_VOLT:
            Serial.println("Woke up from low battery voltage");
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("Woke up from ext0");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            Serial.println("Woke up from ext1");
            // Handle radio
            if (1) {
                // "Hello, world!\0"
                uint8_t buf[14];
                int err = radio.readData(buf, 14);
                buf[13] = 0;
                radio.finishReceive();
                Serial.printf("Received '%s', err = %d\n", buf, err);
                radio.startReceive();
            } else {
                Serial.println("Nothing to receive");
            }
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            Serial.println("Woke up from touchpad");
            break;
        default:
            Serial.println("Woke up from other event");
            break;
    }

    if (esp_rtc_get_time_us() > (last_heartbeat_us + HEARTBEAT_INTERVAL_US)) {
        last_heartbeat_us = esp_rtc_get_time_us();
        Serial.printf("Sending heartbeat at %lld\n", last_heartbeat_us);
    } else {
        Serial.printf("Not sending heartbeat yet, time = %lld, next heartbeat at %lld\n", esp_rtc_get_time_us(), last_heartbeat_us + HEARTBEAT_INTERVAL_US);
    }

    esp_sleep_enable_timer_wakeup(HEARTBEAT_INTERVAL_US);
    esp_sleep_enable_ext1_wakeup(1 << gdo0 | 1 << gdo2, ESP_EXT1_WAKEUP_ANY_HIGH);

    Serial.println("Going to sleep");
    esp_deep_sleep_start();
}

void loop() {}
