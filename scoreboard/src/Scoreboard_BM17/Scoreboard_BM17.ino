/*
  MQTT-based scoreboard reporting.

  It connects to an MQTT server then:
  - publishes:
      - "asOne/score/heartbeat": current millis() every two seconds
  - subscribes:
      - "asOne/score/#": multi-level wildcard
      - "asOne/scoreboard/#": multi-level wildcard
  - processes:
      - "asOne/score/state": [0,1,2] current state of the system
        - 0: idle/not playing.  scoreboard will mess around on its own.
        - 1: active/playing.  scoreboard will process the following subscriptions
        - 2: won/flames.  scoreboard will do some (garish) fanfare while the flames are shooting
      - "asOne/score/logo": bytestream castable to CRGB, applied to all LEDs in logo
        - "asOne/score/logo/direct": bytestream castable to CRGB[15]
      - "asOne/score/timer": [0-99] current timer to display on the countdown
        - "asOne/score/timer/direct: bytestream castable to CRGB[26]
      - "asOne/score/leftBPM": [0-199] current heartrate to display on left score
        - "asOne/score/leftBPM/direct": bytestream castable to CRGB[47]
      - "asOne/score/rightBPM": [0-199] current heartrate to display on right score
        - "asOne/score/rightBPM/direct": bytestream castable to CRGB[47]

      - "asOne/scoreboard/directOnly": [0,1] whether to render digits locally
        - 0: local display.  scoreboard will render all messages
        - 1: direct only.  another process is responsible for translating numeric values into direct RGB values

      - "asOne/scoreboard/acceleration": [0,1,2,3] LED driver mode
        - 0: use processor-based FastLED library
        - 1: use UART to drive LEDs
        - 2: use UART with gamma correction
        - 3: use UART with gamma correction and temporal dithering

   - publishes:
      - "asOne/openPixelControl/scoreboardAddress": String. IP address the scoreboard is listening on for OPC messages


*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Metro.h>
#include <Streaming.h>
// required prior to #include
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_INTERRUPT_RETRY_COUNT 1
#include <FastLED.h>

WiFiClient espClient;
PubSubClient mqtt;

// track the state
byte state = 1;

byte directOnly = 0;
byte acceleration = 0;

// track the logo
const byte nLogoLED = 15;
const byte startLogo = 47;
const byte stopLogo = startLogo + nLogoLED - 1;

// small segment displays
const byte nsDigit = 13;
// track the timer
const byte nTimerLED = nsDigit + nsDigit; // 26
const byte startTimer = 62;
const byte stopTimer = startTimer + nTimerLED - 1;

// large segment displays
const byte nlDigit = 20;
const byte nlHundreds = 7;

// track the left BPM
byte leftBPM=100;
const byte nLeftLED = nlHundreds + nlDigit + nlDigit; // 47
const byte startLeft = 88;
const byte stopLeft = startLeft + nLeftLED - 1;

// track the right BPM
byte rightBPM=100;
const byte nRightLED = nLeftLED; // 47
const byte startRight = 0;
const byte stopRight = startRight + nRightLED - 1;

// overall LED array
const byte nTotalLED = nLogoLED + nTimerLED + nLeftLED + nRightLED;
CRGBArray <nTotalLED> leds;

// pin definitions
#define PIN_LED_DMA_METHOD 3
#define PIN_LED_UART_METHOD 2
#define PIN_LED_GPIO_METHOD 15
// select which; set the JUMPER!
#define PIN_LED PIN_LED_UART_METHOD

// track the need to do a FastLED.show();
boolean haveUpdate = false;

// led pinouts and high/low definition for red one.
#define BLUE_LED 2 // DO NOT USE 
#define RED_LED 0
#define RED_ON LOW
#define RED_OFF HIGH

// publish a heartbeat on this interval
#define HEARTBEAT_INTERVAL 2000UL // ms

// set target FPS
const unsigned long targetFPS = 30; // frames per second



void setup() {
  Serial.begin(115200);
  Serial << endl << endl << F("Scoreboard. Startup.") << endl;

  pinMode(RED_LED, OUTPUT);     // Initialize the red LED pin as an output

  // don't allow the WiFi module to sleep.  this interacts with the FastLED library, so key.
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  mqtt.setClient(espClient);
//  const char* mqtt_server = "broker.mqtt-dashboard.com";
  const char* mqtt_server = "asone-console";
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback);

  CRGB foo[nLeftLED];
  if( sizeof(foo) > MQTT_MAX_PACKET_SIZE ) {
    Serial << F("packet size must be >=") << sizeof(foo) << endl;
    Serial << F("packet size is=") << MQTT_MAX_PACKET_SIZE << endl;
    Serial << F("HALTING.") << endl;
    while(1);
  }

  FastLED.addLeds<WS2811, PIN_LED, RGB>(leds, nTotalLED).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);
  leds.fill_solid(CRGB::Red);
  FastLED.show();
  Serial << F("Red.") << endl;
  delay(1000);
  leds.fill_solid(CRGB::Green);
  FastLED.show();
  Serial << F("Green.") << endl;
  delay(1000);
  leds.fill_solid(CRGB::Blue);
  FastLED.show();
  Serial << F("Blue.") << endl;
  delay(1000);
  leds.fill_solid(CRGB::White);
  FastLED.show();
  Serial << F("Blue.") << endl;
  delay(5000);
  
  FastLED.clear(true);
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

    switch(state) {
      case 0: // idle
        static byte idleHue = 0;
        idleHue++;
        leds.fill_rainbow(idleHue);
        haveUpdate = true;
        break;
      case 1: // active/playing
        // wait for messages to process
        break;
      case 2: // won/flames
        // increment up to White
        leds.addToRGB(16);
        haveUpdate = true;
        break;
    }

    // if we have an update, show it
    if ( haveUpdate ) showScoreboard();
    // send a heartbeat on an interval heartbeat interval
    else heartbeatMQTT();
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
    state = payload[0];
    FastLED.clear(); // clear the LEDs
    Serial << F(" = ") << state;
  } else if (t.equals(msgLogo)) {
    if(directOnly){
      Serial << "ignored";
    } else {
      CRGB color = CRGB(payload[0], payload[1], payload[2]);
      leds(startLogo, stopLogo).fill_solid(color);

      Serial << F(" =");
      Serial << F(" R:") << leds[startLogo].red;
      Serial << F(" G:") << leds[startLogo].green;
      Serial << F(" B:") << leds[startLogo].blue;
    }

  } else if (t.equals(msgLogoDirect)) {
    leds(startLogo, stopLogo) = CRGBSet( (CRGB*)payload, nLogoLED );

    //    Serial << F(" =");
    //    Serial << F(" R:") << leds[startLogo + 4].red;
    //    Serial << F(" G:") << leds[startLogo + 4].green;
    //    Serial << F(" B:") << leds[startLogo + 4].blue;

  } else if (t.equals(msgTimer)) {
    if(directOnly){
      Serial << "ignored";
    } else {
      byte timer = payload[0];
      Serial << F(" = ") << timer/10 << F(",") << timer %10;

      setSmallDigit(timer%10, startTimer, CRGB::White, CRGB::Black);
      setSmallDigit(timer/10, startTimer+nsDigit, CRGB::White, CRGB::Black);
    }
  } else if (t.equals(msgTimerDirect)) {
    leds(startTimer, stopTimer) = CRGBSet( (CRGB*)payload, nTimerLED );
  } else if (t.equals(msgLeft)) {
    if(directOnly){
      Serial << "ignored";
    } else {
      leftBPM = payload[0];
      Serial << F(" = ") << leftBPM/100 << F(",") << (leftBPM/10)%10 << F(",") << leftBPM%10;

      // on color based on BPM delta
      CRGB onColor = setBPMColor(leftBPM, rightBPM);
  //    onColor = CRGB::Blue;
      
      setLargeDigit(leftBPM%10, startLeft, onColor, CRGB::Black);
      setLargeDigit((leftBPM/10)%10, startLeft+nlDigit, onColor, CRGB::Black);
      // special case for hundreds digit
      leds(startLeft+2*nlDigit, startLeft+2*nlDigit+nlHundreds-1) = leftBPM/100 ? onColor : CRGB::Black;
    }
  } else if (t.equals(msgLeftDirect)) {
    leds(startLeft, stopLeft) = CRGBSet( (CRGB*)payload, nLeftLED );
  } else if (t.equals(msgRight)) {
    if(directOnly){
      Serial << "ignored";
    } else {
      rightBPM = payload[0];
      Serial << F(" = ") << rightBPM/100 << F(",") << (rightBPM/10)%10 << F(",") << rightBPM%10;

      // on color based on BPM delta
      CRGB onColor = setBPMColor(rightBPM, leftBPM);

      setLargeDigit(rightBPM%10, startRight, onColor, CRGB::Black);
      setLargeDigit((rightBPM/10)%10, startRight+nlDigit, onColor, CRGB::Black);
      // special case for hundreds digit
      leds(startRight+2*nlDigit, startRight+2*nlDigit+nlHundreds-1) = rightBPM/100 ? onColor : CRGB::Black;
    }
   } else if (t.equals(msgRightDirect)) {
    leds(startRight, stopRight) = CRGBSet( (CRGB*)payload, nRightLED );
  } else {
    haveUpdate = false;
    Serial << F(" WARNING. unknown topic. continuing.");
  }

  Serial << endl;
}

CRGB setBPMColor(byte b1, byte b2) {
/*
  // use Blue as the base
  CRGB onColor = CRGB::Blue;
  onColor -= 128;
  if( b1 > b2) {
    // HR too fast.  slow it down, and suggest that by blending in Red
    byte redLevel = map(constrain(b1-b2, 0, 50), 0, 50, 32, 255);
    onColor += CRGB(redLevel, 0, 0);
  } else {
    // HR too slow.  speed it up, and suggest that by blending in Green
    byte greenLevel = map(constrain(b2-b1, 0, 50), 0, 50, 32, 255);
    onColor += CRGB(0, greenLevel, 0);
  }
  return( onColor );
*/
  int deltaBPM = b1 - b2; // positive if b1 > b2
  int hue = HUE_GREEN - deltaBPM; // Green if they're equal.
  hue = constrain(hue, HUE_RED, HUE_BLUE); // Red: slow down; Blue: speed up
  CRGB color = CHSV(hue, 255, 255);
  return(color);
}

