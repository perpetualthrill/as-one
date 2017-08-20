/*
  MQTT-based scoreboard reporting.

  It connects to an MQTT server then:
  - publishes:
      - "asOne/score/rightBPM": [0-199] current heartrate to display on right score
      - "asOne/score/leftBPM": [0-199] current heartrate to display on left score
  - subscribes:
      - "asOne/brain/#": multi-level wildcard
  - processes:
      - "asOne/brain/left": [integer] current left pulse interval
      - "asOne/brain/right": [integer] current right pulse interval

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Metro.h>
#include <Streaming.h>
#include <arduinoFFT.h>

WiFiClient espClient;
PubSubClient mqtt;

/*
  These values can be changed in order to evaluate the functions
*/
const uint16_t samples = 16; //This value MUST ALWAYS be a power of 2
double samplingFrequency = 5000;

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02

// track the left BPM
byte leftBPM = 100;
int leftValue = 0;
boolean leftUpdate = false;

// track the right BPM
byte rightBPM = 100;
byte rightValue = 0;
boolean rightUpdate = false;

// led pinouts and high/low definition for red one.
#define BLUE_LED 2
#define BLUE_ON HIGH
#define BLUE_OFF LOW
#define RED_LED 0
#define RED_ON LOW
#define RED_OFF HIGH

void setup() {
  Serial.begin(115200);
  Serial << endl << endl << F("Heart Rate. Startup.") << endl;

  pinMode(RED_LED, OUTPUT);     // Initialize the red LED pin as an output
  pinMode(BLUE_LED, OUTPUT);     // Initialize the blue LED pin as an output

  // don't allow the WiFi module to sleep.  this interacts with the FastLED library, so key.
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  mqtt.setClient(espClient);
  const char* mqtt_server = "asone-console";
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback);

  Serial << F("Heart Rate.  Startup complete.") << endl;

}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    // this is a blocking routine.  read: we halt until WiFi is connected
    connectWiFi();
  }
  if (!mqtt.connected()) {
    connectMQTT();
  } else {
    // look for a message
    mqtt.loop();

    // if we have an update, show it
    if ( leftUpdate ) computeBPM_Thresh_Left();
    if ( rightUpdate ) computeBPM_Thresh_Right();
    
    //    if ( haveUpdate ) computeBPM_FFT();
    // if ( haveUpdate ) computeBPM_PanTompkins();

    // update the scoreboard if there's a delta
    sendBPM();
  }
}

// the real meat of the work is done here, where we process messages.
void callback(char* topic, byte* payload, unsigned int length) {
  // toggle the blue LED when we get a new message
  static boolean ledState = false;
  ledState = !ledState;
  digitalWrite(RED_LED, ledState);

  // String class is much easier to work with
  String t = topic;
  String m = (char*)payload;

  // list of topics that we'll process
  const String msgLeft = "asOne/brain/left";
  const String msgRight = "asOne/brain/right";

  if (t.equals(msgLeft)) {
    leftValue = m.toInt();
    leftUpdate = true;
  } else if (t.equals(msgRight)) {
    rightValue = m.toInt();
    rightUpdate = true;
  } else {
    Serial << F(" WARNING. unknown topic. continuing.") << endl;
  }
}

void computeBPM_Thresh_Right() {
  // Magic Numbers
  const unsigned long minInterval = 60000UL / 200UL; // 200bpm -> 300ms interval
  const unsigned long maxInterval = 60000UL / 40UL; // 40bpm -> 1500ms interval
  const byte countThreshold = 3;
  const byte smoothing = 10;

  static unsigned long lastTrigger = millis();
  static byte countTrigger = 0;

  unsigned long now = millis();

  static unsigned long smoothedValue = rightValue;
  float triggerLevel = (float)smoothedValue * 1.10; // fold-increase to trigger
  // 1.10 too low
  // 1.25 too low
  // 1.50 too high
  // 1.375 touch too high

  static unsigned long smoothedBPM = rightBPM;

  // if the minimum interval is passed and we have a high postive excursion
  if ( (now - lastTrigger) >= minInterval && rightValue >= triggerLevel ) {
    // require N such events to cause a trigger
    countTrigger++;
    if ( countTrigger >= countThreshold ) {
      unsigned long delta = now - lastTrigger;
      lastTrigger = now;
      unsigned long bpm = 60000.0 / (float)delta;

      // if we're inside the maxInterval, probably not noise
      if ( delta <= maxInterval ) {
        smoothedBPM = (smoothedBPM * (smoothing - 1) + bpm) / smoothing;
        rightBPM = smoothedBPM;

        Serial << "Right trigger! val=" << rightValue << " trigger=" << triggerLevel;
        Serial << " bpm=" << bpm << " smoothed=" << smoothedBPM << endl;
      }
    }
  } else {
    countTrigger = 0;
    smoothedValue = (smoothedValue * (smoothing - 1) + rightValue) / smoothing;
  }

  Serial << "0,255," << rightValue << "," << smoothedValue << "," << countTrigger*100;
  Serial << "," << triggerLevel << "," << smoothedBPM << endl;

  rightUpdate = false;
}

