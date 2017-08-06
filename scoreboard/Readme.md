# Scoreboard

## Wiring/PinOut

![ESP8266 Breakout](datasheets/Adafruit_ESP8266_Breakout.jpg)

Two pins from the ESP8266 are connected for output to the WS2811 LED strings. 

* RX/GPIO3/RXD0.  On the NodeMCU and Adafruit boards, this pin is labeled "RX".
* D4/GPIO2/TXD1.  On the NodeMCU board, this pin is labeled "D4".  On the Adadruit board, this pin is labeled "#2".

An additional pin from the ESP8266 is connected for future extension.

* 15/GPIO15/TXD2.  On the Adafruit board, this pin is labeled "15".

A 75HCT245 transceiver accepts "RX" on A1/2, "D4" on A2/3, "15" on A3/4.  Outputs from A1 and A2 are jumper-selectable and connect to the DATA line of the WS2811 LED strips via a 220 Ohm resistor.

## MQTT Messages

The Scoreboard connects to the MQTT broker, and will reconnect every 500 ms if disconnected.

### Publishes

The Scoreboard announces itself every two seconds by way of a heartbeat, indicating the elapsed time in milliseconds since bootup.

* "asOne/score/heartbeat": current millis() every two seconds

### Subscribes

The Scoreboard subscribes to all topics within the "asOne/score" tree.

* "asOne/score/#": multi-level wildcard
	
### Processes

The Scoreboard processes the following topics.

#### Indirect Methods

With indirect methods, the Scoreboard will process high-level messages and translate them to the LEDs.

* "asOne/score/state": [0,1,2] current state of the system
  * 0: idle/not playing.  scoreboard will mess around on its own.
  * 1: active/playing.  scoreboard will process the following subscriptions
  * 2: won/flames.  scoreboard will do some (garish) fanfare while the flames are shooting
* "asOne/score/logo": bytestream castable to CRGB, applied to all LEDs in logo
* "asOne/score/timer": [0-99] current timer to display on the countdown
* "asOne/score/leftBPM": [0-199] current heartrate to display on left score
* "asOne/score/rightBPM": [0-199] current heartrate to display on right score

`CRGB` is defined in the [FastLED library](https://github.com/FastLED/FastLED/wiki/Pixel-reference):

    CRGB led;
	led.red = 50; // byte value for red
	led.green = 128; // byte value for green
	led.blue = 255; // byte value for blue
	
So, to set the `logo` object to be a light green:

    CRGB logo;
	led.red = 0; // byte value for red
	led.green = 255; // byte value for green
	led.blue = 127; // byte value for blue
    
    mqtt.publish("asOne/score/logo", (uint8_t *)logo, sizeof(logo));

At a primative level, needing no access to `FastLED`:

    byte logo[3];
	led[0] = 0; // byte value for red
	led[1] = 255; // byte value for green
	led[2] = 127; // byte value for blue
    
    mqtt.publish("asOne/score/logo", (uint8_t *)logo, sizeof(logo));

#### Direct Methods

With direct methods, the Scoreboard processes CRGB arrays encoded to bytestream, allowing complete control of the LEDs directly.  See below for ordering.

  * "asOne/score/logo/direct": bytestream castable to CRGB[22]
  * "asOne/score/timer/direct: bytestream castable to CRGB[26]
  * "asOne/score/leftBPM/direct": bytestream castable to CRGB[47]
  * "asOne/score/rightBPM/direct": bytestream castable to CRGB[47]
  
## Physical Layout

### Scoreboard Diagram 

![Scoreboard Diagram](diagram.png)

### Direct Addressing

The "asOne/score/*/direct" topics are CGRB arrays encoded as bytestreams.  

#### Logo

    const byte nLogoLED = 22;
    CRGB pixels[nLogoLED];
    fill_rainbow(pixels, nLogoLED, 0);
    mqtt.publish("asOne/score/logo/direct", (uint8_t *)pixels, sizeof(CRGB)*nLogoLED);

As before, access to the `FastLED` library is not required:

    const byte nLogoLED = 22;
    byte pixels[nLogoLED][3];
	led[0][0] = 0; // byte value for red
	led[0][1] = 255; // byte value for green
	led[0][2] = 127; // byte value for blue
    
    mqtt.publish("asOne/score/logo/direct", (uint8_t *)pixels, sizeof(pixels));
	
#### Timer

    const byte nTimerLED = 26;
    CRGB pixels[nTimerLED];
    fill_rainbow(pixels, nTimerLED, 0);
    mqtt.publish("asOne/score/timer/direct", (uint8_t *)pixels, sizeof(CRGB)*nTimerLED);

#### leftBPM

    const byte nBPMLED = 47;
    CRGB pixels[nBPMLED];
    fill_rainbow(pixels, nBPMLED, 0);
    mqtt.publish("asOne/score/leftBPM/direct", (uint8_t *)pixels, sizeof(CRGB)*nBPMLED);

#### rightBPM

    const byte nBPMLED = 47;
    CRGB pixels[nBPMLED];
    fill_rainbow(pixels, nBPMLED, 0);
    mqtt.publish("asOne/score/rightBPM/direct", (uint8_t *)pixels, sizeof(CRGB)*nBPMLED);


