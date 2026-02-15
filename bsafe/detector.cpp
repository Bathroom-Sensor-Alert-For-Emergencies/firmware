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

    this->hu.configLEDLight(this->hu.eFALLLed, 1);        // Set HP LED switch, it will not light up even if the sensor detects a person present when set to 0.
    this->hu.configLEDLight(this->hu.eHPLed, 1);          // Set FALL LED switch, it will not light up even if the sensor detects a person falling when set to 0.
    this->hu.dmInstallHeight(281);                        // Set installation height, it needs to be set according to the actual height of the surface from the sensor, unit: CM.
    this->hu.dmFallTime(1);                               // Set fall time, the sensor needs to delay the current set time after detecting a person falling before outputting the detected fall, this can avoid false triggering, unit: seconds.
    this->hu.dmUnmannedTime(1);                           // Set unattended time, when a person leaves the sensor detection range, the sensor delays a period of time before outputting a no person status, unit: seconds.
    this->hu.dmFallConfig(this->hu.eResidenceTime, 200);  // Set dwell time, when a person remains still within the sensor detection range for more than the set time, the sensor outputs a stationary dwell status. Unit: seconds.
    this->hu.dmFallConfig(this->hu.eFallSensitivityC, 3); // Set fall sensitivity, range 0~3, the larger the value, the more sensitive.
    this->hu.sensorRet();                                 // Module reset, must perform sensorRet after setting data, otherwise the sensor may not be usable.
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

    if (state != Idle) {
        Serial.printf("%ld,%d\n", millis(), energy);
    }
}

bool Detector::shouldWarn() {
    return this->state == Detector::State::UnconsciousWarned;
}

bool Detector::shouldAlarm() {
    return this->state == Detector::State::Alarm;
}
