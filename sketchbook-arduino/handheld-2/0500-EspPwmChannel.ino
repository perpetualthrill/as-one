#include "driver/ledc.h"
// Implements PWM that is guaranteed not to go over a certain % duty cycle.
// Intended for running LEDs without a resistor, oh noes :-0
class EspPwmChannel {
  private:
  unsigned int _pin, _channel, _maxDuty, _ledChangeMS;
  float _prevPercent = .5;

  // Values above midpoint will be raised, and below lowered. Intensity
  // is a coefficient on the amount that will happen
  // midpoint to be calculated by averaging incoming values. previously it 
  // was fixed at .6 but it turns out the individual sensors are more 
  // variable than that.
  float _midpoint = .6;
  float _prevMidpoint = .6;
  static const int MIDPOINT_COUNT = REPORT_FREQUENCY_HZ; // i.e. one second
  float _midpoints[MIDPOINT_COUNT];
  int _mpIndex = 0;

  // Should be safe? Vrms @ 30% seems to be 1.25V or so -- well under 1.8
  // which is the forward voltage for red LEDs.
  // Bringing it down to .22 to not be over-bright on the playa
  static constexpr float MAX_DUTY_PERCENT = .22;

  // Any led attempt below this will be brought up to it
  static constexpr float MIN_PERCENT = .025;

  // factor by which brightening or darkening are multiplied by
  static constexpr float EXPANDER_INTENSITY = 3.5;

  static const int PWM_RESOLUTION = 10;
  static const int PWM_FREQUENCY = 18000; // this is near the max we can go given 10 bit resolution
  static const int LED_CHANGE_MS = REPORT_PERIOD_MS - 3; // get there a little quicker than we strictly have to

  public:
  // Blank constructor for array initialization. Any use of objects created by it
  // is undefined. And bad. <3
  EspPwmChannel() { }

  EspPwmChannel(int pin, int channel) {
    _pin = pin;
    _channel = channel;
    _maxDuty = (int)(MAX_DUTY_PERCENT * pow(2, PWM_RESOLUTION));
    _ledChangeMS = LED_CHANGE_MS;
    ledc_fade_func_install(_channel);
    ledcSetup(_channel, PWM_FREQUENCY, PWM_RESOLUTION);
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

    // recalculate midpoint every _count readings
    _midpoints[_mpIndex] = percent;
    _mpIndex++;
    if (_mpIndex >= MIDPOINT_COUNT) {
      float total = 0;
      for (int i = 0; i < MIDPOINT_COUNT; i++) {
        total += _midpoints[i];
      }
      // average in previous value to make this less blinky
      float newMidpoint = ((total / MIDPOINT_COUNT) + _prevMidpoint) / 2.0;
      _prevMidpoint = _midpoint;
      _midpoint = newMidpoint;
      _midpoint = total / MIDPOINT_COUNT;
      _mpIndex = 0;
    }
  }

  float expand(float percent) {
    float amount = 0;
    amount = (percent - _midpoint) * EXPANDER_INTENSITY;
    return (percent + amount);
  }

};
