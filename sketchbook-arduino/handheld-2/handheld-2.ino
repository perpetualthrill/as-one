// Placeholder file with imports. Possibly to hold constants at some point in the future?
// Actual code entry points in 0999-main

// Not using arduino, so update sensors manually
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>

// Handy stuffs
#include <math.h>

// 25 ms => 40 hz
// 20 ms => 50 hz
const int REPORT_FREQUENCY_HZ = 50;
const int REPORT_PERIOD_MS = 1000 / REPORT_FREQUENCY_HZ;