void computeBPM_Thresh_Left() {
  // Magic Numbers
  const unsigned long minInterval = 60000UL / 200UL; // 200bpm -> 300ms interval
  const unsigned long maxInterval = 60000UL / 40UL; // 40bpm -> 1500ms interval
  const byte countThreshold = 3;
  const byte smoothing = 10;

  static unsigned long lastTrigger = millis();
  static byte countTrigger = 0;

  unsigned long now = millis();

  static unsigned long smoothedValue = leftValue;
  float triggerLevel = (float)smoothedValue * 1.10; // fold-increase to trigger
  // 1.10 too low
  // 1.25 too low
  // 1.50 too high
  // 1.375 touch too high

  static unsigned long smoothedBPM = leftBPM;

  // if the minimum interval is passed and we have a high postive excursion
  if ( (now - lastTrigger) >= minInterval && leftValue >= triggerLevel ) {
    // require N such events to cause a trigger
    countTrigger++;
    if ( countTrigger >= countThreshold ) {
      unsigned long delta = now - lastTrigger;
      lastTrigger = now;
      unsigned long bpm = 60000.0 / (float)delta;

      // if we're inside the maxInterval, probably not noise
      if ( delta <= maxInterval ) {
        smoothedBPM = (smoothedBPM * (smoothing - 1) + bpm) / smoothing;
        leftBPM = smoothedBPM;

        Serial << "Left trigger! val=" << leftValue << " trigger=" << triggerLevel;
        Serial << " bpm=" << bpm << " smoothed=" << smoothedBPM << endl;
      }
    }
  } else {
    countTrigger = 0;
    smoothedValue = (smoothedValue * (smoothing - 1) + leftValue) / smoothing;
  }

  Serial << "0,255," << leftValue << "," << smoothedValue << "," << countTrigger*100;
  Serial << "," << triggerLevel << "," << smoothedBPM << endl;

  leftUpdate = false;
}


// Pan-Tompkin algorihtm from: https://github.com/blakeMilner/real_time_QRS_detection/blob/master/QRS_arduino/QRS.ino
int tmp = 0;
void computeBPM_PanTompkins() {
  // timing variables
  static unsigned long previousMicros  = 0;        // will store last time LED was updated
  static unsigned long foundTimeMicros = 0;        // time at which last QRS was found
  static unsigned long old_foundTimeMicros = 0;        // time at which QRS before last was found
  static unsigned long currentMicros   = 0;        // current time
  static float bpm = 0;
  const byte BPM_BUFFER_SIZE = 5;

  static unsigned long bpm_buff[BPM_BUFFER_SIZE] = {0};
  static int bpm_buff_WR_idx = 0;
  static int bpm_buff_RD_idx = 0;

  boolean QRS_detected = detect(rightValue);

  if (QRS_detected == true) {
    foundTimeMicros = micros();

    bpm_buff[bpm_buff_WR_idx] = (60.0 / (((float) (foundTimeMicros - old_foundTimeMicros)) / 1000000.0));
    bpm_buff_WR_idx++;
    bpm_buff_WR_idx %= BPM_BUFFER_SIZE;

    bpm += bpm_buff[bpm_buff_RD_idx];

    tmp = bpm_buff_RD_idx - BPM_BUFFER_SIZE + 1;
    if (tmp < 0) tmp += BPM_BUFFER_SIZE;

    bpm -= bpm_buff[tmp];

    bpm_buff_RD_idx++;
    bpm_buff_RD_idx %= BPM_BUFFER_SIZE;

    old_foundTimeMicros = foundTimeMicros;

    rightBPM = bpm;
    Serial << bpm << ",";
    for(byte i=0;i<BPM_BUFFER_SIZE;i++) Serial << "," << bpm_buff[i];
    Serial << endl;
  }
  rightUpdate = false;
}


