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

      - "asOne/scoreboard/acceleration": [0,1,2,3,4] LED driver mode
        - 0: use processor-based FastLED library
        - 1: use UART to drive LEDs
        - 2: use UART with gamma correction
        - 3: use UART with gamma correction and linear temporal interpolation
        - 4: use UART with gamma correction and temporal dithering

   - publishes:
      - "asOne/openPixelControl/scoreboard/ipAddress": String. IP address the scoreboard is listening on for OPC messages


*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Metro.h>
#include <Streaming.h>
#include <WiFiServer.h>
// required prior to #include
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_INTERRUPT_RETRY_COUNT 1
#include <FastLED.h>

extern "C" {
#include <uart.h>
#include <uart_register.h>
}
#define UartWaitBetweenUpdatesMicros 500
#define UartBaud 3200000
#define UART1 1
#define UART1_INV_MASK (0x3f << 19)


WiFiClient espClient;
PubSubClient mqtt;

#define OpcServerPort  7890
WiFiServer opcServer(OpcServerPort);
WiFiClient opcClient;

int opcMessageState;
int opcDataIndex;
int opcMessageLen;
int opcChannel;
int opcCommand;

// track the state
byte state = 1;

byte directOnly = 0;
byte acceleration = 0;

byte serial1 = 0;

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

// for temporal dithering
CRGBArray <nTotalLED> previousLeds;
int errorVal[nTotalLED * 3];

struct ditherTiming {
  unsigned long lastUpdateMillis;
  unsigned long avgUpdateMillis;
};

struct ditherTiming logoDitherTiming;
struct ditherTiming timerDitherTiming;
struct ditherTiming leftDitherTiming;
struct ditherTiming rightDitherTiming;

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
  Serial << F("White.") << endl;
  delay(5000);
  
  FastLED.clear(true);
  Serial << F("Scoreboard.  Startup complete.") << endl;

  opcServer.begin();
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
        if(!directOnly) {
          idleHue++;
          updateDitheringForEverything();
          leds.fill_rainbow(idleHue);
          haveUpdate = true;
        }
        break;
      case 1: // active/playing
        // wait for messages to process
        break;
      case 2: // won/flames
        // increment up to White
        if(!directOnly) {
          updateDitheringForEverything();
          leds.addToRGB(16);
          haveUpdate = true;
        }
        break;
    }

    // if we have an update, show it
    if ( haveUpdate ) showScoreboard();
    // send a heartbeat on an interval heartbeat interval
    heartbeatMQTT();

  }

  if (opcClient) {
    while (opcClient.available()) {
      int c = opcClient.read();
      if(c >= 0) {
        opcStateMachine(c);
      } else {
        break;
      }
    }
  }
  if ((!opcClient || !opcClient.connected()) && (opcClient = opcServer.available())) {
    Serial.println("OPC Connected");
  }

  if ( acceleration ) writeUART();
}

