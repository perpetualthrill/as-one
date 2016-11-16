import processing.serial.*;
Serial mySerial;

String previous = null;
float dataIn = 0;    // incoming byte\

// drawing
float height_old = 0;
float height_new = 0;
int xPos = 1;         // horizontal position of the graph
int cycles = 0;

PrintWriter writer;

void setup() {
  // 32 = /dev/ttyUSB0
  // 0 = COM1
  mySerial = new Serial( this, Serial.list()[0], 38400 );

  writer = createWriter("/home/jack/foo.csv");

  size(1000, 256);
  background(0);
  strokeWeight(2);
  stroke(128);

  delay(10);
}

void draw() {

  // log input
  int millis = millis();
  writer.println(millis+","+dataIn);

  // at the edge of the screen, go back to the beginning:
  if (xPos >= width) {
    if (cycles > 0) {
      return;
    }
    xPos = 0;
    background(0);
    cycles++;
  } else {
    // increment the horizontal position:
    xPos++;
  }
  height_new = height - dataIn; 
  line(xPos - 1, height_old, xPos, height_new);
  height_old = height_new;
}

void serialEvent (Serial myPort) {
  String inString = myPort.readStringUntil('\n');
  if (null != inString) {
    previous = new String(inString);
    float dataInLast = dataIn;
    dataIn = float(inString);
    if (dataIn == Float.NaN) {
      dataIn = dataInLast;
    }
    inString = null;
  }
}

void keyPressed() {
  writer.flush();  // Writes the remaining data to the file
  writer.close();  // Finishes the file
  exit();  // Stops the program
}