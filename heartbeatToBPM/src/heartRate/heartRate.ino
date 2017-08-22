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

WiFiClient espClient;
PubSubClient mqtt;

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

void sendBPM() {
  // track sends so we don't spam the scoreboard
  static byte lastLeftBPM = leftBPM;
  static byte lastRightBPM = rightBPM;

  // publish new BPM, if needed.
  const char* pubRight = "asOne/score/rightBPM";
  const char* pubLeft = "asOne/score/leftBPM";
  static char msg[32];

  if ( leftBPM != lastLeftBPM ) {
    byte *m=&leftBPM;
    mqtt.publish(pubLeft, m, 1);
    lastLeftBPM = leftBPM;
  }
  if ( rightBPM != lastRightBPM ) {
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

