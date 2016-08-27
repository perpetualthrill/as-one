import processing.serial.*;
Serial mySerial;
PrintWriter output;

void setup() {
  // 32 == /dev/ttyUSB0
  mySerial = new Serial( this, Serial.list()[32], 9600 );
  mySerial.bufferUntil('\n');
  output = createWriter( "/home/jack/data.txt" );
}

void draw() {
  String value = null;
  if (mySerial.available() > 0 ) {
    value = mySerial.readStringUntil('\n');
    if ( value != null ) {
      String info = millis() + " -- "+ value ;
      output.print(info);
      print(info);
    }
    value = null;
  }
}

void keyPressed() {
  output.flush();  // Writes the remaining data to the file
  output.close();  // Finishes the file
  exit();  // Stops the program
}