#include <RadioLib.h>

const uint32_t cs = 1;
const uint32_t gdo0 = 2;
const uint32_t gdo2 = 3;
CC1101 radio = new Module(cs, gdo0, RADIOLIB_NC, gdo2);

void setup() {
  Serial.begin(9600);

  int err = radio.begin();
  if (err != RADIOLIB_ERR_NONE) {
    Serial.println("Failed to init radio");
    while (true) { delay(10); }
  }
}

const char* data = "Hello, world!";
const size_t len = strlen(data) + 1;

void loop() {
#ifdef SENSOR
  int err = radio.transmit(data, len);
  delay(1000);
#elsif
  char recv_data[len];
  int err = radio.receive((uint8_t*)recv_data, len);
  recv_data[len - 1] = 0; // Null terminate (just in case)
  Serial.printf("Received: '%s'\n", recv_data);
#endif
}
