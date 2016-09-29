
const int p1input = A0;
const int p3input = A1;
//const int p1input = 6;
//const int p3input = 7;

int p1value = -999;
int p3value = -999;

void setup() {
  pinMode(p1input, INPUT);
  pinMode(p3input, INPUT);
  delay(10);
  Serial.begin(38400);
}

void loop() {
  int new_p1value = analogRead(p1input);
  int new_p3value = analogRead(p3input);
  
  if (abs(new_p1value - p1value) > 2 || abs(new_p3value - p3value) > 2) {
    p1value = new_p1value;
    p3value = new_p3value;
    Serial.print("p1: ");
    Serial.print(p1value);
    Serial.print(" p3: ");
    Serial.println(p3value);
  //  delay(1);
  }
}

