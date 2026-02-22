#pragma once
#include "constants.hpp"
#include "detector.hpp"
#include "lcd.hpp"
#include "lora.hpp"
#include "communicator.hpp"

#if defined(SENSOR)
    // TODO: New sensor pins
    Detector detector{2, 16, 17};
    LoRa lora{1, 13, 25, 26}; // For big thicc board
    // LoRa lora{1, 23, 14, 12}; // For small itty bitty board
    Communicator& comm = lora;
    const uint32_t speaker = 4;
    const int speaker_freq = 440; // A4
    const int speaker_duration_ms = 1000; // 1 s
#elif defined(RECEIVER)
    const uint32_t ack_button = 19;
    const uint32_t pair_led = 18;
    const uint32_t low_power_led = 5;
    const uint32_t speaker = 4;
    const int speaker_freq = 440; // A4
    LCD lcd(12, 13, 14, 27, 26, 25);
    LoRa lora{2, 23, 16, 17};
    Communicator& comm = lora;
#else
    #error "Must define SENSOR or RECEIVER"
#endif
