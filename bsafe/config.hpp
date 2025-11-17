#pragma once
#include "communicator.hpp"
#include "radio.hpp"
#include <Arduino.h>

const uint32_t cs = D6;
const uint32_t gdo0 = D5;
const uint32_t gdo2 = D4;
Radio radio{cs, gdo0, gdo2};
Communicator& comm = radio;

const uint32_t ack_button = D0;
const uint32_t alarm_led = D1;

const unsigned long HEARTBEAT_PERIOD_MS = 1000;
const unsigned long HEARTBEAT_MAX_PERIOD_MS = 2000;
const unsigned long ALARM_RETRY_PERIOD_MS = 1000;
const unsigned long PAIRING_PERIOD_MS = 3000;
const unsigned long LED_BLINK_PERIOD_MS = 500;
