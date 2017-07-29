/*
 Unit test for MQTT-based scoreboard reporting.
 
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
      - "asOne/score/timer": [0-99] current timer to display on the countdown
      - "asOne/score/left": [0-199] current heartrate to display on left score
      - "asOne/score/right": [0-199] current heartrate to display on right score
 
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Metro.h>
#include <Streaming.h>

// Update these with values suitable for your network.

const char* ssid = "...";
const char* password = "...";
const char* mqtt_server = "broker.mqtt-dashboard.com";

char id[] = "ScoreboardClient";
char pub[] = "asOne/score/heartbeat";
char sub[] = "asOne/score/#";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  connectWiFi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void connectWiFi() {

  delay(10);
  
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
}

void callback(char* topic, byte* payload, unsigned int length) {

  // toggle the LED when we get a new message
  static boolean ledState = false;
  ledState = !ledState;
  digitalWrite(BUILTIN_LED, ledState);
  
  // String class is much easier to work with
  String t = topic;
  Serial << F("<- ") << t;

  byte m = payload[0];
  if( length > 1 ) {
    Serial << F("ERROR. payload greater than one byte.  discarding.") << endl;
    return;
  }
  Serial << F("=") << m << endl;

  // list of topics that we'll process
  const String msgState = "asOne/score/state";
  const String msgTimer = "asOne/score/timer";
  const String msgLeft = "asOne/score/left";
  const String msgRight = "asOne/score/right";

  if( t.equals(msgState) ) processState(m);
  else if (t.equals(msgTimer)) processTimer(m);
  else if (t.equals(msgLeft)) processLeft(m);
  else if (t.equals(msgRight)) processRight(m);
  else Serial << F("WARNING. unknown topic. continuing.") << endl;
  
}

void processState(byte m) {
  
}
void processTimer(byte m) {
  
}
void processLeft(byte m) {
  
}
void processRight(byte m) {
  
}


void connectMQTT() {
  static Metro connectInterval(500UL);
  if( connectInterval.check() ) {
  
    Serial << F("Attempting MQTT connection...") << endl;
    // Attempt to connect
    if (client.connect(id)) {
      Serial << F("Connected.") << endl;
      // subscribe
      Serial << F("Subscribing: ") << sub << endl;
      client.subscribe(sub);
    } else {
      Serial << F("Failed. state=") << client.state() << endl;
    }

    connectInterval.reset();
  }
}
void loop() {

  if (!client.connected()) {
    connectMQTT();
  } else {
    client.loop();

    // heartbeat interval
    static Metro heartbeatInterval(2000UL);
    if( heartbeatInterval.check() ) {
      Serial << F("-> ") << pub << F("=") << millis() << endl;
      char msg[32];
      itoa(millis(), msg, 10);
      client.publish(pub, msg);
    }
  }
}
