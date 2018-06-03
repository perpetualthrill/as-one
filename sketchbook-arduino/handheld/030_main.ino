static const int SENSOR_COUNT = 3;
MinimalPulseSensor sensors[SENSOR_COUNT];

// track time until reading next sample
volatile unsigned long nextSampleMicros;

// track time until reporting next sample
volatile unsigned long nextReportMicros;

void setup() {
  sensors[0] = MinimalPulseSensor(A0);
  sensors[1] = MinimalPulseSensor(A3);
  sensors[2] = MinimalPulseSensor(A6);
  nextSampleMicros = micros() + MS_PER_READ * 1000L;
  nextReportMicros = micros() + MS_PER_REPORT * 1000L;

  // This sends a lot of data
  Serial.begin(115200);

  // Configure the ESP32 ADC. This is from esp32-hal-adc.h
  analogSetWidth(10); // range 0 - 1023, as expected by pulsesensor
  analogSetAttenuation(ADC_11db); // highest attenuation, used for 3.3v input

}

// aka PulseSensorPlayground::sawNewSample
boolean checkForSample() {
  
  unsigned long nowMicros = micros();
  if ((long) (nextSampleMicros - nowMicros) > 0L) {
    return false;  // not time yet.
  }

  // ok, time to take a sample. reset the clock
  nextSampleMicros = nowMicros + MS_PER_READ * 1000L;

  // Process new samples
  for (int i = 0; i < SENSOR_COUNT; ++i) {
    sensors[i].readNextSample();
    sensors[i].processLatestSample();
  }

  return true;
}

void loop() {
  if (checkForSample()) {
    if (micros() > nextReportMicros) {
      nextReportMicros = micros() + MS_PER_REPORT * 1000L;
      for (int i = 0; i < SENSOR_COUNT; ++i) {
        if (i != 0) {
          Serial.print(F(","));
        }
        Serial.print(sensors[i].getBeatsPerMinute());
        Serial.print(F(","));
        Serial.print(sensors[i].getLatestSample());
      }
      Serial.println();
    }
  }
}

