#include "comm.hpp"
#include <Arduino.h>

unsigned long last_heartbeat_ms = 0;
unsigned long last_alarm_ms = 0;
unsigned long pairing_start = 0;
std::uint8_t new_id = 0;

enum class State {
    Pairing,
    Idle,
    Detected,
    Alarmed,
};

State state = State::Pairing;

bool detectMovement() {
    // TODO: Placeholder to simulate 0.1% chance
    return random(1000) == 999;
}

void updateState() {
    using enum State;
    switch (state) {
        case Pairing:
            if (millis() > pairing_start + PAIRING_PERIOD_MS) {
                comm.id = new_id;
                state = Idle;
            }
            break;
        case Idle:
            if (detectMovement()) state = Detected;
            break;
        case Detected: // TODO: Do I really need this state??
            comm.alarm();
            last_alarm_ms = millis();
            state = Alarmed;
            break;
        case Alarmed:
            if (millis() >= last_alarm_ms + ALARM_RETRY_PERIOD_MS) {
                comm.alarm();
                last_alarm_ms = millis();
            }
            break;
    }
}

void handlePacket(Packet packet) {
    using enum PacketType;
    using enum State;
    switch (packet.type) {
        case AckAlarm:
            if (packet.id == comm.id && state == Alarmed) state = Idle;
            break;
        case PairSensor:
        case PairReceiver:
            comm.pairResponse();
            break;
        case PairResponse:
            if (state == Pairing) {
                new_id = new_id > packet.id + 1 ? new_id : packet.id + 1; // Our ID must be 1 greater than the maximum ID in the network
            }
            break;
        case Alarm: // Ignore these packets
        case LowPower:
        case Heartbeat:
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

    comm.pairSensor(); // After turning on, request to pair
    pairing_start = millis();
    comm.listen(handlePacket);

    return;
err:
    while (true) delay(100);
}

void loop() {
    if (state != State::Pairing && millis() >= last_heartbeat_ms + HEARTBEAT_PERIOD_MS) {
        last_heartbeat_ms = millis();
        comm.heartbeat();
    }

    updateState();
}
