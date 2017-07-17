#include <PubSubClient.h>
#include <ESP8266WiFi.h>

WiFiClient espClient;
PubSubClient client(espClient);

String testUpper = String("asOne/testUpper");
String testLower = String("asOne/testLower");

const int buttonPinUpper = 14;     // the number of the pushbutton pin
int buttonStateUpper = 0;
const int buttonPinLower = 16;     // the number of the pushbutton pin
int buttonStateLower = 0;

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
      //client.subscribe("asOne/hello");
      client.subscribe(testUpper.c_str());
      client.subscribe(testLower.c_str());
      //client.subscribe("asOne/doPulse");
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
  buttonStateLower = digitalRead(buttonPinLower);
  buttonStateUpper = digitalRead(buttonPinUpper);
  if (buttonStateLower == HIGH) {
    client.publish(testLower.c_str(), "250");
  } else {
    
  }
  if (buttonStateUpper == HIGH) {
    client.publish(testUpper.c_str(), "250");
  } else {
    
  }
  client.loop();
}
