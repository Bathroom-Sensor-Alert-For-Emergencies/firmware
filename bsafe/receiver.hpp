#include "comm.hpp"
#include <Arduino.h>

enum class State {
    Pairing,
    Idle,
    Alarmed,
};

unsigned long pairing_start = 0;
std::uint8_t num_sensors = 0;
unsigned long last_heartbeats[256];
// Stack of alarms
std::uint8_t alarm_i = 0;
std::uint8_t alarms[256];

State state = State::Pairing;

void updateState() {
    using enum State;
    switch (state) {
        case Pairing:
            if (millis() > pairing_start + PAIRING_PERIOD_MS) {
                state = Idle;
            }
            break;
        case Idle:
            for (std::uint8_t i = 0; i < num_sensors; i++) {
                if (millis() > last_heartbeats[i] + HEARTBEAT_MAX_PERIOD_MS) {
                    // TODO: Add heartbeat alarm to alarm stack
                }
            }
            break;
        case Alarmed:
            // TODO: Make buzzer beep and light flash
            break;
    }
}

void handlePacket(Packet packet) {
    using enum PacketType;
    using enum State;
    switch (packet.type) {
        case Alarm:
            if (packet.id == comm.id && state == Alarmed) state = Idle;
            break;
        case AckAlarm:
            // TODO: Remove sensor id from alarm stack
            break;
        case LowPower:
            // TODO: Add low power alarm to alarm stack
            break;
        case PairSensor:
            num_sensors++;
            break;
        case Heartbeat:
            last_heartbeats[packet.id] = millis();
            break;
        case PairReceiver: // Ignore these packets
        case PairResponse:
            break;
        default: // TODO: Test bad packet type
            Serial.printf("Unknown packet type (%d) received\n", packet.type);
            break;
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    if (!comm.begin()) {
        Serial.println("Error initializing radio");
        goto err;
    }

    comm.pairReceiver(); // After turning on, request to pair
    pairing_start = millis();
    comm.listen(handlePacket);

    return;
err:
    while (true) delay(100);
}

void loop() {
    updateState();
}
