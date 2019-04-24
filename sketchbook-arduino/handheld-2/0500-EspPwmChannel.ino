// Implements PWM that is guaranteed not to go over a certain % duty cycle.
// Intended for running LEDs without a resistor, oh noes :-0
class EspPwmChannel {
  private:
  int _pin, _channel, _maxPercent, _maxDuty;

  public:
  // Blank constructor for array initialization. Any use of this 
  // is undefined. And bad. <3
  EspPwmChannel() { }
  
  EspPwmChannel(int pin, int channel, int resolution, int frequency, float maxPercent) {
    _pin = pin;
    _channel = channel;
    _maxPercent = maxPercent;
    _maxDuty = (int)(maxPercent * ((float)(2 ^ resolution)));
    ledcSetup(_channel, frequency, resolution);
    ledcAttachPin(_pin, _channel);
    ledcWrite(_channel, 0);
  }

  void write(float percent) {
    int value = (int)((float)_maxDuty * percent);
    // should not happen. better safe than sorry tho
    if (value > _maxDuty) value = _maxDuty;
    Serial.print("SETTING DUTY: ");
    Serial.println(value);
  }
  
};
