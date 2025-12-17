#pragma once
#include "communicator.hpp"
#include "radio.hpp"
#include <Arduino.h>

const uint32_t cs = D6;

#if defined(SENSOR)
const uint32_t gdo0 = D0;
const uint32_t gdo2 = D1;
const uint32_t otz = D3;
const uint32_t tx = D4;
const uint32_t rx = D5;
#elif defined(RECEIVER)
const uint32_t gdo0 = D5;
const uint32_t gdo2 = D4;
#else
    #error "Must define SENSOR or RECEIVER"
#endif

Radio radio{cs, gdo0, gdo2};
Communicator& comm = radio;

#ifdef RECEIVER
const uint32_t ack_button = D0;
const uint32_t alarm_led = D1;
#endif

const unsigned long HEARTBEAT_PERIOD_MS = 5000;
const unsigned long HEARTBEAT_MAX_PERIOD_MS = 10000;
const unsigned long ALARM_RETRY_PERIOD_MS = 3000;
const unsigned long PAIRING_PERIOD_MS = 10000;
const unsigned long LED_BLINK_PERIOD_MS = 500;
