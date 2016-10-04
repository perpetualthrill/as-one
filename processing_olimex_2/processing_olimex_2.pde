import processing.serial.*;
Serial mySerial;

String previous = null;

PrintWriter writer;

void setup() {
  // 32 == /dev/ttyUSB0
  // 0 = COM1
  mySerial = new Serial( this, Serial.list()[0], 38400 );
  
  writer = createWriter("/home/jack/foo.csv");
  delay(10);
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
      writer.print(inString);
    }
    previous = new String(inString);
    inString = null;
  }
}

void keyPressed() {
  writer.flush();  // Writes the remaining data to the file
  writer.close();  // Finishes the file
  exit();  // Stops the program
}