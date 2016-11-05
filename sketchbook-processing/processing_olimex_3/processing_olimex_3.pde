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
  // 32 == /dev/ttyUSB0
  // 0 = COM1
  mySerial = new Serial( this, Serial.list()[0], 38400 );

  writer = createWriter("/home/jack/foo.csv");

  // window stuff from tutorial
  size(1000, 400);        
  background(0);
  strokeWeight(2);
  stroke(128);

  delay(10);
}

void draw() { 

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
    if (!inString.equals(previous)) {
      //int millis = millis();
      // print("got one at "+millis+" -- "+inString);
      writer.print(inString);
    }
    previous = new String(inString);
    float dataInLast = dataIn;
    dataIn = float(inString);
    if (dataIn == Float.NaN) {
      dataIn = dataInLast;
    } else {
      dataIn = map(dataIn, 0, 255, 0, height);
    }
    inString = null;
  }
}

void keyPressed() {
  writer.flush();  // Writes the remaining data to the file
  writer.close();  // Finishes the file
  exit();  // Stops the program
}