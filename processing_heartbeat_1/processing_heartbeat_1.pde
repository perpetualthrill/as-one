import processing.serial.*;
Serial mySerial;
PrintWriter output;

int[] lastThree = { 9999, 9999, 9999 };
int lastBeatMillis = 0;

void setup() {
  // 32 == /dev/ttyUSB0
  mySerial = new Serial( this, Serial.list()[32], 38400 );
 // mySerial.bufferUntil('\n');
  output = createWriter( "/home/jack/data.txt" );
}

void draw() {
  String value = null;
  int valueInt = 9999;
  if (mySerial.available() > 0 ) {
    value = mySerial.readStringUntil('\n');
    if ( value != null ) {
      int millis = millis();
      try {
        valueInt = Integer.parseInt(value.trim());
      } 
      catch (NumberFormatException e) {
        valueInt = 8888;
      }
      
      float inByte = float(valueInt);
      inByte = (inByte / 660) * 1023;
      String info = millis + " -- "+ inByte ;
      output.println(info);
      valueInt = int(inByte);

      lastThree[2] = lastThree[1];
      lastThree[1] = lastThree[0];
      lastThree[0] = valueInt;
      if (detect(millis)) {
        float bpm = 60000f / (millis - lastBeatMillis);
        if (bpm > 50) {
          // do not print initial beat reading
          println("Beat detected at "+bpm+" BPM");
          println("reference: "+ (millis - lastBeatMillis));
        }
        lastBeatMillis = millis;
      }
    }
    value = null;
  } else {
    delay(10);
    //println("waiting on serial");
  }
}

boolean detect(int millis) {
  println("DETECT: "+lastThree[0]+" "+lastThree[1]+" "+lastThree[2]+" -> "+ //(millis - lastBeatMillis));
        millis);
  if ((lastThree[0] <= 1000) &&
    (lastThree[1] <= 1000) &&
    (lastThree[2] <= 1000) &&
    ((millis - lastBeatMillis) > 300)) {
    return true;
  }
  return false;
} 

void keyPressed() {
  output.flush();  // Writes the remaining data to the file
  output.close();  // Finishes the file
  exit();  // Stops the program
}