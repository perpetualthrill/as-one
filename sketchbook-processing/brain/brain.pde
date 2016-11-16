import processing.serial.*;
Serial mySerial;

volatile String found = null;

PrintWriter writer;

void setup() {
  // 32 = /dev/ttyUSB0
  // 0 = COM1
  mySerial = new Serial( this, Serial.list()[0], 38400 );

  writer = createWriter("/home/jack/foo.csv");

  delay(10);
}

void draw() {
  // log input
  String foundCopy = found;
  if (null != foundCopy) {
    int millis = millis();
    writer.print(millis+","+foundCopy);
    System.out.print(millis+","+foundCopy);
    found = null;
  }
}

void serialEvent (Serial myPort) {
  String inString = myPort.readStringUntil('\n');
  if (null != inString) {
    found = new String(inString);
    inString = null;
  }
}

void keyPressed() {
  writer.flush();  // Writes the remaining data to the file
  writer.close();  // Finishes the file
  exit();  // Stops the program
}