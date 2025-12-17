#include <Arduino.h>

const uint32_t power_signal_pin = D2; // Pin to signal that we should power off
const uint32_t power_off_pin = D3; // Pin to actually power off

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("Started");

    pinMode(power_signal_pin, INPUT);
    pinMode(power_off_pin, OUTPUT);
    digitalWrite(power_signal_pin, LOW);

    Serial.println("Initialized pins");
}

void loop() {
    if (digitalRead(power_signal_pin) == HIGH) {
        Serial.println("Received power off signal");
        delay(1000);
        Serial.println("Powering off");
        digitalWrite(power_off_pin, HIGH);

        while (true) delay(10);
    }
}
