#include <math.h>

// Implements PWM that is guaranteed not to go over a certain % duty cycle.
// Intended for running LEDs without a resistor, oh noes :-0
class EspPwmChannel {
  private:
  int _pin, _channel, _maxDuty;

  // Should be safe? Max RMS @ 30% seems to be 1.25V or so -- well under 1.8
  // which is the forward voltage for red LEDs
  static constexpr float MAX_DUTY_PERCENT = .3; 

  public:
  // Blank constructor for array initialization. Any use of this 
  // is undefined. And bad. <3
  EspPwmChannel() { }
  
  EspPwmChannel(int pin, int channel, int resolution, int frequency) {
    _pin = pin;
    _channel = channel;
    _maxDuty = (int)(MAX_DUTY_PERCENT * pow(2, resolution));
    ledcSetup(_channel, frequency, resolution);
    ledcAttachPin(_pin, _channel);
    ledcWrite(_channel, 0);
  }

  void write(float percent) {
    int value = (int)((float)_maxDuty * percent);
    // should not happen. better safe than sorry tho
    if (value > _maxDuty) value = _maxDuty;
    ledcWrite(_channel, value);
  }

  // TO-DO: Figure out a power function such that the pulses are more visually distinct.
  // Like, keep an average, amplify values above, and diminish ones below. But in a
  // continuous fashion. In DSP, an expander function (opposite of compression)
  
};
