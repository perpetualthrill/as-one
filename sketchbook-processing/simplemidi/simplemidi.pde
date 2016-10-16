// SimpleMidi.pde

import themidibus.*; //Import the library
import javax.sound.midi.MidiMessage; 

MidiBus myBus; 

int currentColor = 0;
int midiInDevice  = 0;
int midiOutDevice = 1;

void setup() {
  size(480, 320);
  MidiBus.list(); 
  myBus = new MidiBus(this, midiInDevice, midiOutDevice); 
}

void draw() {
  background(currentColor);
}

void midiMessage(MidiMessage message, long timestamp, String bus_name) { 
  int status = (int)(message.getMessage()[0] & 0xFF);
  if (status == 254) {
    return; // active sensing message. discard
  }
  
  int note = (int)(message.getMessage()[1] & 0xFF) ;
  //int note = (int)message.getMessage()[1];
  int vel = (int)(message.getMessage()[2] & 0xFF);

  println("Bus " + bus_name + ": Note "+ note + ", vel " + vel+", status "+status);
  println("aka "+(note+vel));
  if (vel > 0 ) {
   currentColor = vel*2;
  }
}