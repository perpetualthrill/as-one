import processing.serial.*;
Serial mySerial;

final int SAMPLE_DELAY_MS = 1;
final int SAMPLE_RATE_HZ = 1000 / SAMPLE_DELAY_MS;
final int FIFO_SIZE = SAMPLE_RATE_HZ / 4; // Quarter second buffer

final int MAX_HEARTRATE_BPM = 150;
final int MIN_WINDOW_MS = 60000 / MAX_HEARTRATE_BPM;
final int MIN_HEARTRATE_BPM = 40;
final int MAX_WINDOW_MS = 60000 / MIN_HEARTRATE_BPM;

final int DIFFERENCE_THRESHOLD = 300; // magic number

final int SENTINEL = 9999;

int[] latest = { SENTINEL, SENTINEL, SENTINEL, SENTINEL, SENTINEL };
int[] fifo = new int[FIFO_SIZE];
boolean fifoIsFilled = false;
int lastBeatMillis = 0;
String inString = null;

void setup() {
  // 32 == /dev/ttyUSB0
  mySerial = new Serial( this, Serial.list()[32], 38400 );
  for (int i = 0; i < FIFO_SIZE; i++) {
    fifo[i] = SENTINEL;
  }
  
  println("sample rate hz: "+SAMPLE_RATE_HZ+" min window ms: "+MIN_WINDOW_MS);
  println("max window ms: "+MAX_WINDOW_MS+" fifo size: "+FIFO_SIZE);
}

void draw() { 
  // all processing in serialEvent
}

boolean detect(int millis, int lastBeatMs) {
  int fifoAvg = fifoAverage(fifo);
  int latestAvg = fifoAverage(latest);
  
//  println("DETECTO: "+(millis - lastBeatMs));
  
  if ((latestAvg - fifoAvg) > DIFFERENCE_THRESHOLD 
      && (millis - lastBeatMs) > MIN_WINDOW_MS 
      && (millis - lastBeatMs) < MAX_WINDOW_MS) {
    println("avg: "+fifoAvg+" latestAvg: "+latestAvg+" difference: "+(latestAvg - fifoAvg));
    return true;
  }
  return false;
}

boolean fifoEnqueue(int[] buffer, int value) {
  boolean foundSentinel = false;
  for (int i = 0; i < buffer.length - 1; i++) {
    buffer[i] = buffer[i+1];
    if (buffer[i] == SENTINEL) {
      foundSentinel = true;
    }
  }
  buffer[buffer.length - 1] = value;
  foundSentinel |= value == SENTINEL;
  return foundSentinel;
}

int fifoAverage(int[] buffer) {
  int sum = 0;
  for (int i = 0; i < buffer.length; i++) {
    sum += buffer[i];
  }
  return sum / buffer.length;
}

void serialEvent (Serial myPort) {
  inString = myPort.readStringUntil('\n');
  String value = null;
  int valueInt = SENTINEL;
  if (null != inString) {
    value = inString;
    int millis = millis();
    try {
      valueInt = Integer.parseInt(value.trim());
    } 
    catch (NumberFormatException e) {
      valueInt = SENTINEL;
      println("integer did not parse: "+e);
    }
    
    valueInt = int(map(float(valueInt), 0, 660, 0, 1023));
    if (valueInt == 1023) {
      valueInt = SENTINEL;
    }

    boolean hasEnoughData = !fifoEnqueue(fifo, valueInt);
    if (fifoIsFilled && !hasEnoughData) {
      fifoIsFilled = false;
      println("FIFO: "+fifoIsFilled);
      lastBeatMillis = 0;
    }
    if (!fifoIsFilled && hasEnoughData) {
      fifoIsFilled = true;
      println("FIFO: "+fifoIsFilled);
      lastBeatMillis = millis + MAX_WINDOW_MS;
    }
    fifoEnqueue(latest, valueInt);

    if (hasEnoughData && detect(millis, lastBeatMillis)) {
      float bpm = 60000f / (millis - lastBeatMillis);
      lastBeatMillis = millis;
      println("DETECTED: "+bpm);
    }
    
    inString = null;
  }
}

void keyPressed() {
  exit();  // Stops the program
}