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
const byte nBPMLED = 47;
const byte startLeft = 98;
const byte stopLeft = startLeft + nBPMLED - 1;
// track the right BPM
const byte startRight = 0;
const byte stopRight = startRight + nBPMLED - 1;

// overall LED array
const byte nWastedLED = 2;
CRGBArray < nWastedLED + nLogoLED + nTimerLED + nBPMLED + nBPMLED > leds;

// send buffer
char msg[MQTT_MAX_PACKET_SIZE];

// led pinouts and high/low definition for red one.
#define BLUE_LED 2
#define RED_LED 0
#define RED_ON LOW
#define RED_OFF HIGH

// publish a heartbeat on this interval
#define SEND_INTERVAL 500UL // ms

void setup() {
  Serial.begin(115200);
  pinMode(BLUE_LED, OUTPUT);     // Initialize the blue LED pin as an output
  pinMode(RED_LED, OUTPUT);     // Initialize the  red LED pin as an output

  mqtt.setClient(espClient);
  const char* mqtt_server = "broker.mqtt-dashboard.com";
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback);
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

    // send update periodically
    static Metro updateInterval(SEND_INTERVAL);
    if ( updateInterval.check() ) {
      // testing state
      //      state += 1;
      //      if( state > 2 ) state=0;
      //      sendState();

      static byte sendWhat = 99;
      sendWhat += 1;
      if(sendWhat > 3) sendWhat=0;

      if(sendWhat==0) {

        // testing msgLogo
        static byte hue = 0;
        hue = hue + 10;
        CRGB pixel = CHSV(hue, 255, 255);
        sendLogo(pixel);
      }
      
      // testing msgLogoDirect
      //      static byte hue;
      //      hue++;
      //      CRGB pixels[nLogoLED];
      //      fill_rainbow(pixels, nLogoLED, hue);
      //      sendLogoDirect(pixels);

      if(sendWhat==1) {
        // testing msgTimer
        static byte timer = 99;
        timer--;
        if (timer > 99) timer = 99;
        sendTimer(&timer);
      }

      if(sendWhat==2) {
        // testing msgLeft
        static byte leftBPM = 199;
        leftBPM-=13;
        if (leftBPM > 199) leftBPM = 199;
        sendLeft(&leftBPM);
      }

      if(sendWhat==3) {
        // testing msgRight
        static byte rightBPM = 199;
        rightBPM+=11;
        if (rightBPM > 199) rightBPM = 0;
        sendRight(&rightBPM);
      }
}
  }
}


void sendLeft(byte *bpm) {
  const char* pub = "asOne/score/leftBPM";

  mqtt.publish(pub, (uint8_t *)bpm, 1);
}

void sendRight(byte *bpm) {
  const char* pub = "asOne/score/rightBPM";

  mqtt.publish(pub, (uint8_t *)bpm, 1);
}

void sendTimer(byte *timer) {
  const char* pub = "asOne/score/timer";

  mqtt.publish(pub, (uint8_t *)timer, 1);
}


void sendLogoDirect(CRGB *pixels) {
  const char* pub = "asOne/score/logo/direct";

  mqtt.publish(pub, (uint8_t *)pixels, sizeof(CRGB)*nLogoLED);

  Serial << F("-> ") << pub << F(" =");
  Serial << F(" R:") << pixels[4].red;
  Serial << F(" G:") << pixels[4].green;
  Serial << F(" B:") << pixels[4].blue;
  Serial << endl;
}

void sendLogo(CRGB pixel) {
  const char* pub = "asOne/score/logo";

  mqtt.publish(pub, (uint8_t *)&pixel, sizeof(pixel));

  Serial << F("-> ") << pub << F(" =");
  Serial << F(" R:") << pixel.red;
  Serial << F(" G:") << pixel.green;
  Serial << F(" B:") << pixel.blue;
  Serial << endl;
}

void sendState() {
  const char* pub = "asOne/score/state";

  itoa(state, msg, 10);
  mqtt.publish(pub, msg);

  Serial << F("-> ") << pub << F(" = ") << state << endl;
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

  const String msgLogo = "asOne/score/logo";
  const String msgLogoDirect = "asOne/score/logo/direct";
  const String msgTimer = "asOne/score/timer";

  if (t.equals(msgLogo)) {
    leds(startLogo, stopLogo) = CRGBSet( (CRGB*)payload, 1 );

    Serial << F(" =");
    Serial << F(" R:") << leds[startLogo].red;
    Serial << F(" G:") << leds[startLogo].green;
    Serial << F(" B:") << leds[startLogo].blue;

  } else if (t.equals(msgLogoDirect)) {
    leds(startLogo, stopLogo) = CRGBSet( (CRGB*)payload, nLogoLED );

    Serial << F(" =");
    Serial << F(" R:") << leds[startLogo + 4].red;
    Serial << F(" G:") << leds[startLogo + 4].green;
    Serial << F(" B:") << leds[startLogo + 4].blue;

  } else if (t.equals(msgTimer)) {
    byte timer = payload[0];
    Serial << F(" = ") << timer;
  }
  Serial << endl;
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

  const char* id = "asOneScoreboardSpoof";
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


