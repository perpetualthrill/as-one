//#define USE_ARDUINO_INTERRUPTS true
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>


// Adjust this number to avoid noise when idle
const int THRESHOLD = 500;

const int PEAK_TROUGH = 30;
const int MAX_SIGNAL = 1024;

const int SENSOR_COUNT = 4;
PulseSensorPlayground pulseSensor(SENSOR_COUNT);

const int REPORT_PERIOD_MS = 25;
long lastReport;

void setup() {
  Serial.begin(115200);


  // Configure the ESP32 ADC. This is from esp32-hal-adc.h
  analogSetWidth(10); // range 0 - 1023, as expected by pulsesensor
  analogSetAttenuation(ADC_11db); // highest attenuation, used for 3.3v input


  pulseSensor.analogInput(A6, 0); // board D34
  pulseSensor.analogInput(A7, 1); // board D35
  pulseSensor.analogInput(A4, 2); // board D32
  pulseSensor.analogInput(A5, 3); // board D33
  pulseSensor.setThreshold(THRESHOLD);
  pulseSensor.begin();

  lastReport = millis();
}

void loop() {

  if ((millis() - lastReport) > REPORT_PERIOD_MS) {

    int inBeatCount = 0;
    for (int i = 0; i < SENSOR_COUNT; i++) {
      int sample = pulseSensor.getLatestSample(i);
      int bpm = pulseSensor.getBeatsPerMinute(i);
      if (pulseSensor.isInsideBeat(i)) inBeatCount++;

      int fixedSample = sample;
      fixedSample = fixedSample / 4;

      Serial.print(fixedSample);
      Serial.print(",");
//      Serial.print(bpm);
//      Serial.print(",");
    }

    Serial.println(inBeatCount * 40);
    lastReport = millis();
  }

  // Need to do this constantly because we're not using the timer
  pulseSensor.sawNewSample();

  // But not too constantly
  delay(1);

}
