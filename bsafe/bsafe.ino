#if defined(SENSOR)
    #include "sensor.hpp"
#elif defined(RECEIVER)
    #include "receiver.hpp"
#else
    #error "Must define SENSOR or RECEIVER"
#endif
// void setup() {
//   Serial.begin(115200);
//   delay(2000);
// }
// void loop() {
//   Serial.println("Test");
//   delay(1000);
// }
// #include "power_test.hpp"
// #if defined(SENSOR)
//     #include "test.hpp"
// #elif defined(RECEIVER)
//     #include "sleep.hpp"
// #else
//     #error "Must define SENSOR or RECEIVER"
// #endif
