// Teensy 3.0 -- Pin 0 = RX, Pin 1 = TX
#define HWSERIAL Serial1

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(38400);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // set the data rate for the SoftwareSerial port
  HWSERIAL.begin(115200);
  delay(1);
}

void loop() { // run over and over
  int bytecount = HWSERIAL.available();
  if (bytecount) {
    for (int i = 0; i < bytecount; i++) {
      int val = HWSERIAL.read();
      Serial.println(val);
    }
  }
}
