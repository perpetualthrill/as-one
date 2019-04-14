#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

const int ANALOG_INPUT_3 = A3;

// Adjust this number to avoid noise when idle
const int THRESHOLD = 550;

PulseSensorPlayground pulseSensor(1);

void setup() {
  Serial.begin(115200);

  pulseSensor.analogInput(ANALOG_INPUT_3, 0);
  pulseSensor.setThreshold(THRESHOLD);
  pulseSensor.begin();
}

void loop() {
  int sample = pulseSensor.getLatestSample(0);
  int bpm = pulseSensor.getBeatsPerMinute(0);
  //int fixedSample = sample / 4;
  int fixedSample = sample;
  Serial.print(fixedSample);
  Serial.print(",");
  Serial.println(bpm);
  delay(250);
}
