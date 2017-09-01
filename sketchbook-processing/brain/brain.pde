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


void setup() {
  // 32 = /dev/ttyUSB0
  // 0 = COM1
  //println(Serial.list());
  for (String foo : Serial.list()) {
    println(foo);
  }
  //mySerial = new Serial( this, Serial.list()[3], 38400 );
  // -- /dev/tty.usbmodem3227731
  mySerial = new Serial(this, "/dev/tty.usbmodem3227731", 38400 );

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

void draw() {
  ArrayList<String> listCopy;
  synchronized (allFound) {
    listCopy = new ArrayList<String>(allFound);
    allFound.clear();
  }
  if (!listCopy.isEmpty()) {
    //int millis = millis();
    for (String found : listCopy) {
      String topic;
      String byteString;
      if (found.startsWith("RIGHT: ")) {
        topic = "asOne/brain/right";
        byteString = found.substring(7).trim();
      } else if(found.startsWith("LEFT: ")) {
        topic = "asOne/brain/left";
        byteString = found.substring(7).trim();
      } else if(found.startsWith("BUTTON HIGH")) {
        publishButton("OFF");
       // mqtt.publish("asOne/score/state", one);
       // mqtt.publish("asOne/score/state", one);
        
       // mqtt.publish("asOne/score/state", one);
        continue;
      } else if(found.startsWith("BUTTON LOW")) {
        publishButton("ON");
       // mqtt.publish("asOne/score/state", two);
        continue;
      } else {
        continue;
      }
      int resultCode = Integer.parseInt(byteString);
      mqtt.publish(topic, ""+resultCode);
    }
  }
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