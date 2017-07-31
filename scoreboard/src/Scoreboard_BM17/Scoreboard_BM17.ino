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
byte leftBPM;
const byte nLeftLED = 47;
const byte startLeft = 98;
const byte stopLeft = startLeft + nLeftLED - 1;
// track the right BPM
byte rightBPM;
const byte nRightLED = nLeftLED;
const byte startRight = 0;
const byte stopRight = startRight + nRightLED - 1;

// overall LED array
const byte nWastedLED = 2;
const byte wastedPosition[] = {70,97};
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

// large segment displays
const boolean T = true;
const boolean F = false;
const byte nlDigit = 20;
const byte nlHundreds = 7;
const boolean lDigit[10][nlDigit] = {
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19
  { T, F, F, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T }, // 0
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19
  { T, F, F, F, F, F, F, F, F, T, T, T, T, T, T, F, F, F, F, F }, // 1
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19
  { T, T, T, T, T, T, T, T, T, T, F, F, T, T, T, T, T, T, F, F }, // 2
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19
  { T, T, T, T, F, F, T, T, T, T, T, T, T, T, T, T, T, T, F, F }, // 3
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19
  { T, T, T, T, F, F, F, F, F, T, T, T, T, T, T, F, F, T, T, T }, // 4
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19
  { T, T, T, T, F, F, T, T, T, T, T, T, F, F, T, T, T, T, T, T }, // 5
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19
  { T, T, T, T, T, T, T, T, T, T, T, T, F, F, T, T, T, T, T, T }, // 6
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19
  { T, F, F, F, F, F, F, F, F, T, T, T, T, T, T, T, T, T, F, F }, // 7
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19
  { T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T }, // 8
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19
  { T, T, T, T, F, F, F, F, F, T, T, T, T, T, T, T, T, T, T, T } // 9
};
// small segment displays
const byte nsDigit = 13;
const boolean sDigit[10][nsDigit] = {
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12
  { T, F, T, T, T, T, T, T, T, T, T, T, T }, // 0
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12
  { T, F, F, F, F, F, T, T, T, T, F, F, F }, // 1
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12
  { T, T, T, T, T, T, T, F, T, T, T, T, F }, // 2
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12
  { T, T, T, F, T, T, T, T, T, T, T, T, F }, // 3
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12
  { T, T, T, F, F, F, T, T, T, T, F, T, T }, // 4
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12
  { T, T, T, F, T, T, T, T, F, T, T, T, T }, // 5
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12
  { T, T, T, T, T, T, T, T, F, T, T, T, T }, // 6
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12
  { T, F, F, F, F, F, T, T, T, T, T, T, F }, // 7
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12
  { T, T, T, T, T, T, T, T, T, T, T, T, T }, // 8
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12
  { T, T, T, F, F, F, T, T, T, T, T, T, T } // 9
};


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

  FastLED.setBrightness(255);
  FastLED.clear(true);
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

// the real meat of the work is done here, where we process messages.
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