// the real meat of the work is done here, where we process messages.
void callback(char* topic, byte* payload, unsigned int length) {
  // toggle the blue LED when we get a new message
  static boolean ledState = false;
  ledState = !ledState;
  digitalWrite(RED_LED, ledState);

  // String class is much easier to work with
  char * msg = (char*)payload;
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

  const String msgDirectOnly = "asOne/scoreboard/directOnly";
  const String msgAcceleration = "asOne/scoreboard/acceleration";

  if ( t.equals(msgState) ) {
    state = payload[0];
    FastLED.clear(); // clear the LEDs
    Serial << F(" = ") << state;
  } else if (t.equals(msgLogo)) {
    if(directOnly){
      Serial << "ignored";
    } else {
      CRGB color = CRGB(payload[0], payload[1], payload[2]);

      updateDithering(startLogo, stopLogo, &logoDitherTiming);
      leds(startLogo, stopLogo).fill_solid(color);

      Serial << F(" =");
      Serial << F(" R:") << leds[startLogo].red;
      Serial << F(" G:") << leds[startLogo].green;
      Serial << F(" B:") << leds[startLogo].blue;
    }

  } else if (t.equals(msgLogoDirect)) {
    updateDithering(startLogo, stopLogo, &logoDitherTiming);
    leds(startLogo, stopLogo) = CRGBSet( (CRGB*)payload, nLogoLED );

    //    Serial << F(" =");
    //    Serial << F(" R:") << leds[startLogo + 4].red;
    //    Serial << F(" G:") << leds[startLogo + 4].green;
    //    Serial << F(" B:") << leds[startLogo + 4].blue;

  } else if (t.equals(msgTimer)) {
    if(directOnly){
      Serial << "ignored";
    } else {
//      String m = (char*)payload;
//      byte timer = m.toInt();
      byte timer = payload[0];
      Serial << F(" = ") << timer/10 << F(",") << timer %10;

      updateDithering(startTimer, stopTimer, &timerDitherTiming);
      setSmallDigit(timer%10, startTimer, CRGB::White, CRGB::Black);
      setSmallDigit(timer/10, startTimer+nsDigit, CRGB::White, CRGB::Black);
    }
  } else if (t.equals(msgTimerDirect)) {
    updateDithering(startTimer, stopTimer, &timerDitherTiming);
    leds(startTimer, stopTimer) = CRGBSet( (CRGB*)payload, nTimerLED );
  } else if (t.equals(msgLeft)) {
    if(directOnly){
      Serial << "ignored";
    } else {
//      String m = (char*)payload;
//      byte bpm = m.toInt();
      leftBPM = payload[0];
      Serial << F(" = ") << leftBPM/100 << F(",") << (leftBPM/10)%10 << F(",") << leftBPM%10;

      updateDithering(startLeft, stopLeft, &leftDitherTiming);

      // on color based on BPM delta
      CRGB onColor = setBPMColor(leftBPM, rightBPM);
//      onColor = CRGB::Blue;
      
      setLargeDigit(leftBPM%10, startLeft, onColor, CRGB::Black);
      setLargeDigit((leftBPM/10)%10, startLeft+nlDigit, onColor, CRGB::Black);
      // special case for hundreds digit
      leds(startLeft+2*nlDigit, startLeft+2*nlDigit+nlHundreds-1) = leftBPM/100 ? onColor : CRGB::Black;
    }
  } else if (t.equals(msgLeftDirect)) {
    updateDithering(startLeft, stopLeft, &leftDitherTiming);
    leds(startLeft, stopLeft) = CRGBSet( (CRGB*)payload, nLeftLED );
  } else if (t.equals(msgRight)) {
    if(directOnly){
      Serial << "ignored";
    } else {
//      String m = (char*)payload;
//      byte bpm = m.toInt();
//      rightBPM = bpm;
      rightBPM = payload[0];
      Serial << F(" = ") << rightBPM/100 << F(",") << (rightBPM/10)%10 << F(",") << rightBPM%10;

      updateDithering(startRight, stopRight, &rightDitherTiming);

      // on color based on BPM delta
      CRGB onColor = setBPMColor(rightBPM, leftBPM);

      setLargeDigit(rightBPM%10, startRight, onColor, CRGB::Black);
      setLargeDigit((rightBPM/10)%10, startRight+nlDigit, onColor, CRGB::Black);
      // special case for hundreds digit
      leds(startRight+2*nlDigit, startRight+2*nlDigit+nlHundreds-1) = rightBPM/100 ? onColor : CRGB::Black;
    }
  } else if (t.equals(msgRightDirect)) {
    updateDithering(startRight, stopRight, &rightDitherTiming);
    leds(startRight, stopRight) = CRGBSet( (CRGB*)payload, nRightLED );
  } else if (t.equals(msgDirectOnly)) {
    directOnly = payload[0];
    if(!directOnly && acceleration > 1) {
      // no reason to smooth animations if we're using the onboard patterns
      acceleration = 1;
    }
    haveUpdate = false;
    Serial << F(" = ") << directOnly;
  } else if (t.equals(msgAcceleration)) {
    acceleration = payload[0];
    setUART();
    haveUpdate = false;
    Serial << F(" = ") << acceleration;
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
  if( acceleration ) return; // LEDs handled by UART loop

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
      mqtt.publish("asOne/hello", "hello from scoreboard");
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
    publishMyIP();
    heartbeatInterval.reset();
  }
}

