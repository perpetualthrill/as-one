//#define USE_ARDUINO_INTERRUPTS true
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>


// Adjust this number to avoid noise when idle
const int THRESHOLD = 500;

const int PEAK_TROUGH = 30;
const int MAX_SIGNAL = 1024;

const int SENSOR_COUNT = 2;
PulseSensorPlayground pulseSensor(SENSOR_COUNT);

const int REPORT_PERIOD_MS = 25;
long lastReport;

void setup() {
  Serial.begin(115200);

  pulseSensor.analogInput(A3, 0);
  pulseSensor.analogInput(A2, 1);
  pulseSensor.setThreshold(THRESHOLD);
  pulseSensor.begin();

  lastReport = millis();
}

void loop() {

  if ((millis() - lastReport) > REPORT_PERIOD_MS) {

    for (int i = 0; i < SENSOR_COUNT; i++) {
      int sample = pulseSensor.getLatestSample(i);
      int bpm = pulseSensor.getBeatsPerMinute(i);

      // Remove obnoxious connection noise
      int fixedSample = sample;
      if (fixedSample > (MAX_SIGNAL - PEAK_TROUGH)) {
        fixedSample = PEAK_TROUGH;
      } else if (fixedSample < PEAK_TROUGH) {
        fixedSample = 0;
      }
      fixedSample = fixedSample / 4;

      Serial.print(fixedSample);
      Serial.print(",");
      Serial.print(bpm);
      Serial.print(",");
    }

    Serial.println();
    lastReport = millis();
  }

  // Need to do this constantly because we're not using the timer
  pulseSensor.sawNewSample();

}
