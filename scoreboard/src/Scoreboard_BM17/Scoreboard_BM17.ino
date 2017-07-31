/*
  MQTT-based scoreboard reporting.

  It connects to an MQTT server then:
  - publishes:
      - "asOne/score/heartbeat": current millis() every two seconds
  - subscribes:
      - "asOne/score/#": multi-level wildcard
  - processes:
      - "asOne/score/state": [0,1,2] current state of the system
        - 0: idle/not playing.  scoreboard will mess around on its own.
        - 1: active/playing.  scoreboard will process the following subscriptions
        - 2: won/flames.  scoreboard will do some (garish) fanfare while the flames are shooting
      - "asOne/score/logo": bytestream castable to CRGB, applied to all LEDs in logo
        - "asOne/score/logo/direct": bytestream castable to CRGB[22]
      - "asOne/score/timer": [0-99] current timer to display on the countdown
        - "asOne/score/timer/direct: bytestream castable to CRGB[26]
      - "asOne/score/leftBPM": [0-199] current heartrate to display on left score
        - "asOne/score/leftBPM/direct": bytestream castable to CRGB[47]
      - "asOne/score/rightBPM": [0-199] current heartrate to display on right score
        - "asOne/score/rightBPM/direct": bytestream castable to CRGB[47]

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Metro.h>
#include <Streaming.h>
#include <FastLED.h>

WiFiClient espClient;
PubSubClient mqtt;

// track the state
byte state = 0;

// track the logo
const byte nLogoLED = 22;
const byte startLogo = 47;
const byte stopLogo = startLogo + nLogoLED - 1;

// track the timer
const byte nTimerLED = 26;
const byte startTimer = 71;
const byte stopTimer = startTimer + nTimerLED - 1;

// track the left BPM
const byte nLeftLED = 47;
const byte startLeft = 98;
const byte stopLeft = startLeft + nLeftLED - 1;
// track the right BPM
const byte nRightLED = nLeftLED;
const byte startRight = 0;
const byte stopRight = startRight + nRightLED - 1;

// overall LED array
const byte nWastedLED = 2;
CRGBArray < nWastedLED + nLogoLED + nTimerLED + nLeftLED + nRightLED > leds;

// track the need to do a FastLED.show();
boolean haveUpdate = false;

// led pinouts and high/low definition for red one.
#define BLUE_LED 2
#define RED_LED 0
#define RED_ON LOW
#define RED_OFF HIGH

// publish a heartbeat on this interval
#define HEARTBEAT_INTERVAL 2000UL // ms

void setup() {
  Serial.begin(115200);
  Serial << endl << endl << F("Scoreboard. Startup.") << endl;

  pinMode(BLUE_LED, OUTPUT);     // Initialize the blue LED pin as an output
  pinMode(RED_LED, OUTPUT);     // Initialize the  red LED pin as an output

  mqtt.setClient(espClient);
  const char* mqtt_server = "broker.mqtt-dashboard.com";
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback);

  CRGB foo[nLeftLED];
  Serial << F("packet size must be >=") << sizeof(foo) << endl;
  Serial << F("packet size is=") << MQTT_MAX_PACKET_SIZE << endl;

  Serial << F("Scoreboard.  Startup complete.") << endl;
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
    if ( haveUpdate ) showScoreboard();

    // send a heartbeat on an interval heartbeat interval
    heartbeatMQTT();

  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  // toggle the blue LED when we get a new message
  static boolean ledState = false;
  ledState = !ledState;
  digitalWrite(BLUE_LED, ledState);

  // String class is much easier to work with
  String t = topic;

  // drop our own heartbeat
  const String msgHeartbeat = "asOne/score/heartbeat";
  if ( t.equals(msgHeartbeat) ) return;

  Serial << F("<- ") << t;

  // we have an update, unless otherwise noted
  haveUpdate = true;

  // list of topics that we'll process
  const String msgState = "asOne/score/state";
  const String msgLogo = "asOne/score/logo";
  const String msgLogoDirect = "asOne/score/logo/direct";
  const String msgTimer = "asOne/score/timer";
  const String msgTimerDirect = "asOne/score/timer/direct";
  const String msgLeft = "asOne/score/leftBPM";
  const String msgLeftDirect = "asOne/score/leftBPM/direct";
  const String msgRight = "asOne/score/rightBPM";
  const String msgRightDirect = "asOne/score/rightBPM/direct";

  if ( t.equals(msgState) ) {
    state = String((char*)payload).toInt();
    Serial << F(" = ") << state;
  } else if (t.equals(msgLogo)) {
    leds(startLogo, stopLogo) = CRGBSet( (CRGB*)payload, 1 );

    Serial << F(" =");
    Serial << F(" R:") << leds[startLogo].red;
    Serial << F(" G:") << leds[startLogo].green;
    Serial << F(" B:") << leds[startLogo].blue;

  } else if (t.equals(msgLogoDirect)) {
    leds(startLogo, stopLogo) = CRGBSet( (CRGB*)payload, nLogoLED );

//    Serial << F(" =");
//    Serial << F(" R:") << leds[startLogo + 4].red;
//    Serial << F(" G:") << leds[startLogo + 4].green;
//    Serial << F(" B:") << leds[startLogo + 4].blue;

  } else if (t.equals(msgTimer)) {
  } else if (t.equals(msgTimerDirect)) {
    leds(startTimer, stopTimer) = CRGBSet( (CRGB*)payload, nTimerLED );
  } else if (t.equals(msgLeft)) {
  } else if (t.equals(msgLeftDirect)) {
    leds(startLeft, stopLeft) = CRGBSet( (CRGB*)payload, nLeftLED );
  } else if (t.equals(msgRight)) {
  } else if (t.equals(msgRightDirect)) {
    leds(startRight, stopRight) = CRGBSet( (CRGB*)payload, nRightLED );
  } else {
    haveUpdate = false;
    Serial << F(" WARNING. unknown topic. continuing.");
  }

  Serial << endl;
}

void showScoreboard() {
  haveUpdate = false;
}

void connectWiFi() {
  digitalWrite(RED_LED, RED_ON);
  delay(10);

  // Update these.
  const char* ssid = "Looney_Ext";
  const char* password = "TinyandTooney";

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

  const char* id = "asOneScoreboard";
  const char* sub = "asOne/score/#";

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

void heartbeatMQTT() {
  const char* pub = "asOne/score/heartbeat";
  static char msg[32];
  static Metro heartbeatInterval(HEARTBEAT_INTERVAL);

  if ( heartbeatInterval.check() ) {
    itoa(millis(), msg, 10);
    mqtt.publish(pub, msg);
    heartbeatInterval.reset();
  }
}

