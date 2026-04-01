#include "config.hpp"
#include "alarm.hpp"
#include <Arduino.h>
#include <deque>
#include <bitset>

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

State state = State::Pairing;
unsigned long pairing_start = 0;

unsigned long last_heartbeats[NUM_SENSORS];
std::bitset<NUM_SENSORS> valids{};
std::deque<Alarm> alarms{};

void setAlarm(bool enable) {
    static bool enabled = false;

    if (enable && !enabled) {
        enabled = true;
        tone(speaker, speaker_freq);
    } else if (!enable && enabled){
        enabled = false;
        noTone(speaker);
    }
}

inline bool buttonPressed() {
    return digitalRead(ack_button) == HIGH;
}

inline void pair(DeviceID id) {
    last_heartbeats[id] = millis() + PAIRING_PERIOD_MS;
    valids.set(id, true);
}

void updateState() {
    using enum State;
    switch (state) {
        case Pairing:
            if (millis() > pairing_start + PAIRING_PERIOD_MS) {
                // Initialize heartbeat timestamps
                unsigned long time = millis();
                unsigned num_sensors = 0;
                for (std::size_t i = 0; i < NUM_SENSORS; i++) {
                    last_heartbeats[i] = time;
                    if (valids[i]) num_sensors++;
                }

                state = Idle;
                digitalWrite(pair_led, LOW);
                lcd.clear();
                Serial.printf("Done pairing, found %d sensors, id %s (%d)\n", num_sensors, ID_STRING(comm.id), comm.id);
            }
            break;
        case Idle:
            // Check if we haven't received a heartbeat from each paired sensor
            for (std::size_t i = 0; i < NUM_SENSORS; i++) {
                if (valids[i] && millis() > last_heartbeats[i] + HEARTBEAT_MAX_PERIOD_MS) {
                    // Add heartbeat alarm to alarm queue
                    lcd.emplace(AlarmType::Heartbeat, i);
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
                    Alarm alarm = lcd.dequeue();

                    if (alarm.type == AlarmType::Heartbeat) {
                        last_heartbeats[alarm.id] = millis(); // Give enough time for heartbeat to come
                        Serial.printf("Acknowledged heartbeat alarm for node %s\n", ID_STRING(alarm.id));
                    } else if (alarm.type == AlarmType::Alarm) {
                        comm.ackAlarm(alarm.id);
                        Serial.printf("Sent acknowledgement to node %s for %s alarm\n", ID_STRING(alarm.id), alarm.name());
                    }
                }

                if (alarms.empty()) {
                    setAlarm(false);
                    Serial.println("No more alarms, returning to idle state");
                    state = Idle;
                }

                // TODO: Lazy debounce
                while (buttonPressed()) delay(100);
                Serial.println("Released alarm acknowledge button");
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
                last_heartbeats[packet.id] = millis(); // Alarm serves as heartbeat as well to reduce traffic
                lcd.emplace(AlarmType::Alarm, packet.id);
                state = Alarmed;
                Serial.printf("Alarm from node %s\n", ID_STRING(packet.id));
            } else {
                Serial.printf("Ignoring alarm from node %s while pairing\n", ID_STRING(packet.id));
            }
            break;
        case LowPower:
            // Add alarm to stack
            if (state != Pairing) {
                lcd.emplace(AlarmType::LowPower, packet.id);
                state = Alarmed;
                Serial.printf("Low power alarm from node %s\n", ID_STRING(packet.id));
            } else {
                Serial.printf("Ignoring low power warning from node %d while pairing\n", ID_STRING(packet.id));
            }
            break;
        case PairSensor:
            pair(packet.id);
            lcd.paired(packet.id);
            Serial.printf("Paired new sensor. Now at %d sensors\n");
            break;
        case PairResponse:
            if (state == Pairing) {
                pair(packet.id);
                lcd.pairedId(packet.id);
                Serial.printf("Received pair response from node %d\n", ID_STRING(packet.id));
            } else {
                Serial.printf("Ignoring pair response from node %d while not pairing\n", ID_STRING(packet.id));
            }
            break;
        case Heartbeat:
            if (state != Pairing) {
                last_heartbeats[packet.id] = millis();
                Serial.printf("Received heartbeat from node %s\n", ID_STRING(packet.id));
            } else {
                Serial.printf("Ignoring heartbeat from node %s while pairing\n", ID_STRING(packet.id));
            }
            break;
        case Disconnect:
            valids.set(packet.id, false);
            break;
        case AckAlarm: // Ignore these packets
        case PairReceiver:
        default:
            Serial.printf("Unknown packet type (%d) received from node %s\n", packet.type, ID_STRING(packet.id));
            break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Initialized serial");

    pinMode(pair_led, OUTPUT);
    digitalWrite(pair_led, HIGH);
    pinMode(low_power_led, OUTPUT);
    digitalWrite(low_power_led, LOW);
    pinMode(speaker, OUTPUT);
    noTone(speaker);
    pinMode(ack_button, INPUT);
    Serial.println("Initialized pins");

    if (!comm.begin()) {
        Serial.println("Error initializing LoRa module");
        goto err;
    }
    Serial.println("Initialized LoRa module");

    lcd.begin(&alarms);
    Serial.println("Initialized LCD screen");

    Serial.println("Initialized");

    comm.pairReceiver(); // After turning on, request to pair
    Serial.println("Sent pair receiver request");

    pairing_start = millis();
    lcd.pair();

    return;
err:
    while (true) delay(100);
}

unsigned long last_print = 0;

void loop() {
    if (millis() - last_print > 1000) {
        last_print = millis();
        Serial.printf("In %s state", stateName(state));
        if (state == State::Alarmed) Serial.printf(", %d alarms", alarms.size());
        Serial.printf("\n");
        delay(2);
    }

    Packet packet{};
    if (comm.recvPacket(&packet)) {
        handlePacket(packet);
    }

    updateState();
}
