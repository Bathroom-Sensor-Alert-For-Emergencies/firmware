#include "communicator.hpp"
#include "radio.hpp"

const uint32_t cs = D6;
const uint32_t gdo0 = D5;
const uint32_t gdo2 = D4;
Radio radio{cs, gdo0, gdo2};
Communicator& comm = radio;

void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.println("Starting radio");
    if (!comm.begin()) {
        digitalWrite(LED_BUILTIN, HIGH);
        while (true) {
            Serial.println("Failed to init radio");
            delay(1000);
        }
    } else {
        Serial.println("Initialized radio");
    }

#ifdef RECEIVER
    Serial.printf("Listening for packet\n");
    comm.listen([](Packet packet) {
        Serial.printf("Received packet: ");
        using enum PacketType;
        switch (packet.type) {
            case Alarm:
                Serial.printf("Alarm from node %d\n", packet.id);
                break;
            case AckAlarm:
                Serial.printf("Alarm acknowledged by node %d\n", packet.id);
                break;
            case LowPower:
                Serial.printf("Low power from node %d\n", packet.id);
                break;
            case Pair:
                Serial.printf("Pair request\n");
                break;
            case Heartbeat:
                Serial.printf("Heartbeat\n");
                break;
            default:
                Serial.printf("Unknown packet type (%d) received\n", packet.type);
                break;
        }
    });
#endif
}

void loop() {
#ifdef SENSOR
    bool err = comm.alarm();
    Serial.printf("Sent! (err = %d)\n", err);

    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(750);
#elif defined(RECEIVER)
#else
    #error "Must define SENSOR or RECEIVER"
#endif
}
