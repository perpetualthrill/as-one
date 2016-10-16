import processing.serial.*;
Serial mySerial;

String previous = null;

void setup() {
  // 32 == /dev/ttyUSB0
  mySerial = new Serial( this, Serial.list()[32], 38400 );
}

void draw() { 
  // all processing in serialEvent
}

void serialEvent (Serial myPort) {
  String inString = myPort.readStringUntil('\n');
  if (null != inString) {
    if (!inString.equals(previous)) {
      int millis = millis();
      print("got one at "+millis+" -- "+inString);
    }
    previous = new String(inString);
    inString = null;
  }
}

void keyPressed() {
  exit();  // Stops the program
}