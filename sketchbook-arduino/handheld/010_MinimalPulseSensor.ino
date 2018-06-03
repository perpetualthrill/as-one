#include <Arduino.h>

static const int MS_PER_READ = 2;

class MinimalPulseSensor {

  // Configuration
  int InputPin;           // Analog input pin for MinimalPulseSensor.
  
  // Pulse detection output variables.
  // Volatile because our pulse detection code could be called from an Interrupt
  volatile int BPM;                // int that holds raw Analog in 0. updated every call to readSensor()
  volatile int Signal;             // holds the latest incoming raw data (0..1023)
  volatile int IBI;                // int that holds the time interval (ms) between beats! Must be seeded!
  volatile boolean Pulse;          // "True" when User's live heartbeat is detected. "False" when not a "live beat".
  volatile boolean QS;             // The start of beat has been detected and not read by the Sketch.
  volatile int threshSetting;      // used to seed and reset the thresh variable
  volatile int amp;                         // used to hold amplitude of pulse waveform, seeded (sample value)
  volatile unsigned long lastBeatTime;      // used to find IBI. Time (sampleCounter) of the previous detected beat start.
  
  // Variables internal to the pulse detection algorithm.
  // Not volatile because we use them only internally to the pulse detection.
  unsigned long sampleIntervalMs;  // expected time between calls to readSensor(), in milliseconds.
  int rate[10];                    // array to hold last ten IBI values (ms)
  unsigned long sampleCounter;     // used to determine pulse timing. Milliseconds since we started.
  int P;                           // used to find peak in pulse wave, seeded (sample value)
  int T;                           // used to find trough in pulse wave, seeded (sample value)
  int thresh;                      // used to find instant moment of heart beat, seeded (sample value)
  boolean firstBeat;               // used to seed rate array so we startup with reasonable BPM
  boolean secondBeat;              // used to seed rate array so we startup with reasonable BPM

  public:

  // Blank default constructor for array initialization
  MinimalPulseSensor() { } 
  
  MinimalPulseSensor(int inputPin) {
  //class MinimalPulseSensor(int inputPin) {
    // Initialize the default configuration
    InputPin = inputPin;
  
    // Initialize (seed) the pulse detector
    sampleIntervalMs = MS_PER_READ;
    for (int i = 0; i < 10; ++i) {
      rate[i] = 0;
    }
    QS = false;
    BPM = 0;
    IBI = 600;                  // 600ms per beat = 100 Beats Per Minute (BPM)
    Pulse = false;
    sampleCounter = 0;
    lastBeatTime = 0;
    P = 512;                    // peak at 1/2 the input range of 0..1023
    T = 512;                    // trough at 1/2 the input range.
    threshSetting = 550;        // used to seed and reset the thresh variable
    thresh = 550;               // threshold a little above the trough
    amp = 100;                  // beat amplitude 1/10 of input range.
    firstBeat = true;           // looking for the first beat
    secondBeat = false;         // not yet looking for the second beat in a row
  }
  
  void setThreshold(int threshold) {
    threshSetting = threshold;
    thresh = threshold;
  }
  
  int getLatestSample() {
    return Signal;
  }
  
  int getBeatsPerMinute() {
    return BPM;
  }
  
  int getInterBeatIntervalMs() {
    return IBI;
  }
  
  int getPulseAmplitude() {
    return amp;
  }
  
  unsigned long getLastBeatTime() {
    return lastBeatTime;
  }
  
  boolean sawStartOfBeat() {
    // Disable interrupts to avoid a race with the ISR.
    boolean started = QS;
    QS = false;
    return started;
  }
  
  boolean isInsideBeat() {
    return Pulse;
  }
  
  void readNextSample() {
    // We assume assigning to an int is atomic.
    Signal = analogRead(InputPin);
  }
  
  void processLatestSample() {
    sampleCounter += sampleIntervalMs;         // keep track of the time in mS with this variable
    int N = sampleCounter - lastBeatTime;      // monitor the time since the last beat to avoid noise
  
    //  find the peak and trough of the pulse wave
    if (Signal < thresh && N > (IBI / 5) * 3) { // avoid dichrotic noise by waiting 3/5 of last IBI
      if (Signal < T) {                        // T is the trough
        T = Signal;                            // keep track of lowest point in pulse wave
      }
    }
  
    if (Signal > thresh && Signal > P) {       // thresh condition helps avoid noise
      P = Signal;                              // P is the peak
    }                                          // keep track of highest point in pulse wave
  
    //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
    // signal surges up in value every time there is a pulse
    if (N > 250) {                             // avoid high frequency noise
      if ( (Signal > thresh) && (Pulse == false) && (N > (IBI / 5) * 3) ) {
        Pulse = true;                          // set the Pulse flag when we think there is a pulse
        IBI = sampleCounter - lastBeatTime;    // measure time between beats in mS
        lastBeatTime = sampleCounter;          // keep track of time for next pulse
  
        if (secondBeat) {                      // if this is the second beat, if secondBeat == TRUE
          secondBeat = false;                  // clear secondBeat flag
          for (int i = 0; i <= 9; i++) {       // seed the running total to get a realisitic BPM at startup
            rate[i] = IBI;
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
  
        rate[9] = IBI;                          // add the latest IBI to the rate array
        runningTotal += rate[9];                // add the latest IBI to runningTotal
        runningTotal /= 10;                     // average the last 10 IBI values
        BPM = 60000 / runningTotal;             // how many beats can fit into a minute? that's BPM!
        QS = true;                              // set Quantified Self flag (we detected a beat)
      }
    }
  
    if (Signal < thresh && Pulse == true) {  // when the values are going down, the beat is over
      Pulse = false;                         // reset the Pulse flag so we can do it again
      amp = P - T;                           // get amplitude of the pulse wave
      thresh = amp / 2 + T;                  // set thresh at 50% of the amplitude
      P = thresh;                            // reset these for next time
      T = thresh;
    }
  
    if (N > 2500) {                          // if 2.5 seconds go by without a beat
      thresh = threshSetting;                // set thresh default
      P = 512;                               // set P default
      T = 512;                               // set T default
      lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date
      firstBeat = true;                      // set these to avoid noise
      secondBeat = false;                    // when we get the heartbeat back
    }
  }
};
