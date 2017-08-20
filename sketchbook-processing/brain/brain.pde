import processing.serial.*;
import mqtt.*;

Serial mySerial;

PrintWriter writer;
MQTTClient mqtt;

ArrayList<String> allFound = new ArrayList<String>();
int lastMillis = 0;

void setup() {
  // 32 = /dev/ttyUSB0
  // 0 = COM1
  mySerial = new Serial( this, Serial.list()[0], 38400 );

  writer = createWriter("/home/jack/foo.csv");

  mqtt = new MQTTClient(this);
  mqtt.connect("mqtt://asone-console", "brain");

  // waiting for lots of stuff, really. serial, mqtt, and so forth
  delay(10);

  mqtt.publish("/asOne/hello", "hello from brain");
}

/**
 * does nothing for now. needed to make the library not complain tho
 */
void messageReceived(String topic, byte[] payload) {
  println("new message: " + topic + " - " + new String(payload));
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
      String topic;
      String byteString;
      if (found.startsWith("RIGHT: ")) {
        topic = "/asOne/brain/right";
        byteString = found.substring(7).trim();
      } else if(found.startsWith("LEFT: ")) {
        topic = "/asOne/brain/left";
        byteString = found.substring(7).trim();
      } else {
        continue;
      }
      int resultCode = Integer.parseInt(byteString);
      //String line = topic +" "+resultCode;
      //writer.println(line);
      //System.out.println(line);
      mqtt.publish(topic, ""+resultCode);
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
  mySerial.stop();
  exit();  // Stops the program
}
