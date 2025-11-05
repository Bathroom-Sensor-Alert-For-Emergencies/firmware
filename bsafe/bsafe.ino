#if defined(SENSOR)
    #include "sensor.hpp"
#elif defined(RECEIVER)
    #include "receiver.hpp"
#else
    #error "Must define SENSOR or RECEIVER"
#endif
