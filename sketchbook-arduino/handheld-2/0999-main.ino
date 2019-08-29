// Reasonable (?) thresholds for resting heart rate
const int MAX_BPM = 125;
const int MIN_INTERVAL_MS = (int)(((float)60 / MAX_BPM) * 1000);
const int MIN_BPM = 60;

// known bad value
static const int SENTINEL = -9999; 

// sensor readings
const int SENSOR_COUNT = 4;
const int sensors[SENSOR_COUNT] = { A5, A4, A7, A6 };
const int READING_BUFFER_SIZE = REPORT_FREQUENCY_HZ * 2;
int readings[SENSOR_COUNT][READING_BUFFER_SIZE];
int averageReading[SENSOR_COUNT] = { 1023, 1023, 1023, 1023 };
const int AVERAGE_PERIOD_MS = 2000;

// factor to multiply averages by, to filter out bogus beat readings
// sensor 2 seems to like this at 1.025, while the other gets no
// readings at all that way :-(
static constexpr float AVERAGE_BONUS = 1.025;

long lastReport = millis();
long lastAverage = millis();

long nextMotor = 0;
const int MOTOR_PERIOD_MS = 150;
const int MOTOR_PIN = 13; // board D13

EspPwmChannel leds[SENSOR_COUNT];

void setup() {
  Serial.begin(115200);

  // Configure the ESP32 ADC. This is from esp32-hal-adc.h
  analogSetWidth(10); // range 0 - 1023, as expected by pulsesensor
  analogSetAttenuation(ADC_11db); // highest attenuation, used for 3.3v input

  // Configure feedback outputs
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  leds[0] = EspPwmChannel(12, 0); // board D12
  leds[1] = EspPwmChannel(14, 1); // board D14
  leds[2] = EspPwmChannel(27, 2); // board D27
  leds[3] = EspPwmChannel(26, 3); // board D26

  // fill reading array with do-not-use sentinels 
  for (int i = 0; i < SENSOR_COUNT; i++) {
    for (int j = 0; j < READING_BUFFER_SIZE; j++) {
      readings[i][j] = SENTINEL;
    }
  }
}

void insertAtZero(int value, int a[], int len) {
  for (int i = len; i >= 0; i--) {
    a[i+1] = a[i];
  }
  a[0] = value;
}

void printArray(int a[], int len) {
  for (int i = 0; i < len; i++) {
    Serial.print(a[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void loop() {
  long now = millis();

  if ((now - lastReport) > REPORT_PERIOD_MS) {

    // take readings
    for (int i = 0; i < SENSOR_COUNT; i++) {
      int sample = analogRead(sensors[i]);

      // Update LED percent. Sensor is 10 bit.
      leds[i].write(((float)sample) / 1023.0);

      // Save valid samples to averaging array
      if (sample < 200) {
        insertAtZero(SENTINEL, readings[i], READING_BUFFER_SIZE);
      } else {
        insertAtZero(sample, readings[i], READING_BUFFER_SIZE);
      }

      Serial.print(sample);
      Serial.print(",");
    }

    // recalculate averages every once in a while
    if ((now - lastAverage) > AVERAGE_PERIOD_MS) {
      for (int i = 0; i < SENSOR_COUNT; i++) {
        int sum = 0;
        int divisor = 0;
        for (int j = 0; j < READING_BUFFER_SIZE; j++) {
          if (readings[i][j] != SENTINEL) {
            sum += readings[i][j];
            divisor++;
          }
        }
        if (divisor > 0) {
          averageReading[i] = (int)(((float)sum / (float)divisor) * AVERAGE_BONUS);
        }
      }
      lastAverage = now;
    }
    for (int i = 0; i < SENSOR_COUNT; i++) {
      Serial.print(averageReading[i]);
      Serial.print(",");
    }

    // determine consensus
    int inBeatCount = 0;
    for (int i = 0; i < SENSOR_COUNT; i++) {
      // this is safe vs sentinels because the sentinel value is negative so this
      // test will always fail for any bogus values
      if ((readings[i][0] > averageReading[i])
          && (readings[i][1] > averageReading[i])
          && (readings[i][2] > averageReading[i])) {
            inBeatCount++;
      }
    }

    // Put down a bump on the plotter if there is consensus
    Serial.println(inBeatCount * 40);

    // Trigger motor feedback if enough time has passed and sensors are in-beat
    if ((inBeatCount > 0) &&
        (now > (nextMotor + MIN_INTERVAL_MS))) {
      nextMotor = now;
    }

    // Turn motor on or off as necessary
    if ((now >= nextMotor) && (now <= (nextMotor + MOTOR_PERIOD_MS))) {
   //    digitalWrite(MOTOR_PIN, HIGH);
    } else {
      digitalWrite(MOTOR_PIN, LOW);
    }

    lastReport = now;
  }

  yield();

}
