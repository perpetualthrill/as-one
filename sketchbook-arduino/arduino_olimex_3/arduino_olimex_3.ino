// Teensy 3.0 -- Pin 0 = RX, Pin 1 = TX
#define HWSERIAL Serial1

const int SAMPLE_RATE_HZ = 60; // match 60fps on processing loop
const int SAMPLE_DELAY_MS = 1000 / SAMPLE_RATE_HZ;
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(38400);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // set the data rate for the olimex uart
  HWSERIAL.begin(115200);

  // debounce
  delay(1);
}

void loop() {
  int bytecount = HWSERIAL.available();
  if (bytecount) {
    for (int i = 0; i < bytecount; i++) {
      int val = HWSERIAL.read();
      Serial.println(val);
    }
  }
  delay(SAMPLE_DELAY_MS);
}
