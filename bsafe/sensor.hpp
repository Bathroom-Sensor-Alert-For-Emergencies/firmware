#include "config.hpp"
#include "detector.hpp"
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

State state = State::Pairing;

void setWarn(bool enable) {
    static bool enabled = false;
    if (enable && !enabled) {
        enabled = true;
        tone(speaker, speaker_freq, speaker_duration_ms);
    } else if (!enable && enabled){
        enabled = false;
        noTone(speaker);
    }
}

void updateState() {
    using enum State;

    // Send heartbeat
    if (state != Pairing && state != Alarmed && millis() - last_heartbeat_ms > HEARTBEAT_PERIOD_MS) {
        last_heartbeat_ms = millis();
        comm.heartbeat();
        Serial.println("Sent heartbeat");
    }

    // Read motion detector data
    detector.update();

    switch (state) {
        case Pairing:
            if (millis() - pairing_start > PAIRING_PERIOD_MS) {
                state = Idle;
                Serial.printf("Done pairing, id %s (%d)\n", ID_STRING(comm.id), comm.id);
            }
            break;
        case Idle:
            if (detector.shouldWarn()) {
            // if (random(1000000) == 0) { // Mock
                state = Detected;
                Serial.println("Warned!");
            }
            break;
        case Detected:
            setWarn(true);
            if (detector.shouldAlarm()) {
                comm.alarm();
                last_alarm_ms = millis();
                state = Alarmed;
                Serial.println("Alarmed!");
            } else if (!detector.shouldWarn()) {
                state = Idle;
                Serial.println("Detected movement, back to idle state");
            }
            break;
        case Alarmed:
            setWarn(false);
            if (millis() >= last_alarm_ms + ALARM_RETRY_PERIOD_MS) {
                comm.alarm();
                Serial.println("Retried alarm");
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
                detector.acknowledge();
                Serial.printf("Node %s acknowledged alarm\n", ID_STRING(packet.id));
            } else {
                Serial.printf("Ignoring alarm acknowledgement for node %s, state = %d\n", ID_STRING(packet.id), state);
            }
            break;
        case PairSensor:
        case PairReceiver:
            Serial.printf("Received pair request from node %s\n", ID_STRING(packet.id));
            last_heartbeat_ms = millis(); // Don't send next heartbeat to avoid interference
            comm.pairResponse();
            break;
        case Alarm: // Ignore these packets
        case LowPower:
        case Heartbeat:
        case PairResponse:
            break;
        default: // TODO: Test bad packet type
            Serial.printf("Unknown packet type (%d) received from node %s\n", packet.type, ID_STRING(packet.id));
            break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Initialized Serial");

    pinMode(speaker, OUTPUT);
    noTone(speaker);

    if (!comm.begin()) {
        Serial.println("Error initializing LoRa module");
        goto err;
    }
    Serial.println("Initialized LoRa module");

    if (!detector.begin()) {
        Serial.println("Error initializing motion detector");
        goto err;
    }
    Serial.println("Initialized motion detector");

    Serial.println("Initialized");

    comm.pairSensor(); // After turning on, request to pair
    Serial.println("Sent pair sensor request");

    pairing_start = millis();

    return;
err:
    while (true) delay(100);
}

void loop() {
    Packet packet{};
    if (comm.recvPacket(&packet)) {
        handlePacket(packet);
    }

    updateState();
}
