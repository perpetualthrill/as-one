

const int analogIn = A0;

int sensorValue = 0;

void setup() {
  pinMode(analogIn, INPUT);
  delay(10);
  Serial.begin(9600);
}

void loop() {
  sensorValue = analogRead(analogIn);
  Serial.print(sensorValue);
  delay(25);
}