#define M       5
#define N       30
#define winSize     250
#define HP_CONSTANT   ((float) 1 / (float) M)
#define MAX_BPM     200

// circular buffer for input ecg signal
// we need to keep a history of M + 1 samples for HP filter
float ecg_buff[M + 1] = {0};
int ecg_buff_WR_idx = 0;
int ecg_buff_RD_idx = 0;

// circular buffer for input ecg signal
// we need to keep a history of N+1 samples for LP filter
float hp_buff[N + 1] = {0};
int hp_buff_WR_idx = 0;
int hp_buff_RD_idx = 0;

// LP filter outputs a single point for every input point
// This goes straight to adaptive filtering for eval
float next_eval_pt = 0;

// running sums for HP and LP filters, values shifted in FILO
float hp_sum = 0;
float lp_sum = 0;

// working variables for adaptive thresholding
float treshold = 0;
boolean triggered = false;
int trig_time = 0;
float win_max = 0;
int win_idx = 0;

// numebr of starting iterations, used determine when moving windows are filled
int number_iter = 0;

// resolution of RNG
#define RAND_RES 100000000


boolean detect(float new_ecg_pt) {
  // copy new point into circular buffer, increment index
  ecg_buff[ecg_buff_WR_idx++] = new_ecg_pt;
  ecg_buff_WR_idx %= (M + 1);


  /* High pass filtering */
  if (number_iter < M) {
    // first fill buffer with enough points for HP filter
    hp_sum += ecg_buff[ecg_buff_RD_idx];
    hp_buff[hp_buff_WR_idx] = 0;
  } else {
    hp_sum += ecg_buff[ecg_buff_RD_idx];

    tmp = ecg_buff_RD_idx - M;
    if (tmp < 0) tmp += M + 1;

    hp_sum -= ecg_buff[tmp];

    float y1 = 0;
    float y2 = 0;

    tmp = (ecg_buff_RD_idx - ((M + 1) / 2));
    if (tmp < 0) tmp += M + 1;

    y2 = ecg_buff[tmp];

    y1 = HP_CONSTANT * hp_sum;

    hp_buff[hp_buff_WR_idx] = y2 - y1;
  }

  // done reading ECG buffer, increment position
  ecg_buff_RD_idx++;
  ecg_buff_RD_idx %= (M + 1);

  // done writing to HP buffer, increment position
  hp_buff_WR_idx++;
  hp_buff_WR_idx %= (N + 1);


  /* Low pass filtering */

  // shift in new sample from high pass filter
  lp_sum += hp_buff[hp_buff_RD_idx] * hp_buff[hp_buff_RD_idx];

  if (number_iter < N) {
    // first fill buffer with enough points for LP filter
    next_eval_pt = 0;

  }
  else {
    // shift out oldest data point
    tmp = hp_buff_RD_idx - N;
    if (tmp < 0) tmp += (N + 1);

    lp_sum -= hp_buff[tmp] * hp_buff[tmp];

    next_eval_pt = lp_sum;
  }

  // done reading HP buffer, increment position
  hp_buff_RD_idx++;
  hp_buff_RD_idx %= (N + 1);


  /* Adapative thresholding beat detection */
  // set initial threshold
  if (number_iter < winSize) {
    if (next_eval_pt > treshold) {
      treshold = next_eval_pt;
    }

    // only increment number_iter iff it is less than winSize
    // if it is bigger, then the counter serves no further purpose
    number_iter++;
  }

  // check if detection hold off period has passed
  if (triggered == true) {
    trig_time++;

    if (trig_time >= 100) {
      triggered = false;
      trig_time = 0;
    }
  }

  // find if we have a new max
  if (next_eval_pt > win_max) win_max = next_eval_pt;

  // find if we are above adaptive threshold
  if (next_eval_pt > treshold && !triggered) {
    triggered = true;

    return true;
  }
  // else we'll finish the function before returning FALSE,
  // to potentially change threshold

  // adjust adaptive threshold using max of signal found
  // in previous window
  if (win_idx++ >= winSize) {
    // weighting factor for determining the contribution of
    // the current peak value to the threshold adjustment
    float gamma = 0.175;

    // forgetting factor -
    // rate at which we forget old observations
    // choose a random value between 0.01 and 0.1 for this,
    float alpha = 0.01 + ( ((float) random(0, RAND_RES) / (float) (RAND_RES)) * ((0.1 - 0.01)));

    // compute new threshold
    treshold = alpha * gamma * win_max + (1 - alpha) * treshold;

    // reset current window index
    win_idx = 0;
    win_max = -10000000;
  }

  // return false if we didn't detect a new QRS
  return false;

}
void computeBPM_FFT() {
  static unsigned long tic = millis();

  /* Build raw data */
  static byte ringIndex = 0;
  static byte ringBuffer[samples];
  ringBuffer[ringIndex] = rightValue; // build up the data

  // reset
  rightUpdate = false;

  // loop through to get more samples
  if (ringIndex != samples - 1) {
    ringIndex++;
    return;
  }
  // reset the ring buffer.
  ringIndex = 0;
  unsigned long toc = millis();

  //  samplingFrequency = (float)samples*1000UL/(float)(toc-tic);
  samplingFrequency = 1000.0 / 16.0;
  Serial << "freq: " << samplingFrequency << " delta: " << toc - tic << endl;
  tic = toc;

  arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

  /*
    These are the input and output vectors
    Input vectors receive computed results from FFT
  */
  double vReal[samples];
  double vImag[samples];

  // load the data
  for ( byte i = 0; i < samples; i++ ) vReal[i] = ringBuffer[i];

  /* Print the results of the simulated sampling according to time */
  //  Serial.println("Data:");
  //  PrintVector(vReal, samples, SCL_TIME);
  //  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  /* Weigh data */
  //  Serial.println("Weighed data:");
  //  PrintVector(vReal, samples, SCL_TIME);
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
  //  Serial.println("Computed Real values:");
  //  PrintVector(vReal, samples, SCL_INDEX);
  //  Serial.println("Computed Imaginary values:");
  //  PrintVector(vImag, samples, SCL_INDEX);
  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
  //  Serial.println("Computed magnitudes:");
  //  PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);
  double x = FFT.MajorPeak(vReal, samples, samplingFrequency);
  Serial.println(x, 6);

}


