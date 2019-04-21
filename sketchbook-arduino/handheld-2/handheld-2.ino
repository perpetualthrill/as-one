#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>

// Adjust this number to avoid noise when idle
const int THRESHOLD = 500;

// Reasonable (?) thresholds for resting heart rate
const int MAX_BPM = 135;
const int MIN_BPM = 50;

const int SENSOR_COUNT = 4;
PulseSensorPlayground pulseSensor(SENSOR_COUNT);

// Hold bpm readings to acheive consensus
int bpmBuffer[SENSOR_COUNT];

// 25 ms => 40 hz
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
      if (bpm > MAX_BPM) bpm /= 2; // sensors often report double
      if (bpm < MIN_BPM) bpm *= 2; // ... or half
      bpmBuffer[i] = bpm;

      if (pulseSensor.isInsideBeat(i)) inBeatCount++;

      int fixedSample = sample;
      fixedSample = fixedSample / 4;

      Serial.print(fixedSample);
      Serial.print(",");
    }

    for (int i = 0; i < SENSOR_COUNT; i++) {
      Serial.print(bpmBuffer[i]);
      Serial.print(",");
    }

    bool hasConsensus = false;
    for (int i = 0; i < (SENSOR_COUNT - 1); i++) {
      for (int j = i + 1; j < SENSOR_COUNT; j++) {
        // close enough!
        if (bpmBuffer[j] - 1 <= bpmBuffer[i] <= bpmBuffer[j] + 1) hasConsensus = true;
      }
    }

    if (hasConsensus) {
      Serial.println(inBeatCount * 40);
    } else {
      Serial.println(0);
    }

    lastReport = millis();
  }

  // Need to do this constantly because we're not using the timer
  pulseSensor.sawNewSample();

  // But not too constantly
  delay(1);

}
