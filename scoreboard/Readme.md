# Scoreboard

![Scoreboard Diagram](diagram.png)

## MQTT Messages

### Publishes
    - "asOne/score/heartbeat": current millis() every two seconds

### Subscribes
    - "asOne/score/#": multi-level wildcard
	
### Processes
    - "asOne/score/state": [0,1,2] current state of the system
        - 0: idle/not playing.  scoreboard will mess around on its own.
        - 1: active/playing.  scoreboard will process the following subscriptions
        - 2: won/flames.  scoreboard will do some (garish) fanfare while the flames are shooting
    - "asOne/score/logo": bytestream castable to CRGB, applied to all LEDs in logo
        - "asOne/score/logo/direct": bytestream castable to CRGB[22]
    - "asOne/score/timer": [0-99] current timer to display on the countdown
       - "asOne/score/timer/direct: bytestream castable to CRGB[26]
    - "asOne/score/leftBPM": [0-199] current heartrate to display on left score
        - "asOne/score/leftBPM/direct": bytestream castable to CRGB[47]
    - "asOne/score/rightBPM": [0-199] current heartrate to display on right score
        - "asOne/score/rightBPM/direct": bytestream castable to CRGB[47]