void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{

  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
        break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
        break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / samples);
        break;
    }
    Serial.print(abscissa, 6);
    Serial.print(" ");
    Serial.print(vData[i], 4);
    Serial.println();
  }
  Serial.println();
}

void sendBPM() {
  // track sends so we don't spam the scoreboard
  static byte lastLeftBPM = leftBPM;
  static byte lastRightBPM = rightBPM;

  // publish new BPM, if needed.
  const char* pubRight = "asOne/score/rightBPM";
  const char* pubLeft = "asOne/score/leftBPM";
  static char msg[32];

  if ( leftBPM != lastLeftBPM ) {
//    itoa(leftBPM, msg, 10);
//    mqtt.publish(pubLeft, msg);
    byte *m=&leftBPM;
    mqtt.publish(pubLeft, m, 1);
    lastLeftBPM = leftBPM;
  }
  if ( rightBPM != lastRightBPM ) {
//    itoa(rightBPM, msg, 10);
//    mqtt.publish(pubRight, msg);
    byte *m=&rightBPM;
    mqtt.publish(pubRight, m, 1);
    lastRightBPM = rightBPM;
  }
}

void connectWiFi() {
  digitalWrite(RED_LED, RED_ON);
  delay(10);

  // Update these.
  //  const char* ssid = "Looney_Ext";
  //  const char* password = "TinyandTooney";
  const char* ssid = "AsOne";
  const char* password = "fuckthapolice";

  // We start by connecting to a WiFi network
  Serial << endl;
  Serial << F("Connecting to ") << ssid << endl;

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial << F(".");
  }
  Serial << endl;

  Serial << F("WiFi connected") << endl;
  Serial << F("IP address: ") << WiFi.localIP() << endl;;
  digitalWrite(RED_LED, RED_OFF);
}


void connectMQTT() {
  digitalWrite(RED_LED, RED_ON);

  const char* id = "asOneHeartRate";
  const char* sub = "asOne/brain/#";

  static Metro connectInterval(500UL);
  if ( connectInterval.check() ) {

    Serial << F("Attempting MQTT connection...") << endl;
    // Attempt to connect
    if (mqtt.connect(id)) {
      mqtt.publish("asOne/hello", "hello from heartrate monitor"

      Serial << F("Connected.") << endl;
      // subscribe
      Serial << F("Subscribing: ") << sub << endl;
      mqtt.subscribe(sub);
      digitalWrite(RED_LED, RED_OFF);

    } else {
      Serial << F("Failed. state=") << mqtt.state() << endl;
    }

    connectInterval.reset();
  }
}

