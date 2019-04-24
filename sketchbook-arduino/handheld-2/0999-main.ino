#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>

// Adjust this number to avoid noise when idle
const int THRESHOLD = 500;

// Reasonable (?) thresholds for resting heart rate
const int MAX_BPM = 135;
const int MIN_INTERVAL_MS = (int)(((float)60 / MAX_BPM) * 1000);
const int MIN_BPM = 50;

const int SENSOR_COUNT = 4;
PulseSensorPlayground pulseSensor(SENSOR_COUNT);

// Hold bpm readings to acheive consensus
int bpmBuffer[SENSOR_COUNT];

// 25 ms => 40 hz
const int REPORT_PERIOD_MS = 25;
long lastReport = millis();

long nextMotor = 0;
long lastMotor = millis();
const int MOTOR_PERIOD_MS = 180;

EspPwmChannel leds[SENSOR_COUNT];

void setup() {
  Serial.begin(115200);

  // Configure the ESP32 ADC. This is from esp32-hal-adc.h
  analogSetWidth(10); // range 0 - 1023, as expected by pulsesensor
  analogSetAttenuation(ADC_11db); // highest attenuation, used for 3.3v input

  // Configure pulseSensor library and gather an initial sample
  pulseSensor.analogInput(A6, 0); // board D34
  pulseSensor.analogInput(A7, 1); // board D35
  pulseSensor.analogInput(A4, 2); // board D32
  pulseSensor.analogInput(A5, 3); // board D33
  pulseSensor.setThreshold(THRESHOLD);
  pulseSensor.begin();
  pulseSensor.sawNewSample();

  // Configure outputs
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  //leds.assign(EspPwmChannel(12, 1, 10, 10000, .30));
  leds[0] = EspPwmChannel(12, 1, 10, 10000, .30);
}

void loop() {
  long now = millis();

  if ((now - lastReport) > REPORT_PERIOD_MS) {

    int inBeatCount = 0;
    float fixedSampleSum = 0;
    for (int i = 0; i < SENSOR_COUNT; i++) {
      int sample = pulseSensor.getLatestSample(i);
      int bpm = pulseSensor.getBeatsPerMinute(i);
      if (bpm > MAX_BPM) bpm /= 2; // sensors often report double
      if (bpm < MIN_BPM) bpm *= 2; // ... or half
      bpmBuffer[i] = bpm;

      if (pulseSensor.isInsideBeat(i)) inBeatCount++;

      int fixedSample = sample;
      fixedSample = fixedSample / 4;
      fixedSampleSum += fixedSample;

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
        if ((bpmBuffer[i] >= MIN_BPM) &&
            ((bpmBuffer[j] - 1 == bpmBuffer[i]) ||
             (bpmBuffer[j] == bpmBuffer[i]) ||
             (bpmBuffer[j] + 1 == bpmBuffer[i]))) {
          hasConsensus = true;
        }
      }
    }

    if (hasConsensus) {
      Serial.println(inBeatCount * 40);
      // Trigger motor feedback if enough time has passed
      if ((inBeatCount > 0) &&
          (now > (nextMotor + MIN_INTERVAL_MS))) {
        lastMotor = nextMotor;
        nextMotor = now;
      }
    } else {
      Serial.println(0);
    }

    // Turn motor on or off as necessary
    if ((now >= nextMotor) && (now <= (nextMotor + MIN_INTERVAL_MS))) {
      digitalWrite(13, HIGH);
    } else {
      digitalWrite(13, LOW);
    }

    // Update LEDs
    leds[0].write(fixedSampleSum / (float)(2 ^ 10));

    lastReport = now;
  }

  // Need to call this constantly because we're not using the timer
  pulseSensor.sawNewSample();

  // ... but not too constantly
  delay(1);

}
