/*
  MQTT-based scoreboard reporting.

  It connects to an MQTT server then:
  - publishes:
      - "asOne/score/rightBPM": [0-199] current heartrate to display on right score
      - "asOne/score/leftBPM": [0-199] current heartrate to display on left score
      - "asOne/fe/doHeartbeat": [integer] do flame effect at right BPM 
  - subscribes:
      - "asOne/brain/#": multi-level wildcard
  - processes:
      - "asOne/brain/left": [integer] current left pulse interval
      - "asOne/brain/right": [integer] current right pulse interval
      - "asOne/brain/button": [String] button state

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Metro.h>
#include <Streaming.h>

WiFiClient espClient;
PubSubClient mqtt;

// track the left BPM
byte leftBPM = 100;
int leftValue = 0;
boolean leftUpdate = false;
unsigned long leftLastTrigger = millis();
byte leftCountTrigger = 0;
unsigned long leftSmoothedValue = leftValue;
unsigned long leftSmoothedBPM = leftBPM;

// track the right BPM
byte rightBPM = 100;
byte rightValue = 0;
boolean rightUpdate = false;
unsigned long rightLastTrigger = millis();
byte rightCountTrigger = 0;
unsigned long rightSmoothedValue = rightValue;
unsigned long rightSmoothedBPM = rightBPM;

// Constants
const unsigned long MIN_INTERVAL = 60000UL / 200UL; // 200bpm -> 300ms interval
const unsigned long MAX_INTERVAL = 60000UL / 40UL; // 40bpm -> 1500ms interval
const byte COUNT_THRESHOLD = 3;
const char* RIGHT_TOPIC = "asOne/score/rightBPM";
const char* LEFT_TOPIC = "asOne/score/leftBPM";
const char* FE_TOPIC = "asOne/fe/doHeartbeat";

const byte SMOOTHING_INT = 10;
const float SMOOTHING_FLOAT = 1.10; // fold-increase to trigger
  // 1.10 too low
  // 1.25 too low
  // 1.50 too high
  // 1.375 touch too high

// led pinouts and high/low definition for red one.
#define BLUE_LED 2
#define BLUE_ON HIGH
#define BLUE_OFF LOW
#define RED_LED 0
#define RED_ON LOW
#define RED_OFF HIGH

void setup() {
  Serial.begin(38400);
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
  String m = String((char *)payload);

  // list of topics that we'll process
  const String msgLeft = "asOne/brain/left";
  const String msgRight = "asOne/brain/right";
  const String msgButton = "asOne/brain/button";

  if (t.equals(msgLeft)) {
    leftValue = m.toInt();
    leftUpdate = true;
  } else if (t.equals(msgRight)) {
    rightValue = m.toInt();
    rightUpdate = true;
  } else if (t.equals(msgButton)) {
    processButtonMessage(m);
  } else {
    Serial << F(" WARNING. unknown topic. continuing.") << endl;
  }
}

void processButtonMessage(String payload) {
  Serial << "process button: " << payload << endl;
  if(payload.startsWith("ON")) {
    String fred = "";
    fred += rightBPM;
    mqtt.publish(FE_TOPIC, fred.c_str());
    Serial << "Triggering flame effect at " << fred << endl;
  }
}

void computeBPM_Thresh(unsigned long smoothedValue, unsigned long lastTrigger, byte value, byte countTrigger,
                       byte bpm, unsigned long smoothedBPM) {
  unsigned long now = millis();
  float triggerLevel = (float)smoothedValue * SMOOTHING_FLOAT;

  // if the minimum interval is passed and we have a high postive excursion
  if ( (now - lastTrigger) >= MIN_INTERVAL && value >= triggerLevel ) {
    // require N such events to cause a trigger
    countTrigger++;
    if ( countTrigger >= COUNT_THRESHOLD ) {
      unsigned long delta = now - lastTrigger;
      lastTrigger = now;
      unsigned long foundBpm = 60000.0 / (float)delta;

      // if we're inside the MAX_INTERVAL, probably not noise
      if ( delta <= MAX_INTERVAL ) {
        smoothedBPM = (smoothedBPM * (SMOOTHING_INT - 1) + foundBpm) / SMOOTHING_INT;
        bpm = smoothedBPM;

        Serial << "Right trigger! val=" << rightValue << " trigger=" << triggerLevel;
        Serial << " bpm=" << foundBpm << " smoothed=" << smoothedBPM << endl;
      }
    }
  } else {
    rightCountTrigger = 0;
    rightSmoothedValue = (rightSmoothedValue * (SMOOTHING_INT - 1) + rightValue) / SMOOTHING_INT;
  }

 // Serial << "0,255," << rightValue << "," << smoothedValue << "," << countTrigger*100;
 // Serial << "," << triggerLevel << "," << smoothedBPM << endl;

  rightUpdate = false;

}

void computeBPM_Thresh_Right() {
  unsigned long now = millis();
  float triggerLevel = (float)rightSmoothedValue * SMOOTHING_FLOAT;

  // if the minimum interval is passed and we have a high postive excursion
  if ( (now - rightLastTrigger) >= MIN_INTERVAL && rightValue >= triggerLevel ) {
    // require N such events to cause a trigger
    rightCountTrigger++;
    if ( rightCountTrigger >= COUNT_THRESHOLD ) {
      unsigned long delta = now - rightLastTrigger;
      rightLastTrigger = now;
      unsigned long bpm = 60000.0 / (float)delta;

      // if we're inside the MAX_INTERVAL, probably not noise
      if ( delta <= MAX_INTERVAL ) {
        rightSmoothedBPM = (rightSmoothedBPM * (SMOOTHING_INT - 1) + bpm) / SMOOTHING_INT;
        rightBPM = rightSmoothedBPM;

        Serial << "Right trigger! val=" << rightValue << " trigger=" << triggerLevel;
        Serial << " bpm=" << bpm << " smoothed=" << rightSmoothedBPM << endl;
      }
    }
  } else {
    rightCountTrigger = 0;
    rightSmoothedValue = (rightSmoothedValue * (SMOOTHING_INT - 1) + rightValue) / SMOOTHING_INT;
  }

//  Serial << "0,255," << rightValue << "," << rightSmoothedValue << "," << rightCountTrigger*100;
//  Serial << "," << triggerLevel << "," << rightSmoothedBPM << endl;

  rightUpdate = false;
}

void computeBPM_Thresh_Left() {
  unsigned long now = millis();
  float triggerLevel = (float)leftSmoothedValue * SMOOTHING_FLOAT;

  // if the minimum interval is passed and we have a high postive excursion
  if ( (now - leftLastTrigger) >= MIN_INTERVAL && leftValue >= triggerLevel ) {
    // require N such events to cause a trigger
    leftCountTrigger++;
    if ( leftCountTrigger >= COUNT_THRESHOLD ) {
      unsigned long delta = now - leftLastTrigger;
      leftLastTrigger = now;
      unsigned long bpm = 60000.0 / (float)delta;

      // if we're inside the MAX_INTERVAL, probably not noise
      if ( delta <= MAX_INTERVAL ) {
        leftSmoothedBPM = (leftSmoothedBPM * (SMOOTHING_INT - 1) + bpm) / SMOOTHING_INT;
        leftBPM = leftSmoothedBPM;

        Serial << "Left trigger! val=" << leftValue << " trigger=" << triggerLevel;
        Serial << " bpm=" << bpm << " smoothed=" << leftSmoothedBPM << endl;
      }
    }
  } else {
    leftCountTrigger = 0;
    leftSmoothedValue = (leftSmoothedValue * (SMOOTHING_INT - 1) + leftValue) / SMOOTHING_INT;
  }

//  Serial << "0,255," << leftValue << "," << leftSmoothedValue << "," << leftCountTrigger*100;
//  Serial << "," << triggerLevel << "," << leftSmoothedBPM << endl;

  leftUpdate = false;
}

void sendBPM() {
  // track sends so we don't spam the scoreboard
  static byte lastLeftBPM = leftBPM;
  static byte lastRightBPM = rightBPM;

  // publish new BPM, if needed.
  if ( leftBPM != lastLeftBPM ) {
    byte *m=&leftBPM;
    mqtt.publish(LEFT_TOPIC, m, 1);
    lastLeftBPM = leftBPM;
  }
  if ( rightBPM != lastRightBPM ) {
    byte *m=&rightBPM;
    mqtt.publish(RIGHT_TOPIC, m, 1);
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
      mqtt.publish("asOne/hello", "hello from heartrate monitor");

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

