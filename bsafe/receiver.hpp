#include "config.hpp"
#include <Arduino.h>
#include <stack>
#include <vector>

enum class State {
    Pairing,
    Idle,
    Alarmed,
};

const char* stateName(State s) {
    using enum State;
    switch (s) {
        case Pairing:
            return "pairing";
        case Idle:
            return "idle";
        case Alarmed:
            return "alarmed";
    }
}

enum class AlarmType {
    Heartbeat,
    Alarm,
    LowPower,
};

struct Alarm {
    AlarmType type{};
    DeviceID id{};

    Alarm() = default;
    Alarm(AlarmType type, DeviceID id) : type{type}, id{id} {}

    const char* name() {
        using enum AlarmType;
        switch (this->type) {
            case Heartbeat:
                return "heartbeat";
            case Alarm:
                return "unresponsive";
            case LowPower:
                return "low power";
        }
    }
};


volatile unsigned long pairing_start = 0;
// TODO: What happens if a sensor gets removed from the network?
// Disconnect button that sends special message?
volatile std::uint8_t num_sensors = 0;
volatile unsigned long last_heartbeats[256];

std::stack<Alarm> alarms;

volatile State state = State::Pairing;

void setAlarm(bool enable) {
    // static unsigned long last_turn_on = 0;
    // static bool enabled = false;
    //
    // if (!enabled && enable) last_turn_on = millis();
    // enabled = enable;
    //
    // bool high = ((millis() - last_turn_on) / LED_BLINK_PERIOD_MS) % 2 == 0;
    // digitalWrite(alarm_led, enabled && high ? HIGH : LOW);
    digitalWrite(alarm_led, enable ? HIGH : LOW);
}

inline bool buttonPressed() {
    return digitalRead(ack_button) == HIGH;
}

void updateState() {
    using enum State;
    switch (state) {
        case Pairing:
            if (millis() > pairing_start + PAIRING_PERIOD_MS) {
                state = Idle;
                Serial.printf("Done pairing, found %d sensors\n", num_sensors);
                // while (true) delay(100);
            }
            break;
        case Idle:
            // Check if we haven't received a heartbeat from each paired sensor
            for (std::uint8_t i = 0; i < num_sensors; i++) {
                if (millis() > last_heartbeats[i] + HEARTBEAT_MAX_PERIOD_MS) {
                    // Add heartbeat alarm to alarm stack
                    alarms.emplace(AlarmType::Heartbeat, i);
                    state = Alarmed;
                    Serial.printf("Haven't received heartbeat from node %d\n", i);
                }
            }
            break;
        case Alarmed:
            setAlarm(true);

            if (buttonPressed()) {
                Serial.println("Pressed alarm acknowledge button");

                if (!alarms.empty()) {
                    Alarm alarm = alarms.top();
                    if (alarm.type == AlarmType::Heartbeat) {
                        last_heartbeats[alarm.id] = millis(); // Give enough time for heartbeat to come
                    } else {
                        comm.ackAlarm(alarm.id);
                        Serial.printf("Sent acknowledgement to node %d for %s alarm\n", alarms.top().id, alarm.name());
                    }

                    alarms.pop();
                }

                if (alarms.empty()) {
                    setAlarm(false);
                    Serial.println("No more alarms, returning to idle state");
                    state = Idle;
                }

                // TODO: Lazy debounce
                delay(250);
            }
            break;
    }
}

void handlePacket(Packet packet) {
    using enum PacketType;
    using enum State;
    switch (packet.type) {
        case Alarm:
            // Add alarm to stack
            if (state != Pairing) {
                alarms.emplace(AlarmType::Alarm, packet.id);
                state = Alarmed;
                Serial.printf("Alarm from node %d\n", packet.id);
            } else {
                Serial.printf("Ignoring alarm from node %d while pairing\n", packet.id);
            }
            break;
        case LowPower:
            // Add alarm to stack
            if (state != Pairing) {
                alarms.emplace(AlarmType::LowPower, packet.id);
                state = Alarmed;
                Serial.printf("Low power alarm from node %d\n", packet.id);
            } else {
                Serial.printf("Ignoring low power warning from node %d while pairing\n", packet.id);
            }
            break;
        case PairSensor:
            num_sensors++;
            Serial.printf("Paired new sensor. Now at %d sensors\n", num_sensors);
            break;
        case PairResponse:
            if (state == Pairing) num_sensors++;
            Serial.printf("Received pair response from node %d. Now at %d sensors\n", packet.id, num_sensors);
            break;
        case Heartbeat:
            last_heartbeats[packet.id] = millis();
            Serial.printf("Received heartbeat from node %d\n", packet.id);
            break;
        case AckAlarm: // Ignore these packets
        case PairReceiver:
        default: // TODO: Remember to test bad packet type
            Serial.printf("Unknown packet type (%d) received\n", packet.type);
            break;
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    pinMode(alarm_led, OUTPUT);
    digitalWrite(alarm_led, LOW);
    pinMode(ack_button, INPUT);

    if (!comm.begin()) {
        Serial.println("Error initializing radio");
        goto err;
    }

    comm.pairReceiver(); // After turning on, request to pair
    pairing_start = millis();
    // comm.listen(handlePacket);
    comm.startRecv();

    return;
err:
    while (true) delay(100);
}

void loop() {
    if (millis() % 1000 == 0) {
        Serial.printf("In %s state", stateName(state));
        if (state == State::Alarmed) Serial.printf(", %d alarms", alarms.size());
        Serial.printf("\n");
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
