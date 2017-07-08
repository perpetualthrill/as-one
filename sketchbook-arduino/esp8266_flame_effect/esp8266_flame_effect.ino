#include <PubSubClient.h>
#include <ESP8266WiFi.h>

WiFiClient espClient;
PubSubClient client(espClient);

String testUpper = String("asOne/testUpper");
String testLower = String("asOne/testLower");

void setup() {
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
  
  if (checkTopic.equals(testUpper)) {
    Serial.print("do upper: ");
    Serial.println(checkPayload.toInt());
  } else if (checkTopic.equals(testLower)) {
    Serial.print("do lower: ");
    Serial.println(checkPayload.toInt());
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("asOne/hello");
      client.subscribe(testUpper.c_str());
      client.subscribe(testLower.c_str());
      client.subscribe("asOne/doPulse");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {
    if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

