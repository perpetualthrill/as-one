// Teensy 3.2 
// Serial1: Pin 0 = RX, Pin 1 = TX
// Serial2: RX on 9, TX on 10

// Sampling related constants
const int SAMPLE_RATE_HZ = 90;
const int SAMPLE_DELAY_MS = 1000 / SAMPLE_RATE_HZ;
const int FIFO_SIZE = SAMPLE_RATE_HZ / 2; // half second buffer
const int LATEST_SIZE = 3;
const int GOOD_BEAT_QUEUE_SIZE = 3;
const float MAX_BPM_DIFFERENCE_PERCENT = .10;

// Timing related constants
const int MS_PER_SECOND = 60000;
const int MAX_HEARTRATE_BPM = 130;
const int MIN_WINDOW_MS = MS_PER_SECOND / MAX_HEARTRATE_BPM;
const int MIN_HEARTRATE_BPM = 50;
const int MAX_WINDOW_MS = MS_PER_SECOND / MIN_HEARTRATE_BPM;

// Magic number: a large enough difference from average to trigger
// beat detection. Empirically gathered.
const int DIFFERENCE_THRESHOLD = 12;

// known-bad identifier
const int SENTINEL = 9999;

// globals
int latest[] = { SENTINEL, SENTINEL };
int fifo[FIFO_SIZE];
int lastBeatMs = 0;
int goodBeatTimes[GOOD_BEAT_QUEUE_SIZE];

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(38400);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("HELLO ON SERIAL");

  // set the data rate for the olimex uart
  Serial1.begin(115200);

  // zero out found beats
  for (int i = 0; i < GOOD_BEAT_QUEUE_SIZE; i++) {
    goodBeatTimes[i] = 0;
  }

  // debounce
  delay(10);
}

void checkAndEnqueue(int value, int buff[], int buffSize) {
  for (int i = buffSize - 1; i > 0; i--) {
    buff[i] = buff[i - 1];
  }
  if ((value > 250) || (value < 5)) {
    // user is disconnected or data is otherwise faulty
    buff[0] = SENTINEL;
  } else {
    buff[0] = value;
  }
}

int bufferAverage(int buff[], int buffSize) {
  int sum = 0;
  for (int i = 0; i < buffSize; i++) {
    if (buff[i] == SENTINEL) {
      return -1;
    }
    sum += buff[i];
  }
  return sum / buffSize;
}

bool detectBeat() {
  int fifoAvg = bufferAverage(fifo, FIFO_SIZE);
  int latestAvg = bufferAverage(latest, LATEST_SIZE);
  if ((latestAvg - fifoAvg) > DIFFERENCE_THRESHOLD) {
    return true;
  }
  return false;
}

void bufferValue(int val) {
  checkAndEnqueue(val, latest, LATEST_SIZE);
  checkAndEnqueue(val, fifo, FIFO_SIZE);
}

bool checkFifoForSentinel() {
  for (int i = 0; i < FIFO_SIZE; i++) {
    if (fifo[i] == SENTINEL) {
      return true;
    }
  }
  return false;
}

void enqueueGoodBeat(int time) {
  for (int i = GOOD_BEAT_QUEUE_SIZE - 1; i > 0; i--) {
    goodBeatTimes[i] = goodBeatTimes[i - 1];
  }
  goodBeatTimes[0] = time;
}

bool giveFeedbackForBeat() {
  int bpms[GOOD_BEAT_QUEUE_SIZE-1];
  for (int i = 0; i < GOOD_BEAT_QUEUE_SIZE - 1; i++) {
    bpms[i] = MS_PER_SECOND / (goodBeatTimes[i] - goodBeatTimes[i+1]);
  }
  for (int i = 0; i < GOOD_BEAT_QUEUE_SIZE - 1; i++) {
    // Serial.println(bpms[i]);
    if (bpms[i] > MAX_HEARTRATE_BPM || bpms[i] < MIN_HEARTRATE_BPM) {
      return false;
    }
  }
  for (int i = 0; i < GOOD_BEAT_QUEUE_SIZE - 2; i++) {
    float higher = (float) max(bpms[i], bpms[i+1]);
    float lower = (float) min(bpms[i], bpms[i+1]);
    // Serial.println((higher / lower)-1);
    if (((higher / lower) - 1) > MAX_BPM_DIFFERENCE_PERCENT) {
      return false;
    }
  }
  return true;
}

void loop() {
  // Get value from mod-ekg
  if (Serial1.available()) {
    int bytecount = 0;
    int val = -1;
    while (Serial1.available()) {
      val = Serial1.read();
      bytecount++;
    }
    Serial1.clear();

    // output reading to USB
    Serial.print("RIGHT: ");
    Serial.println(val);

    // check for a detected beat
    int currentTime = millis();
    bufferValue(val);
    if (!checkFifoForSentinel() && detectBeat()) {
      int interval = currentTime - lastBeatMs;
      lastBeatMs = currentTime;
      if (interval > MIN_WINDOW_MS && interval < MAX_WINDOW_MS) {
        enqueueGoodBeat(currentTime);
        if (giveFeedbackForBeat()) {
          Serial.print("BEAT: ");
          Serial.println(MS_PER_SECOND / interval);
          Serial.println("FEEDBACK");
        } else {
          Serial.println("NOBEAT");
        }
      }
    }
  }
  delay(SAMPLE_DELAY_MS);
}
