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
int leftInterval = 0;

// track the right BPM
byte rightBPM = 100;
byte rightValue = 0;

// track need for update
boolean haveUpdate = false;

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
    if ( haveUpdate ) computeBPM();
    //    if ( haveUpdate ) sendBPM();
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

  // we have an update, unless otherwise noted
  haveUpdate = true;

  // list of topics that we'll process
  const String msgLeft = "asOne/brain/left";
  const String msgRight = "asOne/brain/right";

  if (t.equals(msgLeft)) {
    leftInterval = m.toInt();
  } else if (t.equals(msgRight)) {
    rightValue = m.toInt();
    //    Serial << rightValue << ",0,255" << endl;
  } else {
    haveUpdate = false;
    Serial << F(" WARNING. unknown topic. continuing.") << endl;
  }
}

void computeBPM() {
  static unsigned long lastTrigger = millis();
  unsigned long now = millis();
  
  const unsigned long maxInterval = 60000UL/40UL; // 40bpm -> 1500ms interval
  const unsigned long minInterval = 60000UL/200UL; // 200bpm -> 300ms interval
 
  static unsigned long smoothedBPM = rightValue;
  float triggerLevel = (float)smoothedBPM*1.25; // fold-increase to trigger
  // 1.10 too low
  // 1.25 too low
  // 1.50 too high
  // 1.375 touch too high
  
  if( (now-lastTrigger)>= minInterval && rightValue >= triggerLevel ) {
    unsigned long delta = now - lastTrigger;
    lastTrigger = now;
    float bpm = 60000.0/(float)delta;
    const byte smoothing = 10;
    smoothedBPM = (smoothedBPM*(smoothing-1)+bpm)/smoothing;
    rightBPM = smoothedBPM;
    Serial << "Trigger! val=" << rightValue << " trigger=" << triggerLevel;
    Serial << " smoothed=" << rightBPM << " bpm=" << bpm << endl;
  } 

  haveUpdate=false;
}

void computeBPM_Old() {
  static unsigned long tic = millis();
  
  /* Build raw data */
  static byte ringIndex = 0;
  static byte ringBuffer[samples];
  ringBuffer[ringIndex] = rightValue; // build up the data

  // reset 
  haveUpdate = false;
  
  // loop through to get more samples
  if (ringIndex != samples - 1) {
    ringIndex++;
    return;
  }
  // reset the ring buffer.
  ringIndex = 0;
  unsigned long toc = millis();
  
//  samplingFrequency = (float)samples*1000UL/(float)(toc-tic);
  samplingFrequency = 1000.0/16.0;
  Serial << "freq: " << samplingFrequency << " delta: " << toc-tic << endl;
  tic = toc;

  arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

  /*
    These are the input and output vectors
    Input vectors receive computed results from FFT
  */
  double vReal[samples];
  double vImag[samples];

  // load the data
  for( byte i=0;i<samples;i++ ) vReal[i] = ringBuffer[i];
  
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
  // promote bit size on tracking information to ease smoothing calculations
  static unsigned long lastLeftBPM = leftBPM;
  static unsigned long lastRightBPM = leftBPM;

  static unsigned long lastLeftInterval = leftInterval;
  static unsigned long lastrightValue = rightValue;

  unsigned long newLeftInterval = lastLeftInterval;
  unsigned long newrightValue = lastrightValue;
  unsigned long newLeftBPM = lastLeftBPM;
  unsigned long newRightBPM = lastRightBPM;

  // compute new intervals, if needed
  // apply exponential smoothing to the interval information.
  const byte intervalSmoothing = 5; // 1 for no smoothing.
  if ( rightValue != lastrightValue ) {
    newLeftInterval = (lastLeftInterval * (intervalSmoothing - 1) + rightValue) / intervalSmoothing;
  }
  if ( rightValue != lastrightValue ) {
    newrightValue = (lastrightValue * (intervalSmoothing - 1) + rightValue) / intervalSmoothing;
  }

  // compute new BPM, if needed
  // apply exponential smoothing to the interval information.
  const byte bpmSmoothing = 5; // 1 for no smoothing.
  if ( newrightValue != lastrightValue ) {
    unsigned long newBPM = 60000UL / newrightValue; // ms/beat -> beat/min
    newRightBPM = (lastRightBPM * (bpmSmoothing - 1) + newBPM) / bpmSmoothing;
    lastrightValue = newrightValue;
  }
  if ( newLeftInterval != lastLeftInterval ) {
    unsigned long newBPM = 60000UL / newLeftInterval; // ms/beat -> beat/min
    newLeftBPM = (lastLeftBPM * (bpmSmoothing - 1) + newBPM) / bpmSmoothing;
    lastLeftInterval = newLeftInterval;
  }

  // publish new BPM, if needed.
  const char* pubRight = "asOne/score/rightBPM";
  const char* pubLeft = "asOne/score/leftBPM";
  static char msg[32];

  if ( newLeftBPM != lastLeftBPM ) {
    itoa(newLeftBPM, msg, 10);
    mqtt.publish(pubLeft, msg);
    lastLeftBPM = newLeftBPM;
  }
  if ( newRightBPM != lastRightBPM ) {
    itoa(newRightBPM, msg, 10);
    mqtt.publish(pubRight, msg);
    lastRightBPM = lastLeftBPM;
  }

  // we processed the update
  haveUpdate = false;
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

