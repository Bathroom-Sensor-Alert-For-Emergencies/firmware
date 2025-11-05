#pragma once
#include "communicator.hpp"
#include "radio.hpp"

const uint32_t cs = D6;
const uint32_t gdo0 = D5;
const uint32_t gdo2 = D4;
Radio radio{cs, gdo0, gdo2};
Communicator& comm = radio;

const unsigned long HEARTBEAT_PERIOD_MS = 1000;
const unsigned long HEARTBEAT_MAX_PERIOD_MS = 2000;
const unsigned long ALARM_RETRY_PERIOD_MS = 1000;
const unsigned long PAIRING_PERIOD_MS = 3000;
