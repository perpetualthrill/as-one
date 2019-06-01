import processing.serial.*;
import mqtt.*;

Serial mySerial;

PrintWriter writer;
MQTTClient mqtt;

ArrayList<String> allFound = new ArrayList<String>();
int lastMillis = 0;
byte[] two = { 2 };
byte[] one = { 1 };
byte[] zero = { 0 };

int rightValue = 0;
ArrayList<Integer> rightValues = new ArrayList<Integer>();
final int BUFFER_SIZE = 60; // one second?
ArrayList<Integer> newestRightValues = new ArrayList<Integer>();
final int NEWEST_BUFFER_SIZE = 3;
int lastBeatMs = 0;
ArrayList<Integer> beats = new ArrayList<Integer>();
final int BEATS_SIZE = 4;
int bpmAverage = 0;
int lastFireTime = 0;

// Timing related constants
final int MS_PER_MINUTE = 60000;
final int MAX_HEARTRATE_BPM = 160;
final int MIN_WINDOW_MS = MS_PER_MINUTE / MAX_HEARTRATE_BPM;
final int MIN_HEARTRATE_BPM = 50;
final int MAX_WINDOW_MS = MS_PER_MINUTE / MIN_HEARTRATE_BPM;

// Magic number: a large enough difference from average to trigger
// beat detection. Empirically gathered.
final int DIFFERENCE_THRESHOLD = 12;

void setup() {
  // 32 = /dev/ttyUSB0
  // 0 = COM1
  //println(Serial.list());
  for (String foo : Serial.list()) {
    println(foo);
  }
  //mySerial = new Serial( this, Serial.list()[3], 38400 );
  // -- /dev/tty.usbmodem3227731
//  mySerial = new Serial(this, "/dev/tty.usbmodem3227731", 38400 );

  //writer = createWriter("/home/jack/foo.csv");

  mqtt = new MQTTClient(this);
  mqtt.connect("mqtt://asone-console", "brain");

  // waiting for lots of stuff, really. serial, mqtt, and so forth
  delay(10);

  mqtt.publish("asOne/hello", "hello from brain");
  mqtt.publish("asOne/score/state", one);
  mqtt.publish("asOne/scoreboard/directOnly", one);
}

/**
 * does nothing for now. needed to make the library not complain tho
 */
void messageReceived(String topic, byte[] payload) {
  println("new message: " + topic + " - " + new String(payload));
}

void enqueueRightValue(int value) {
  rightValues.add(value);
  if (rightValues.size() > BUFFER_SIZE) {
    rightValues.remove(0);
  }
  newestRightValues.add(value);
  if (newestRightValues.size() > NEWEST_BUFFER_SIZE) {
    newestRightValues.remove(0);
  }
}

void draw() {

  // copy values
  ArrayList<String> listCopy;
  synchronized (allFound) {
    listCopy = new ArrayList<String>(allFound);
    allFound.clear();
  }

  // process input and enqueue if necessary
  if (!listCopy.isEmpty()) {
    //int millis = millis();
    for (String found : listCopy) {
      String topic;
      String byteString;
      //println(found);
      if (found.startsWith("RIGHT: ")) {
        topic = "asOne/brain/right";
        byteString = found.substring(7).trim();
        rightValue = Integer.parseInt(byteString);
        enqueueRightValue(rightValue);
        //} else if(found.startsWith("LEFT: ")) {
        //  topic = "asOne/brain/left";
        //  byteString = found.substring(7).trim();
      } else if (found.startsWith("BUTTON HIGH")) {
        publishButton("OFF");
        // mqtt.publish("asOne/score/state", one);
        // mqtt.publish("asOne/score/state", one);
        // mqtt.publish("asOne/score/state", one);
        continue;
      } else if (found.startsWith("BUTTON LOW")) {
        publishButton("ON");
        // mqtt.publish("asOne/score/state", two);
        if((bpmAverage < MAX_HEARTRATE_BPM && bpmAverage > MIN_HEARTRATE_BPM) 
            && (millis() - lastFireTime) > 100) {
          mqtt.publish("asOne/fe/doHeartbeat", ""+bpmAverage);
          println("asOne/fe/doHeartbeat"+ " "+bpmAverage);
          lastFireTime = millis();
        }
        continue;
      } else {
        continue;
      }
      //int resultCode = Integer.parseInt(byteString);
      //mqtt.publish(topic, ""+resultCode);
      //println(resultCode);
    }
  }

  // check queue for beat
  int bigAverage = averageValues(rightValues);
  int newAverage = averageValues(newestRightValues);
  int currentTime = millis();
  int interval = currentTime - lastBeatMs;
  if ((newAverage - bigAverage) > DIFFERENCE_THRESHOLD) {
    lastBeatMs = currentTime;
    if (interval > MIN_WINDOW_MS && interval < MAX_WINDOW_MS) {
      beats.add(currentTime);
      if (beats.size() > BEATS_SIZE) {
        beats.remove(0);
        bpmAverage = 0;
        for (int i = 0; i < BEATS_SIZE - 1; i++) {
          bpmAverage += MS_PER_MINUTE / (beats.get(i+1) - beats.get(i));
          print(" "+(MS_PER_MINUTE / (beats.get(i+1) - beats.get(i)))+" ");
        }
        bpmAverage /= (BEATS_SIZE - 1);
        byte[] fred = { (byte) bpmAverage };
        mqtt.publish("asOne/score/rightBPM", fred);
        println(" -> "+bpmAverage);
      }
    }
  }
  
  // reset after three seconds
  if ((beats.size() > 0) && (interval > 2000)) {
    println("timed out: resetting to zero");
    beats = new ArrayList<Integer>();
    rightValues = new ArrayList<Integer>();
    newestRightValues = new ArrayList<Integer>();
    bpmAverage = 0;
    mqtt.publish("asOne/score/rightBPM", zero);
  }
  
}

int averageValues(ArrayList<Integer> list) {
  if (list.size() == 0) {
    return 0;
  }
  int total = 0;
  for (Integer val : list) {
    total += val;
  }
  return total / list.size();
}

void publishButton(String state) {
  mqtt.publish("asOne/brain/button", state);
  println("asOne/brain/button: "+state);
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
  //  writer.flush();  // Writes the remaining data to the file
  //  writer.close();  // Finishes the file
  //  mySerial.stop();
  //  exit();  // Stops the program
}