void publishMyIP() {
  const char* pub = "asOne/openPixelControl/scoreboard/ipAddress";
  IPAddress ip = WiFi.localIP();
  static byte msg[4];
  msg[0] = ip[0];
  msg[1] = ip[1];
  msg[2] = ip[2];
  msg[3] = ip[3];

  mqtt.publish(pub, msg, 4); // 4 bytes, not null-terminated
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

void setUART() {
  if(acceleration) {
    startUART();
  } else {
    stopUART();
  }
}

void startUART() {
  if( serial1 ) return;
  serial1 = 1;

  Serial1.begin(UartBaud, SERIAL_6N1, SERIAL_TX_ONLY);
  CLEAR_PERI_REG_MASK(UART_CONF0(UART1), UART1_INV_MASK);
  SET_PERI_REG_MASK(UART_CONF0(UART1), (BIT(22)));
}

void stopUART() {
  if( !serial1 ) return;
  serial1 = 0;

  Serial1.end();
}

// From Makuna's NeoPixelBus
static const uint8_t uartBitPatterns[4] = {
  0b110111, // On wire: 1 000 100 0 [Neopixel reads 00]
  0b000111, // On wire: 1 000 111 0 [Neopixel reads 01]
  0b110100, // On wire: 1 110 100 0 [Neopixel reads 10]
  0b000100, // On wire: 1 110 111 0 [NeoPixel reads 11]
};

// 16-bit gamma correction, from jes5199's esperPixels
const uint16_t gamma16[] = {
 0, 256, 256, 256, 257, 257, 258, 259,
260, 262, 264, 266, 269, 272, 275, 279,
284, 289, 295, 301, 308, 316, 324, 333,
343, 354, 365, 377, 390, 404, 419, 435,
451, 469, 488, 507, 528, 549, 572, 596,
621, 647, 674, 703, 733, 764, 796, 829,
864, 900, 938, 977, 1017, 1058, 1102, 1146,
1192, 1240, 1289, 1340, 1392, 1446, 1501, 1558,
1617, 1677, 1739, 1803, 1868, 1936, 2005, 2076,
2148, 2223, 2299, 2377, 2458, 2540, 2624, 2710,
2798, 2888, 2980, 3074, 3170, 3268, 3368, 3471,
3575, 3682, 3791, 3902, 4015, 4130, 4248, 4368,
4491, 4615, 4742, 4872, 5003, 5138, 5274, 5413,
5555, 5698, 5845, 5994, 6145, 6299, 6456, 6615,
6776, 6941, 7108, 7277, 7450, 7625, 7802, 7983,
8166, 8352, 8541, 8732, 8926, 9124, 9324, 9527,
9733, 9941, 10153, 10368, 10585, 10806, 11029, 11256,
11486, 11718, 11954, 12193, 12435, 12680, 12929, 13180,
13435, 13693, 13954, 14218, 14486, 14756, 15031, 15308,
15589, 15873, 16160, 16451, 16746, 17043, 17344, 17649,
17957, 18268, 18583, 18902, 19224, 19550, 19879, 20212,
20548, 20888, 21232, 21579, 21930, 22285, 22643, 23005,
23371, 23741, 24114, 24491, 24872, 25257, 25646, 26038,
26435, 26835, 27239, 27648, 28060, 28476, 28896, 29320,
29748, 30180, 30616, 31057, 31501, 31949, 32402, 32858,
33319, 33784, 34253, 34727, 35204, 35686, 36172, 36662,
37157, 37656, 38159, 38667, 39179, 39695, 40215, 40740,
41270, 41804, 42342, 42885, 43432, 43984, 44540, 45101,
45666, 46236, 46811, 47390, 47974, 48562, 49155, 49753,
50355, 50962, 51573, 52190, 52811, 53437, 54068, 54703,
55343, 55989, 56638, 57293, 57953, 58617, 59287, 59961,
60641, 61325, 62014, 62708, 63407, 64112, 64821, 65535
};



static inline uint8_t getTxLengthForUART1()
{
  return (U1S >> USTXC) & 0xff;
}

void writeUART() {
  static bool pausing = false;
  static unsigned long pauseUntilMicros = 0;

  if (getTxLengthForUART1() == 0) {
    if(!pausing) {
      pausing = true;
      pauseUntilMicros = micros() + UartWaitBetweenUpdatesMicros;
    } else {
      if(pauseUntilMicros <= micros()) {
        pausing = false;
        fillUART();
      }
    }
  }
}

static inline void writeByteToUART1(uint8_t byte)
{
  U1F = byte;
}

void fillUART() {
  unsigned long nowMillis = millis();
  for(int i = 0; i < nTotalLED * 3; i++) {
    uint8_t availableFifoSpace = 0;
    while(availableFifoSpace < 4) {
      availableFifoSpace = (UART_TX_FIFO_SIZE - getTxLengthForUART1());
    }

    byte val = colorValue(i, nowMillis);
    writeByteToUART1(uartBitPatterns[(val >> 6) & 0x3]);
    writeByteToUART1(uartBitPatterns[(val >> 4) & 0x3]);
    writeByteToUART1(uartBitPatterns[(val >> 2) & 0x3]);
    writeByteToUART1(uartBitPatterns[ val       & 0x3]);
  }
}

byte colorValue(int i, unsigned long nowMillis) {
  int pixelNum = i / 3;
  int colorNum = i % 3;

  byte requestedColor = leds[pixelNum][colorNum];

  if(acceleration < 2) {
    return requestedColor;
  }

  if(acceleration == 2) {
    // adafruit-style constant gamma correction
    return gamma16[requestedColor] / 256;
  }

  byte previousColor = previousLeds[pixelNum][colorNum];

  struct ditherTiming *myDitherTiming = ditherTimingForPixel(pixelNum);

  int deltaMax = 256;
  int delta = 0;
  if(myDitherTiming->avgUpdateMillis) {
    delta = (deltaMax * (nowMillis - myDitherTiming->lastUpdateMillis)) / myDitherTiming->avgUpdateMillis;
  }
  if(delta < 0) delta = 0;
  if(delta > deltaMax) delta = deltaMax;

  // Linear interpolation between the requested color and the previous color
  long requestedGamma = delta * gamma16[requestedColor];
  long previousGamma = (deltaMax - delta) * gamma16[previousColor];
  long value16 = (requestedGamma + previousGamma) / deltaMax;

  if(acceleration == 3) {
    return (value16 / 256);
  }

  int error = errorVal[pixelNum * 3 + colorNum];

  byte value8 = (value16 >> 8) & 0xFF;
  error = (value16 & 0xFF) + error;

  // Dither the linear interpolation with the accumulated rounding errors of the previous values
  if(error > 0 && value8 < 255 && value8 > 0) {
    value8 += 1;
    error -= 256;
  }
  errorVal[pixelNum * 3 + colorNum] = error;

  return value8;
}

struct ditherTiming *ditherTimingForPixel(byte pixelNum) {
  if(pixelNum >= startRight && pixelNum <= stopRight) {
    return &rightDitherTiming;
  }

  if(pixelNum >= startLeft && pixelNum <= stopLeft) {
    return &leftDitherTiming;
  }

  if(pixelNum >= startLogo && pixelNum <= stopLogo) {
    return &logoDitherTiming;
  }

  if(pixelNum >= startTimer && pixelNum <= stopTimer) {
    return &timerDitherTiming;
  }
}

void updateDithering(byte start, byte stop, struct ditherTiming *myDitherTiming) {
  previousLeds(start, stop) = leds(start, stop);
  updateDitherTiming(myDitherTiming);
}

void updateDitheringForEverything() {
  updateDithering(startLogo, stopLogo, &logoDitherTiming);
  updateDithering(startTimer, stopTimer, &timerDitherTiming);
  updateDithering(startLeft, stopLeft, &leftDitherTiming);
  updateDithering(startRight, stopRight, &rightDitherTiming);
}

void updateDitherTiming(struct ditherTiming *myDitherTiming) {
  unsigned long now = millis();
  unsigned long elapsed = now - myDitherTiming->lastUpdateMillis;
  if(myDitherTiming->avgUpdateMillis == 0) {
    myDitherTiming->avgUpdateMillis = elapsed;
  } else {
    myDitherTiming->avgUpdateMillis = (myDitherTiming->avgUpdateMillis + elapsed) / 2;
  }
  myDitherTiming->lastUpdateMillis = now;
}

void opcStateMachine(char c) {
    if(opcMessageState == 0) {
      opcMessageState = 1;
      opcChannel = c;
    } else if(opcMessageState == 1) {
      opcMessageState = 2;
      opcCommand = c;
      if(opcCommand == 0) {
        updateDitheringForChannel(opcChannel);
      }
    } else if(opcMessageState == 2) {
      opcMessageState = 3;
      opcMessageLen = c * 256;
    } else if(opcMessageState == 3) {
      opcMessageState = 4;
      opcMessageLen += c;
      opcDataIndex = 0;
    } else if(opcMessageState == 4) {
      opcMessageLen -= 1;
      if(opcCommand == 0) {
        int pixelIndex = opcDataIndex / 3;
        int color = opcDataIndex % 3;
        setPixelColor(opcChannel, pixelIndex, color, c);
      }
      opcDataIndex += 1;
    }

    if(opcMessageState == 4 && opcMessageLen <= 0) {
      opcMessageState = 0;
    }
}

void setPixelColor(byte channel, int pixelIndex, byte color, byte value) {
  if(channel == 0) { // not really used in As One but this is OPC spec
    setPixelColor(1, pixelIndex, color, value);
    setPixelColor(2, pixelIndex, color, value);
    setPixelColor(3, pixelIndex, color, value);
    setPixelColor(4, pixelIndex, color, value);
    return;
  }

  int pixelOffset;
  switch(channel) {
    case 1: pixelOffset = startRight; break;
    case 2: pixelOffset = startLogo; break;
    case 3: pixelOffset = startTimer; break;
    case 4: pixelOffset = startLeft; break;
  }

  leds[pixelOffset + pixelIndex][color] = value;
  haveUpdate = true;
}

void updateDitheringForChannel(byte channel) {
  switch(channel) {
    case 0: updateDitheringForEverything(); break;
    case 1: updateDithering(startRight, stopRight, &rightDitherTiming); break;
    case 2: updateDithering(startLogo, stopLogo, &logoDitherTiming); break;
    case 3: updateDithering(startTimer, stopTimer, &timerDitherTiming); break;
    case 4: updateDithering(startLeft, stopLeft, &leftDitherTiming); break;
  }
}