void showScoreboard() {
  // could be getting a ton of update calls over the WiFi
  // so, we let those aggregate before we do a show();
  static Metro updateInterval(1000UL/targetFPS);

  // no update?  bail out.
  if( !haveUpdate ) return;
  // not time for an update?  bail out.
  if( !updateInterval.check() ) return;

  updateInterval.reset();
  haveUpdate = false;
  FastLED.show();
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

  const char* id = "asOneScoreboard";
  const char* scoreSub = "asOne/score/#";
  const char* configSub = "asOne/scoreboard/#";

  static Metro connectInterval(500UL);
  if ( connectInterval.check() ) {

    Serial << F("Attempting MQTT connection...") << endl;
    // Attempt to connect
    if (mqtt.connect(id)) {
      Serial << F("Connected.") << endl;
      // subscribe
      Serial << F("Subscribing: ") << scoreSub << endl;
      mqtt.subscribe(scoreSub);
      Serial << F("Subscribing: ") << configSub << endl;
      mqtt.subscribe(configSub);
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

// Segment layout: 
// https://docs.google.com/spreadsheets/d/1Li9DfwNnN8ccxYK40BgAzFoFvAQTD7QX6ummpo726QA/edit#gid=0

// large segment displays
const boolean T = true;
const boolean F = false;
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



