#include "Instruction.h"

void Instruction::callback(char* topic, byte* payload, unsigned int length) {

  // toggle the LED when we get a new message
  static boolean ledState = false;
  ledState = !ledState;
  digitalWrite(BLUE_LED, ledState);

  // indicate there's not new message
  this->newMessage = false;

  // String class is much easier to work with
  String t = topic;
  int m = String((char*)payload).toInt();

  // drop our own heartbeat
  const String msgHeartbeat = "asOne/score/heartbeat";
  if ( t.equals(msgHeartbeat) ) return;

  Serial << F("<- ") << t << F("=") << m << endl;

  // list of topics that we'll process
  const String msgState = "asOne/score/state";
  const String msgTimer = "asOne/score/timer";
  const String msgLeft = "asOne/score/left";
  const String msgRight = "asOne/score/right";

  if ( t.equals(msgState) ) {
    this->state = m;
    this->newMessage = true;
  } else if (t.equals(msgTimer)) {
    this->timer = m;
    this->newMessage = true;
  } else if (t.equals(msgLeft)) {
    this->left = m;
    this->newMessage = true;
  } else if (t.equals(msgRight)) {
    this->right = m;
    this->newMessage = true;
  } else {
    Serial << F("WARNING. unknown topic. continuing.") << endl;
  }
}

void Instruction::begin() {
  pinMode(BLUE_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(RED_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

  connectWiFi();

  client.setClient(espClient);
  client.setServer(mqtt_server, 1883);
  // So, how the hell do I resolve this gobbledygook?
  client.setCallback(callback);
}

void Instruction::connectWiFi() {
  digitalWrite(RED_LED, RED_ON);
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
  digitalWrite(RED_LED, RED_OFF);
}


void Instruction::connectMQTT() {
  digitalWrite(RED_LED, RED_ON);

  static Metro connectInterval(500UL);
  if ( connectInterval.check() ) {

    Serial << F("Attempting MQTT connection...") << endl;
    // Attempt to connect
    if (client.connect(id)) {
      Serial << F("Connected.") << endl;
      // subscribe
      Serial << F("Subscribing: ") << sub << endl;
      client.subscribe(sub);
      digitalWrite(RED_LED, RED_OFF);

    } else {
      Serial << F("Failed. state=") << client.state() << endl;
    }

    connectInterval.reset();
  }
}


boolean Instruction::update() {
  if (!client.connected()) {
    connectMQTT();
  } else {
    this->newMessage = false;
    client.loop();

    // heartbeat interval
    static Metro heartbeatInterval(HEARTBEAT_INTERVAL);
    if ( heartbeatInterval.check() ) {
      //      Serial << F("-> ") << pub << F("=") << millis() << endl;
      char msg[32];
      itoa(millis(), msg, 10);
      client.publish(pub, msg);
      heartbeatInterval.reset();
    }

    return ( this->newMessage );
  }
}
