#include "driver/ledc.h"
// Implements PWM that is guaranteed not to go over a certain % duty cycle.
// Intended for running LEDs without a resistor, oh noes :-0
class EspPwmChannel {
  private:
  unsigned int _pin, _channel, _maxDuty, _ledChangeMS;
  float _prevPercent = .5;

  // Should be safe? Vrms @ 30% seems to be 1.25V or so -- well under 1.8
  // which is the forward voltage for red LEDs
  static constexpr float MAX_DUTY_PERCENT = .3;

  // Any led attempt below this will be brought up to it
  static constexpr float MIN_PERCENT = .025;

  static constexpr float EXPANDER_MIDPOINT = .6;
  static constexpr float EXPANDER_INTENSITY = 3;

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
    // average and expand dynamic range
    float averagedPercent = (percent + _prevPercent) / 2;
    float expandedPercent = expand(averagedPercent);

    // set base darkness -- never want it to go black
    if (expandedPercent < MIN_PERCENT) expandedPercent = MIN_PERCENT;

    // duty cycle to write
    int value = (int)((float)_maxDuty * expandedPercent);

    // should not happen. better safe than sorry tho
    if (value > _maxDuty) value = _maxDuty;

    // write the value
    ledc_set_fade_time_and_start(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)_channel, value, _ledChangeMS, LEDC_FADE_NO_WAIT);

    // save input to average with next
    _prevPercent = percent;
  }

  float expand(float percent) {
    float amount = 0;
    amount = (percent - EXPANDER_MIDPOINT) * EXPANDER_INTENSITY;
    return (percent + amount);
  }

};
