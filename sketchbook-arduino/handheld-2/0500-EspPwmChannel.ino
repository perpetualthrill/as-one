#include "driver/ledc.h"
// Implements PWM that is guaranteed not to go over a certain % duty cycle.
// Intended for running LEDs without a resistor, oh noes :-0
class EspPwmChannel {
  private:
  unsigned int _pin, _channel, _maxDuty, _ledChangeMS;

  // Should be safe? Vrms @ 30% seems to be 1.25V or so -- well under 1.8
  // which is the forward voltage for red LEDs
  static constexpr float MAX_DUTY_PERCENT = .3;

  static constexpr float EXPANDER_MIDPOINT = .5;
  static constexpr float EXPANDER_INTENSITY = 2.3;

  public:
  // Blank constructor for array initialization. Any use of this 
  // is undefined. And bad. <3
  EspPwmChannel() { }
  
  EspPwmChannel(int pin, int channel, int resolution, int frequency, int ledChangeMS) {
    _pin = pin;
    _channel = channel;
    _maxDuty = (int)(MAX_DUTY_PERCENT * pow(2, resolution));
    _ledChangeMS = ledChangeMS;
    ledc_fade_func_install(_channel);
    ledcSetup(_channel, frequency, resolution);
    ledcAttachPin(_pin, _channel);
    ledcWrite(_channel, 0);
  }

  void write(float percent) {
    float expandedPercent = expand(percent);
    int value = (int)((float)_maxDuty * expandedPercent);
    // should not happen. better safe than sorry tho
    if (value > _maxDuty) value = _maxDuty;
    ledcWrite(_channel, value);
    //ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)_channel, value, _ledChangeMS);
  }

  float expand(float percent) {
    float amount = 0;
    amount = (percent - EXPANDER_MIDPOINT) * EXPANDER_INTENSITY;
    return (percent + amount);
  }

};
