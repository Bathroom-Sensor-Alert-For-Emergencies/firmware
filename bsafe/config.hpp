#pragma once
#include "communicator.hpp"
#include "lora.hpp"
#include "lcd.hpp"

#if defined(SENSOR)
// TODO: New sensor pins
const uint32_t otz = D3;
const uint32_t tx = D4;
const uint32_t rx = D5;
// Detector detector{otz, tx, rx};
#elif defined(RECEIVER)
const uint32_t ack_button = 19;
const uint32_t pair_led = 18;
const uint32_t low_power_led = 5;
const uint32_t speaker = 6;
const int speaker_freq = 440; // A4
const int speaker_duration = 1000; // 1 s
LCD lcd(12, 13, 14, 27, 26, 25);
LoRa lora{0, 23};
Communicator& comm = lora;
#else
    #error "Must define SENSOR or RECEIVER"
#endif

const unsigned long HEARTBEAT_PERIOD_MS = 5000;
const unsigned long HEARTBEAT_MAX_PERIOD_MS = 10000;
const unsigned long ALARM_RETRY_PERIOD_MS = 3000;
const unsigned long PAIRING_PERIOD_MS = 10000;
const unsigned long LED_BLINK_PERIOD_MS = 500;
