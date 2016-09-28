
const int p1input = A0;
const int p3input = A1;

void setup() {
  pinMode(p1input, INPUT);
  pinMode(p3input, INPUT);
  delay(10);
  Serial.begin(38400);
}

void loop() {
  int p1value = analogRead(p1input);
  int p3value = analogRead(p3input);
  Serial.print("p1: ");
  Serial.print(p1value);
  Serial.print(" p3: ");
  Serial.println(p3value);
  delay(1);
}

