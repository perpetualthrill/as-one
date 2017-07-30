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
      - "asOne/score/timer": [0-99] current timer to display on the countdown
      - "asOne/score/left": [0-199] current heartrate to display on left score
      - "asOne/score/right": [0-199] current heartrate to display on right score

*/

#ifndef Instruction_h
#define Instruction_h

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Metro.h>
#include <Streaming.h>

#define RED_LED 0
#define RED_ON LOW
#define RED_OFF HIGH
#define BLUE_LED 2

#define HEARTBEAT_INTERVAL 2000UL // ms

class Instruction {
  public:
    void begin(); // startup
    boolean update(); // returns true with a new message

    byte getState(); // returns state value
    byte getTimer(); // returns timer value
    byte getLeft(); // returns left value
    byte getRight(); // returns right value

  private:
    byte state, timer, left, right;
    boolean newMessage;

    void connectWiFi(); // BLOCKING. connect to the WiFi network.
    void connectMQTT(); // (re)connect to MQTT server.
    void callback(char* topic, byte* payload, unsigned int length); // callback used with new message
    
    // Update these.
    const char* ssid = "Looney";
    const char* password = "TinyandTooney";
    const char* mqtt_server = "broker.mqtt-dashboard.com";

    const char* id = "asOneScoreboard";
    const char* pub = "asOne/score/heartbeat";
    const char* sub = "asOne/score/#";

    WiFiClient espClient;
    PubSubClient client;

};

#endif
