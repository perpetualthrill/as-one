static const int SENSOR_COUNT = 3;
BufferPulseSensor sensors[SENSOR_COUNT];

// for each sensor, keep the last n readings
static const int MEDIAN_ARRAY_SIZE = SENSOR_COUNT * 3;
int medianArray[MEDIAN_ARRAY_SIZE];

// track time until reading next sample
volatile unsigned long nextSampleMicros;

// track time until reporting next sample
volatile unsigned long nextReportMicros;

void setup() {
  sensors[0] = BufferPulseSensor(A0);
  sensors[1] = BufferPulseSensor(A3);
  sensors[2] = BufferPulseSensor(A6);
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

void orderedInsert(int value, int a[]) {
  int insertPoint = 0;
  for (int i = 0; i < MEDIAN_ARRAY_SIZE; i++) {
    if (a[i] < value) {
      insertPoint = i;
    }
  }
  for (int j = 0; j < insertPoint; j++) {
    a[j] = a[j+1];
  }
  a[insertPoint] = value;
}

void keepBPM(int value) {
  // discard spam
  int dupes = 0;
  for (int i = 0; i < MEDIAN_ARRAY_SIZE; i++) {
    if (medianArray[i] == value) dupes++;
    //if (dupes > SENSOR_COUNT) return;
    if (dupes > 0) return;
  }
  // replace last value, shifting down
  for (int i = 0; i < MEDIAN_ARRAY_SIZE - 1; i++) {
    medianArray[i] = medianArray[i+1];
  }
  medianArray[MEDIAN_ARRAY_SIZE - 1] = value;
}

void loop() {
  if (checkForSample()) {
    if (micros() > nextReportMicros) {
      nextReportMicros = micros() + MS_PER_REPORT * 1000L;
      for (int i = 0; i < SENSOR_COUNT; i++) {
        /*
        if (i != 0) {
          Serial.print(F(","));
        }
        //Serial.print(sensors[i].getLatestAverage());
        Serial.print(sensors[i].getBeatsPerMinute2());
        Serial.print(",");
        Serial.print(sensors[i].checkDetecto());
        */
        int bpm = sensors[i].getBeatsPerMinute2();
        if (bpm > MIN_BPM && bpm < MAX_BPM) {
         // Serial.println(bpm);
          keepBPM(bpm);
        }
      }
      // Sort array
      int sortedArray[MEDIAN_ARRAY_SIZE];
      for (int i = 0; i < MEDIAN_ARRAY_SIZE; i++) {
        orderedInsert(medianArray[i], sortedArray);
      }
      // Display
      for (int i = 0; i < MEDIAN_ARRAY_SIZE; i++) {
        Serial.print(sortedArray[i]);
        Serial.print(",");
      }
      Serial.println(sortedArray[MEDIAN_ARRAY_SIZE / 2]);
    }
  }
}