//    Serial << F(" =");
//    Serial << F(" R:") << leds[startLogo].red;
//    Serial << F(" G:") << leds[startLogo].green;
//    Serial << F(" B:") << leds[startLogo].blue;

  } else if (t.equals(msgLogoDirect)) {
    leds(startLogo, stopLogo) = CRGBSet( (CRGB*)payload, nLogoLED );

    //    Serial << F(" =");
    //    Serial << F(" R:") << leds[startLogo + 4].red;
    //    Serial << F(" G:") << leds[startLogo + 4].green;
    //    Serial << F(" B:") << leds[startLogo + 4].blue;

  } else if (t.equals(msgTimer)) {
    byte timer = payload[0];
    Serial << F(" = ") << timer/10 << F(",") << timer %10;

    setSmallDigit(timer%10, startTimer, CRGB::White, CRGB::Black);
    setSmallDigit(timer/10, startTimer+nsDigit, CRGB::White, CRGB::Black);
  } else if (t.equals(msgTimerDirect)) {
    leds(startTimer, stopTimer) = CRGBSet( (CRGB*)payload, nTimerLED );
  } else if (t.equals(msgLeft)) {
    leftBPM = payload[0];
    Serial << F(" = ") << leftBPM/100 << F(",") << (leftBPM/10)%10 << F(",") << leftBPM%10;

    setLargeDigit(leftBPM%10, startLeft, CRGB::White, CRGB::Black);
    setLargeDigit((leftBPM/10)%10, startTimer+nlDigit, CRGB::White, CRGB::Black);
    // special case for hundreds digit
    leds(startRight+2*nlDigit, startRight+2*nlDigit+nlHundreds) = leftBPM/100 ? CRGB::White : CRGB::Black;
    
  } else if (t.equals(msgLeftDirect)) {
    leds(startLeft, stopLeft) = CRGBSet( (CRGB*)payload, nLeftLED );
  } else if (t.equals(msgRight)) {
    rightBPM = payload[0];
    Serial << F(" = ") << rightBPM/100 << F(",") << (rightBPM/10)%10 << F(",") << rightBPM%10;

    setLargeDigit(rightBPM%10, startRight, CRGB::White, CRGB::Black);
    setLargeDigit((rightBPM/10)%10, startRight+nlDigit, CRGB::White, CRGB::Black);
    // special case for hundreds digit
    leds(startRight+2*nlDigit, startRight+2*nlDigit+nlHundreds) = rightBPM/100 ? CRGB::White : CRGB::Black;
    
   } else if (t.equals(msgRightDirect)) {
    leds(startRight, stopRight) = CRGBSet( (CRGB*)payload, nRightLED );
  } else {
    haveUpdate = false;
    Serial << F(" WARNING. unknown topic. continuing.");
  }

  Serial << endl;
}

void setSmallDigit(byte val, byte startPos, CRGB on, CRGB off) { 
  val = constrain(val, 0, 9);
  for( byte i=0; i<nsDigit; i++ ) {
    leds[startPos+i] = sDigit[val][i] ? on : off;
  }
}

void setLargeDigit(byte val, byte startPos, CRGB on, CRGB off) { 
  val = constrain(val, 0, 9);
  for( byte i=0; i<nlDigit; i++ ) {
    leds[startPos+i] = lDigit[val][i] ? on : off;
  }
}

void showScoreboard() {
  haveUpdate = false;
  FastLED.show();
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

// Layout: https://docs.google.com/spreadsheets/d/1Li9DfwNnN8ccxYK40BgAzFoFvAQTD7QX6ummpo726QA/edit#gid=0

void testDigitL(byte val) {
  val = constrain(val, 0, 9);

  Serial << lDigit[val][17]<< lDigit[val][16] << lDigit[val][15] << lDigit[val][14] << endl;
  Serial << lDigit[val][18]<<       '0'       <<       '0'       << lDigit[val][13] << endl;
  Serial << lDigit[val][19]<<       '0'       <<       '0'       << lDigit[val][12] << endl;
  Serial << lDigit[val][ 3]<< lDigit[val][ 2] << lDigit[val][ 1] << lDigit[val][ 0] << endl;
  Serial << lDigit[val][ 4]<<       '0'       <<       '0'       << lDigit[val][11] << endl;
  Serial << lDigit[val][ 5]<<       '0'       <<       '0'       << lDigit[val][10] << endl;
  Serial << lDigit[val][ 6]<< lDigit[val][ 7] << lDigit[val][ 8] << lDigit[val][ 9] << endl;
  Serial << endl;
}

void testDigitS(byte val) {
  val = constrain(val, 0, 9);

  Serial << sDigit[val][11]<< sDigit[val][10] << sDigit[val][ 9] << endl;
  Serial << sDigit[val][12]<<       '0'       << sDigit[val][ 8] << endl;
  Serial << sDigit[val][ 2]<< sDigit[val][ 1] << sDigit[val][ 0] << endl;
  Serial << sDigit[val][ 3]<<       '0'       << sDigit[val][ 7] << endl;
  Serial << sDigit[val][ 4]<< sDigit[val][ 5] << sDigit[val][ 6] << endl;
  Serial << endl;
}
 
