#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

int lastval = 999;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(38400);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // set the data rate for the SoftwareSerial port
  mySerial.begin(115200);
}

void loop() { // run over and over
  int bytecount = mySerial.available();
  if (bytecount) {
    // this is usually one or two bytes
    // Serial.println(bytecount);
    for (int i = 0; i < bytecount; i++) {
      int val = mySerial.read();
    //  if (val != 0) { // why is it zero so often???
        Serial.print(val);
        Serial.print(" ");
 //     }
    }
    Serial.println();
  }
}
