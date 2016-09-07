

const int analogIn = A0;

int sensorValue = 0;

void setup() {
  pinMode(analogIn, INPUT);
  delay(10);
  Serial.begin(38400);
}

void loop() {
  sensorValue = analogRead(analogIn);
  Serial.println(sensorValue);
  delay(1);
}
