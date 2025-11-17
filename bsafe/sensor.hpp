#include "config.hpp"
#include <Arduino.h>

enum class State {
    Pairing,
    Idle,
    Detected,
    Alarmed,
};

const char* stateName(State s) {
    using enum State;
    switch (s) {
        case Pairing:
            return "pairing";
        case Idle:
            return "idle";
        case Detected:
            return "detected";
        case Alarmed:
            return "alarmed";
    }
}

unsigned long last_heartbeat_ms = 0;
unsigned long last_alarm_ms = 0;
unsigned long pairing_start = 0;
std::uint8_t new_id = 0;

State state = State::Pairing;

bool detectUnresponsive() {
    // TODO: Placeholder to simulate a small chance
    return random(10000) == 0;
}

void updateState() {
    using enum State;

    // Send heartbeat
    if (state != Pairing && millis() >= last_heartbeat_ms + HEARTBEAT_PERIOD_MS) {
        last_heartbeat_ms = millis();
        comm.heartbeat();
        Serial.println("Sent heartbeat");
    }

    switch (state) {
        case Pairing:
            if (millis() > pairing_start + PAIRING_PERIOD_MS) {
                comm.id = new_id;
                state = Idle;
                Serial.printf("Done pairing, got id %d\n", new_id);
            }
            break;
        case Idle:
            if (detectUnresponsive()) state = Detected;
            break;
        case Detected: // TODO: Do I really need this state??
            comm.alarm();
            last_alarm_ms = millis();
            state = Alarmed;
            Serial.printf("Alarmed!\n");
            break;
        case Alarmed:
            if (millis() >= last_alarm_ms + ALARM_RETRY_PERIOD_MS) {
                comm.alarm();
                Serial.printf("Retried alarm\n");
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
            if (packet.id == comm.id && state == Alarmed) {
                state = Idle;
                Serial.printf("Node %d acknowledged alarm\n", packet.id);
            } else {
                Serial.printf("Noticed alarm acknowledgement for node %d\n", packet.id);
            }
            break;
        case PairSensor:
        case PairReceiver:
            Serial.printf("Received pair request from node %d\n", packet.id);
            comm.pairResponse();
            break;
        case PairResponse:
            Serial.printf("Received pair response from node %d\n", packet.id);
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
    // comm.listen(handlePacket);
    comm.startRecv();

    return;
err:
    while (true) delay(100);
}

void loop() {
    if (millis() % 1000 == 0) {
        Serial.printf("In %s state\n", stateName(state));
        delay(2);
    }

    Packet packet;
    if (comm.getPacket(&packet)) {
        handlePacket(packet);
        updateState();
        comm.startRecv();
    } else {
        updateState();
    }
}
