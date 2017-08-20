#include <PubSubClient.h>
#include <ESP8266WiFi.h>

WiFiClient espClient;
PubSubClient client(espClient);

#define LONG_DELAY_MS 5000

String testUpperTopic = String("asOne/fe/testUpper");
String testLowerTopic = String("asOne/fe/testLower");
String heartbeatTopic = String("asOne/fe/doHeartbeat");

const int upperPin = 12;
const int lowerPin = 13;

int upperPinTargetMillis = 0;
int lowerPinTargetMillis = 0;

void setup() {
  pinMode(upperPin, OUTPUT);
  digitalWrite(upperPin, LOW);
  pinMode(lowerPin, OUTPUT);
  digitalWrite(lowerPin, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.begin(115200);
  Serial.println();

  WiFi.begin("AsOne", "fuckthapolice");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer("asone-console", 1883);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  String checkTopic = String(topic);
  char* p = (char*)malloc(length);
  memcpy(p, payload, length);
  String checkPayload = String(p);
  
  if (checkTopic.equals(testUpperTopic)) {
    Serial.print("do upper: ");
    Serial.println(checkPayload.toInt());
    doPoof(upperPin, checkPayload.toInt());
  } else if (checkTopic.equals(testLowerTopic)) {
    Serial.print("do lower: ");
    Serial.println(checkPayload.toInt());
    doPoof(lowerPin, checkPayload.toInt());
  }
}

void doPoof(int pin, int duration) {
  digitalWrite(pin, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  switch(pin) {
    case upperPin:
      break;
    
  }
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client_flame_effect")) {
      Serial.println("connected");
      client.publish("asOne/hello", "hello from flame effect");
      client.subscribe(testUpperTopic.c_str());
      client.subscribe(testLowerTopic.c_str());
      client.subscribe(heartbeatTopic.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(LONG_DELAY_MS);
    }
  }
}

void stopPoofs() {
  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  stopPoofs();
  client.loop();
}

