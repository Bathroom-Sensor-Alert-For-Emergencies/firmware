#include <RadioLib.h>

const uint32_t cs = D6;
const uint32_t gdo0 = D5;
const uint32_t gdo2 = D4;
CC1101 radio = new Module(cs, gdo0, RADIOLIB_NC, gdo2);

void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.println("Starting radio");
    int err = radio.begin();
    if (err != RADIOLIB_ERR_NONE) {
        digitalWrite(LED_BUILTIN, HIGH);
        while (true) {
            Serial.printf("Failed to init radio, err = %d\n", err);
            delay(1000);
        }
    } else {
        Serial.println("Initialized radio");
    }
}

const char* data = "Hello, world!";
const size_t len = strlen(data) + 1;

void loop() {
#ifdef SENSOR
    int err = radio.transmit(data, len);
    Serial.printf("Sent! (err = %d)\n", err);

    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(750);
#elif defined(RECEIVER)
    char recv_data[len];
    int err = radio.receive((uint8_t*)recv_data, len);
    recv_data[len - 1] = 0; // Null terminate (just in case)
    Serial.printf("Received: '%s' (err = %d)\n", recv_data, err);
#else
    #error "Must define SENSOR or RECEIVER"
#endif
}
