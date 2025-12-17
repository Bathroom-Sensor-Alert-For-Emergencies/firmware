#if defined(SENSOR)
    #include "sensor.hpp"
#elif defined(RECEIVER)
    #include "receiver.hpp"
#else
    #error "Must define SENSOR or RECEIVER"
#endif
// #include "power_test.hpp"
// #if defined(SENSOR)
//     #include "test.hpp"
// #elif defined(RECEIVER)
//     #include "sleep.hpp"
// #else
//     #error "Must define SENSOR or RECEIVER"
// #endif
