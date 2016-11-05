#include "Arduino.h" // needed to define CORE_TEENSY for MIDI.h
#include <MIDI.h>

const int midiChannel = 1;

void setup() {
  // Serial1 is the MIDI port per MIDI.h
  // OMNI is receive channel -- not used
  MIDI.begin(MIDI_CHANNEL_OMNI);

  // Serial2 is the MOD-EKG board
  Serial2.begin(115200);

  // Wait a few ms for ports to settle
  delay(10);
}

void loop() {
  int bytecount = Serial2.available();
  if (bytecount) {
    for (int i = 0; i < bytecount; i++) {
      int val = Serial2.read();
      midiSend(val);
    }
  }
}

void midiSend(int value) {
  int pitch = 0;
  int velocity = 0;
  if (value <= 127) {
    pitch = value;
  } else {
    pitch = value - 127;
    velocity = 127;
  }
  MIDI.sendNoteOn(pitch, velocity, midiChannel);
}

