import processing.serial.*;
Serial mySerial;

volatile String found = null;

PrintWriter writer;

volatile ArrayList<String> allFound = new ArrayList<String>();

void setup() {
  // 32 = /dev/ttyUSB0
  // 0 = COM1
  mySerial = new Serial( this, Serial.list()[0], 38400 );

  writer = createWriter("/home/jack/foo.csv");

  delay(10);
}

void draw() {
  // log input
  ArrayList<String> listCopy;
  synchronized (allFound) {
    listCopy = new ArrayList<String>(allFound);
  }
  if (!listCopy.isEmpty()) {
    int millis = millis();
    for (String found : listCopy) {
      writer.print(millis+","+found);
      System.out.print(millis+","+found);
    }
    synchronized (allFound) {
      allFound.clear();
    }
  }
}

void serialEvent (Serial myPort) {
  String inString = myPort.readStringUntil('\n');
  if (null != inString) {
    synchronized (allFound) {
      allFound.add(new String(inString));
    }
    inString = null;
  }
}

void keyPressed() {
  writer.flush();  // Writes the remaining data to the file
  writer.close();  // Finishes the file
  exit();  // Stops the program
}