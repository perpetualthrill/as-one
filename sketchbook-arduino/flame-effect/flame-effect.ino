#include <PubSubClient.h>
#include <WiFi.h>

WiFiClient espClient;
PubSubClient client(espClient);

const int LONG_DELAY_MS = 5000;
const int MS_PER_SECOND = 60000;
const int HEARTBEAT_INTERVAL_MS = 1000;

String testUpperTopic = String("asOne/fe/testUpper");
String testLowerTopic = String("asOne/fe/testLower");
String heartbeatTopic = String("asOne/fe/heartbeat");
String bpmTopic = String("asOne/fe/doBPM");

const int upperPin = 12;
const int lowerPin = 16;

int upperPinStartMillis = 0;
int lowerPinStartMillis = 0;
int upperPinTargetMillis = 0;
int lowerPinTargetMillis = 0;
bool upperPinOn = false;
bool lowerPinOn = false;
int pauseTargetMillis = 0;
int beatCountdown = 0;
int beatLength = 0;

int heartbeatTargetMillis = millis();

void setup() {

  Serial.begin(115200);
  Serial.println();
  Serial.println("O Hai");

  pinMode(upperPin, OUTPUT);
  internalDigitalWrite(upperPin, LOW);
  pinMode(lowerPin, OUTPUT);
  internalDigitalWrite(lowerPin, LOW);

  Serial.print("Connecting");
  WiFi.begin("AsOne", "purplemotion");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer("asone-console.local", 1883);
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
  char *p = (char*) malloc(length);
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
  } else if (checkTopic.equals(bpmTopic)) {
    int bpm = checkPayload.toInt();
    if ((bpm < 50) || (bpm > 180)) {
      return;
    }
    Serial.print("do heartbeat at bpm = ");
    Serial.println(checkPayload.toInt());
    beatLength = MS_PER_SECOND / bpm;
    beatCountdown = 4;
    enqueueNextBeat();
  }
}

void enqueueNextBeat() {
  beatCountdown--;
  int now = millis();
  upperPinStartMillis = now;
  upperPinTargetMillis = now + (beatLength * .22);
  lowerPinStartMillis = now + (beatLength * .25);
  lowerPinTargetMillis = now + (beatLength * .60);
  pauseTargetMillis = now + beatLength;
  Serial.print("up start ");
  Serial.print(upperPinStartMillis);
  Serial.print(" up end ");
  Serial.print(upperPinTargetMillis);
  Serial.print(" low start ");
  Serial.print(lowerPinStartMillis);
  Serial.print(" low end ");
  Serial.print(lowerPinTargetMillis);
  Serial.println(" \\m/");
}

void doPoof(int pin, int durationMs) {
  internalDigitalWrite(pin, HIGH);
  int now = millis();
  switch(pin) {
    case upperPin:
      upperPinStartMillis = now;
      upperPinTargetMillis = now + durationMs;
      break;
    case lowerPin:
      lowerPinStartMillis = now;
      lowerPinTargetMillis = now + durationMs;
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
      client.subscribe(bpmTopic.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(LONG_DELAY_MS);
    }
  }
}

void internalDigitalWrite(int pin, int val) {
  if (pin == upperPin) {
    if (val == HIGH) {
      if (!upperPinOn) {
        Serial.println("UPPER HIGH");
        digitalWrite(pin, val);
        upperPinOn = true;
      }
    } else if (val == LOW) {
      if (upperPinOn) {
        Serial.println("UPPER LOW");
        digitalWrite(pin, val);
        upperPinOn = false;
      }
    }
  } else if (pin == lowerPin) {
    if (val == HIGH) {
      if (!lowerPinOn) {
        Serial.println("LOWER HIGH");
        digitalWrite(pin, val);
        lowerPinOn = true;
      }
    } else if (val == LOW) {
      if (lowerPinOn) {
        Serial.println("LOWER LOW");
        digitalWrite(pin, val);
        lowerPinOn = false;
      }
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
  int now = millis();

  if (upperPinTargetMillis > 0) {
    if (now >= upperPinStartMillis) {
      if (now < upperPinTargetMillis) {
         internalDigitalWrite(upperPin, HIGH);  
      } else {
         internalDigitalWrite(upperPin, LOW);
         upperPinTargetMillis = 0;
      }
    }
  }

  if (lowerPinTargetMillis > 0) {
    if (now >= lowerPinStartMillis) {
      if (now < lowerPinTargetMillis) {
         internalDigitalWrite(lowerPin, HIGH);  
      } else {
         internalDigitalWrite(lowerPin, LOW);
         lowerPinTargetMillis = 0;
      }
    }
  }

  if (beatCountdown > 0 && now > pauseTargetMillis) {
    enqueueNextBeat();
  }

  if (now > heartbeatTargetMillis) {
    client.publish(heartbeatTopic.c_str(), "hello from flame effect");
    heartbeatTargetMillis = now + HEARTBEAT_INTERVAL_MS;
  }

  client.loop();
}
