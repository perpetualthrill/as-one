#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Metro.h>
#include <Streaming.h>
#include "Instruction.h"

Instruction inst;

void setup() {
  Serial.begin(115200);
  inst.begin();
}

void loop() {
  inst.update();
}

