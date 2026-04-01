#include "detector.hpp"
#include "constants.hpp"
#include <Arduino.h>

Detector::Detector(std::uint8_t uart_num, std::uint32_t rx, std::uint32_t tx)
  : uart{uart_num}, hu{&uart}, rx{rx}, tx{tx}, state{Detector::State::Idle}, timer{0} {}

bool Detector::begin() {
    this->uart.begin(115200, SERIAL_8N1, this->rx, this->tx);
    while (this->hu.begin() != 0) delay(100);
    Serial.println("(begin) Initialized sensor");

    while (this->hu.configWorkMode(this->hu.eFallingMode) != 0) delay(100);
    Serial.println("(begin) Configured work mode");

    this->hu.sensorRet(); // Module reset, must run after setting data, otherwise the sensor may not be usable

    return true;
}

void Detector::update() {
    bool present = this->hu.smHumanData(this->hu.eHumanPresence);
    int energy = this->hu.smHumanData(this->hu.eHumanMovingRange);
    using enum Detector::State;
    switch (this->state) {
        case Idle:
            if (present) {
                this->state = Active;
                Serial.println("(detector) presence detected, switching to active state");
            }
            break;
        case Active:
            if (!present) {
                this->state = Idle;
                Serial.println("(detector) no presence detected, switching to idle state");
            } else if (energy <= ENERGY_UNCONSCIOUS_THRESHOLD) {
                this->state = Unconscious;
                this->timer = millis();
                Serial.println("(detector) low energy, switching to unconscious state");
            }
            break;
        case Unconscious:
            if (!present) {
                this->state = Idle;
                Serial.println("(detector) no presence detected, switching to idle state");
            } else if (energy > ENERGY_UNCONSCIOUS_THRESHOLD) {
                this->state = Active;
                Serial.println("(detector) high energy, switching to active state");
            } else if (millis() - this->timer > TIME_TO_UNCONSCIOUS_WARN_MS) {
                this->state = UnconsciousWarned;
                Serial.println("(detector) met threshold, switching to alarm state");
            }
            break;
        case UnconsciousWarned:
            if (!present) {
                this->state = Idle;
            } else if (energy > ENERGY_UNCONSCIOUS_THRESHOLD) {
                this->state = Active;
            } else if (millis() - this->timer > TIME_TO_UNCONSCIOUS_ALARM_MS) {
                this->state = Alarm;
                this->timer = millis();
            }
            break;
        case Alarm:
            if (!present) {
                this->state = Idle;
                Serial.println("(detector) no presence detected, switching to idle state");
            } else if (energy > ENERGY_UNCONSCIOUS_THRESHOLD) {
                this->state = Active;
                Serial.println("(detector) high energy, switching to active state");
            }
            break;
    }

    // if (state != Idle) {
    //     Serial.printf("%ld,%d\n", millis(), energy);
    // }
}

bool Detector::shouldWarn() {
    return this->state == Detector::State::UnconsciousWarned;
}

bool Detector::shouldAlarm() {
    return this->state == Detector::State::Alarm;
}
