#pragma once

const unsigned long HEARTBEAT_PERIOD_MS = 5 * 1000;
const unsigned long HEARTBEAT_MAX_PERIOD_MS = 15 * 1000;
const unsigned long ALARM_RETRY_PERIOD_MS = 10 * 1000;
const unsigned long PAIRING_PERIOD_MS = 10 * 1000;
const unsigned long LED_BLINK_PERIOD_MS = 500;
const int ENERGY_UNCONSCIOUS_THRESHOLD = 5; // estimate from plot
const unsigned long TIME_TO_UNCONSCIOUS_ALARM_MS = 2 * 60 * 1000;
const unsigned long TIME_TO_UNCONSCIOUS_WARN_MS = 100 * 1000;
