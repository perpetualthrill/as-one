#include <Arduino.h>


class BufferPulseSensor {
  private:
    
  // Internal use constants
  static const int LATEST_SIZE = 3; 
  static const int GOOD_BEAT_SIZE = 3; 
  // known bad value
  static const int SENTINEL = 9999; 
  // Magic number: a large enough difference from average to trigger
  // beat detection. Empirically gathered.
  static const int DIFFERENCE_THRESHOLD = 20;

  // Configuration
  int pin;

  // Sampling
  int signal;              // latest reading from sensor
  int latest[LATEST_SIZE]; // latest signal, measured across a few samples
  int fifo[FIFO_SIZE];     // buffer to measure latest against
  long lastBeatMs = 0;     // when was last beat detected
  long goodBeatTimes[GOOD_BEAT_SIZE];  // keep timestamps for successful detections

  // Blank default constructor for array initialization
  public: BufferPulseSensor() { } 

  // Legit constructor for using this object
  public: BufferPulseSensor(int inputPin) {
    pin = inputPin;
    resetSensor();
  }

  public: void resetSensor() {
    for (int i = 0; i < LATEST_SIZE; i++) {
      latest[i] = SENTINEL;
    }
    for (int i = 0; i < FIFO_SIZE; i++) {
      fifo[i] = SENTINEL;
    }
  }

  private: void bufferValue(int value, int buff[], int buffSize) {
    for (int i = buffSize - 1; i > 0; i--) {
      buff[i] = buff[i - 1];
    }
    if ((value > VALID_SIGNAL_UPPER_BOUND) || (value < VALID_SIGNAL_LOWER_BOUND)) {
      // user is disconnected or data is otherwise faulty
      buff[0] = SENTINEL;
    } else {
      buff[0] = value;
    }
  }

  private: void bufferValue(int val) {
    bufferValue(val, latest, LATEST_SIZE);
    bufferValue(val, fifo, FIFO_SIZE);
  }

  private: int bufferAverage(int buff[], int buffSize) {
    int sum = 0;
    for (int i = 0; i < buffSize; i++) {
      if (buff[i] == SENTINEL) {
        return -1;
      }
      sum += buff[i];
    }
    return sum / buffSize;
  }

  private: bool detectBeat() {
    int fifoAvg = bufferAverage(fifo, FIFO_SIZE);
    int latestAvg = bufferAverage(latest, LATEST_SIZE);
    if (fifoAvg == -1) return false; // A bad data sentinel was found. No possibility of a beat
    if ((latestAvg - fifoAvg) > DIFFERENCE_THRESHOLD) {
      return true;
    }
    return false;
  }  

  private: void saveGoodBeat(int time) {
    for (int i = GOOD_BEAT_SIZE - 1; i > 0; i--) {
      goodBeatTimes[i] = goodBeatTimes[i - 1];
    }
    goodBeatTimes[0] = time;
  }

  public: void readNextSample() {
    signal = analogRead(pin);
    bufferValue(signal);
    long currentTime = millis();
    if (detectBeat()) {
      long interval = currentTime - lastBeatMs;
      lastBeatMs = currentTime;
      if (interval > MIN_INTERVAL_MS && interval < MAX_INTERVAL_MS) {
        saveGoodBeat(currentTime);
      }
    }
  }

  // No-op -- this method exists to match previous implementation
  public: void processLatestSample() { }

  public: int getLatestSample() {
    return signal;
  }

  public: int getBeatsPerMinute() {
    int sumBPM = 0;
    for (int i = 0; i < GOOD_BEAT_SIZE - 1; i++) {
      if ((goodBeatTimes[i] - goodBeatTimes[i+1]) == 0) return 0; // avoid divide by zero
      sumBPM += MS_PER_SECOND / (goodBeatTimes[i] - goodBeatTimes[i+1]);
    }
    int bpm = sumBPM / (GOOD_BEAT_SIZE - 1);
    if (bpm > MAX_BPM || bpm < MIN_BPM) {
      return 0;
    }
    return bpm;
  }

};
