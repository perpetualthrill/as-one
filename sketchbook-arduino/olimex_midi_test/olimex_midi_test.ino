#include "Arduino.h" // needed to define CORE_TEENSY
#include <MIDI.h>

// Arduino.h defines CORE_TEENSY and MIDI.h uses that value to
// choose Serial1 for communication instead of Serial
#if defined(CORE_TEENSY)
#define FOO "is teensy"
#else
#define FOO "not teensy"
#endif

int prevtime;

void setup() {
  MIDI.begin(MIDI_CHANNEL_OMNI);

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("started: ");
  Serial.println(FOO);

  prevtime = millis();
}

void loop() {
  int newtime = millis();
  if ((newtime - prevtime) > 500) {
    prevtime = newtime;
    int r = random(255);
    int pitch = 0;
    int velocity = 0;
    if (r <= 127) {
      pitch = r;
    } else {
      pitch = r - 127;
      velocity = 127;
    }
    MIDI.sendNoteOn(pitch, velocity, 1);
    Serial.print("sent ");
    Serial.println(r);
  }
}

