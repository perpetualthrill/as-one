// Adjust this number to avoid noise when idle
const int THRESHOLD = pow(2, 9); // half of sensor resolution

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
const int MOTOR_PERIOD_MS = 100;
const int MOTOR_PIN = 13; // board D13

EspPwmChannel leds[SENSOR_COUNT];
const int PWM_RESOLUTION = 10;
const int PWM_FREQUENCY = 10000;

void setup() {
  Serial.begin(115200);

  // Configure the ESP32 ADC. This is from esp32-hal-adc.h
  analogSetWidth(10); // range 0 - 1023, as expected by pulsesensor
  analogSetAttenuation(ADC_11db); // highest attenuation, used for 3.3v input

  // Configure pulseSensor library and gather an initial sample
  pulseSensor.analogInput(A5, 0); // board D33
  pulseSensor.analogInput(A4, 1); // board D32
  pulseSensor.analogInput(A7, 2); // board D35
  pulseSensor.analogInput(A6, 3); // board D34
  pulseSensor.setThreshold(THRESHOLD);
  pulseSensor.begin();
  pulseSensor.sawNewSample();

  // Configure feedback outputs
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  leds[0] = EspPwmChannel(12, 0, PWM_RESOLUTION, PWM_FREQUENCY, REPORT_PERIOD_MS - 3); // board D12
  leds[1] = EspPwmChannel(14, 1, PWM_RESOLUTION, PWM_FREQUENCY, REPORT_PERIOD_MS - 3); // board D14
  leds[2] = EspPwmChannel(27, 2, PWM_RESOLUTION, PWM_FREQUENCY, REPORT_PERIOD_MS - 3); // board D27
  leds[3] = EspPwmChannel(26, 3, PWM_RESOLUTION, PWM_FREQUENCY, REPORT_PERIOD_MS - 3); // board D26
}

void loop() {
  long now = millis();

  if ((now - lastReport) > REPORT_PERIOD_MS) {

    int inBeatCount = 0;
    for (int i = 0; i < SENSOR_COUNT; i++) {
      int sample = pulseSensor.getLatestSample(i);
      int bpm = pulseSensor.getBeatsPerMinute(i);
      if (bpm > MAX_BPM) bpm /= 2; // sensors often report double
      if (bpm < MIN_BPM) bpm *= 2; // ... or half
      bpmBuffer[i] = bpm;

      if (pulseSensor.isInsideBeat(i)) inBeatCount++;

      // Update LED percent. Sensor is 10 bit.
      leds[i].write(((float)sample) / pow(2, PWM_RESOLUTION));

      Serial.print(sample);
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
      digitalWrite(MOTOR_PIN, HIGH);
    } else {
      digitalWrite(MOTOR_PIN, LOW);
    }

    lastReport = now;
  }

  // Need to call this constantly because we're not using the timer
  pulseSensor.sawNewSample();

  // ... but not too constantly
  delay(1);

}
