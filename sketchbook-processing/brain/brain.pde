import processing.serial.*;
Serial mySerial;

PrintWriter writer;

ArrayList<String> allFound = new ArrayList<String>();

int lastMillis = 0;

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
    allFound.clear();
  }
  if (!listCopy.isEmpty()) {
    int millis = millis();
    for (String found : listCopy) {
      int resultCode = Integer.parseInt(found.trim());
      String line;
      if (resultCode == 111) {
        int diff = millis - lastMillis;
        int bpm = 60000 / diff;
        lastMillis = millis;
        line = millis+","+Integer.parseInt(found.trim())+","+bpm;
      } else {
        line = millis+","+Integer.parseInt(found.trim())+",0";
      }
      writer.println(line);
      System.out.println(line);
    }
  }
}

void serialEvent (Serial myPort) {
  String inString = myPort.readStringUntil('\n');
  if (null != inString) {
    synchronized (allFound) {
      allFound.add(new String(inString));
    }
  }
}

void keyPressed() {
  writer.flush();  // Writes the remaining data to the file
  writer.close();  // Finishes the file
  exit();  // Stops the program
}