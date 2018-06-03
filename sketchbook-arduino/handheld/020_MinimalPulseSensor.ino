#include <Arduino.h>

class MinimalPulseSensor {

  // Configuration
  int pin;           // Analog input pin for MinimalPulseSensor.

  // Pulse detection output variables.
  // Volatile because our pulse detection code could be called from an Interrupt
  volatile int bpm;                // int that holds raw Analog in 0. updated every call to readSensor()
  volatile int signal;             // holds the latest incoming raw data (0..1023)
  volatile int ibi;                // int that holds the time interval (ms) between beats! Must be seeded!
  volatile boolean pulse;          // "True" when User's live heartbeat is detected. "False" when not a "live beat".
  volatile boolean qs;             // The start of beat has been detected and not read by the Sketch.
  volatile int amp;                         // used to hold amplitude of pulse waveform, seeded (sample value)
  volatile unsigned long lastBeatTime;      // used to find IBI. Time (sampleCounter) of the previous detected beat start.

  // Variables internal to the pulse detection algorithm.
  // Not volatile because we use them only internally to the pulse detection.
  unsigned long sampleIntervalMs;  // expected time between calls to readSensor(), in milliseconds.
  int rate[10];                    // array to hold last ten IBI values (ms)
  unsigned long sampleCounter;     // used to determine pulse timing. Milliseconds since we started.
  int peak;                           // used to find peak in pulse wave, seeded (sample value)
  int trough;                           // used to find trough in pulse wave, seeded (sample value)
  int thresh;                      // used to find instant moment of heart beat, seeded (sample value)
  boolean firstBeat;               // used to seed rate array so we startup with reasonable BPM
  boolean secondBeat;              // used to seed rate array so we startup with reasonable BPM

  public:

  // Blank default constructor for array initialization
  MinimalPulseSensor() { } 

  // Legit constructor for using this object
  MinimalPulseSensor(int inputPin) {
  
    // Initialize the default configuration
    pin = inputPin;

    // Initialize (seed) the pulse detector
    for (int i = 0; i < 10; ++i) {
      rate[i] = 0;
    }
    qs = false;
    bpm = 0;
    ibi = 600;                  // 600ms per beat = 100 Beats Per Minute (bpm)
    pulse = false;
    sampleCounter = 0;
    lastBeatTime = 0;
    peak = MEDIAN_INPUT;                    // peak at 1/2 the input range of 0..1023
    trough = MEDIAN_INPUT;                    // trough at 1/2 the input range.
    thresh = DEFAULT_THRESHOLD;               // threshold a little above the trough
    amp = 100;                  // beat amplitude 1/10 of input range.
    firstBeat = true;           // looking for the first beat
    secondBeat = false;         // not yet looking for the second beat in a row
  }

  int getLatestSample() {
    return signal;
  }

  int getBeatsPerMinute() {
    if (bpm > MAX_BPM || bpm < MIN_BPM) {
      return 0;
    }
    return bpm;
  }

  int getInterBeatIntervalMs() {
    return ibi;
  }

  int getPulseAmplitude() {
    return amp;
  }

  unsigned long getLastBeatTime() {
    return lastBeatTime;
  }

  boolean sawStartOfBeat() {
    boolean started = qs;
    qs = false;
    return started;
  }

  boolean isInsideBeat() {
    return pulse;
  }

  void readNextSample() {
    // We assume assigning to an int is atomic.
    signal = analogRead(pin);
  }

  void processLatestSample() {
    sampleCounter += MS_PER_READ;                  // keep track of the time in mS with this variable
    int timeSinceLastMS = sampleCounter - lastBeatTime;      // monitor the time since the last beat to avoid noise

    //  find the peak and trough of the pulse wave
    if (signal < thresh && timeSinceLastMS > (ibi / 5) * 3) { // avoid dichrotic noise by waiting 3/5 of last IBI
      if (signal < trough) {                        // T is the trough
        trough = signal;                            // keep track of lowest point in pulse wave
      }
    }

    if (signal > thresh && signal > peak) {       // thresh condition helps avoid noise
      peak = signal;                              // P is the peak
    }                                             // keep track of highest point in pulse wave

    // look for beat. signal surges up in value every time there is a pulse
    if (timeSinceLastMS > MIN_INTERVAL_MS) {   // avoid high frequency noise
      if ( (signal > thresh) && (pulse == false) && (timeSinceLastMS > (ibi / 5) * 3) ) {
        pulse = true;                          // set the Pulse flag when we think there is a pulse
        ibi = sampleCounter - lastBeatTime;    // measure time between beats in mS
        lastBeatTime = sampleCounter;          // keep track of time for next pulse

        if (secondBeat) {                      // if this is the second beat, if secondBeat == TRUE
          secondBeat = false;                  // clear secondBeat flag
          for (int i = 0; i <= 9; i++) {       // seed the running total to get a realisitic BPM at startup
            rate[i] = ibi;
          }
        }

        if (firstBeat) {                       // if it's the first time we found a beat, if firstBeat == TRUE
          firstBeat = false;                   // clear firstBeat flag
          secondBeat = true;                   // set the second beat flag
          // IBI value is unreliable so discard it
          return;
        }


        // keep a running total of the last 10 IBI values
        word runningTotal = 0;                  // clear the runningTotal variable

        for (int i = 0; i <= 8; i++) {          // shift data in the rate array
          rate[i] = rate[i + 1];                // and drop the oldest IBI value
          runningTotal += rate[i];              // add up the 9 oldest IBI values
        }

        rate[9] = ibi;                          // add the latest IBI to the rate array
        runningTotal += rate[9];                // add the latest IBI to runningTotal
        runningTotal /= 10;                     // average the last 10 IBI values
        bpm = 60000 / runningTotal;             // how many beats can fit into a minute? that's BPM!
        qs = true;                              // set qs complex flag (we detected a beat)
      }
    }

    if (signal < thresh && pulse == true) {  // when the values are going down, the beat is over
      pulse = false;                         // reset the Pulse flag so we can do it again
      amp = peak - trough;                   // get amplitude of the pulse wave
      thresh = amp / 2 + trough;             // set thresh at 50% of the amplitude
      peak = thresh;                         // reset these for next time
      trough = thresh;
    }

    if (timeSinceLastMS > 2000) {            // reset if time since last beat is very long
      thresh = DEFAULT_THRESHOLD;                // set thresh default
      peak = MEDIAN_INPUT;                   // set P default
      trough = MEDIAN_INPUT;                 // set T default
      lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date
      firstBeat = true;                      // set these to avoid noise
      secondBeat = false;                    // when we get the heartbeat back
      bpm = 0;                               // make sure we don't report this sensor as valid
    }
  }
};
