from machine import SoftI2C, Pin, I2C, PWM
from max30102 import MAX30102, MAX30105_PULSE_AMP_MEDIUM, MAX30105_PULSE_AMP_LOW, MAX30105_PULSE_AMP_HIGH
import time

my_SDA_pin = 16  # I2C SDA pin number here!
my_SCL_pin = 17  # I2C SCL pin number here!
my_i2c_freq = 200000  # I2C frequency (Hz) here!

i2c = SoftI2C(sda=Pin(my_SDA_pin),
              scl=Pin(my_SCL_pin),
              freq=my_i2c_freq)

## for whatever reason must scan after soft i2c or MAX30102 gets confused
i2c.scan()
sensor = MAX30102(i2c=i2c)


## LAMP.
lamp = Pin(15, mode=Pin.OUT)
pwm_led = PWM(lamp)
pwm_led.freq(100000)
pwm_led.duty_u16(int((10/100)*65_535))


class HeartrateMonitor:
    
    ## Constants
    CHECK_FREQUENCY_HZ = 50
    CHECK_INTERVAL_MS = int(1000 / CHECK_FREQUENCY_HZ)
    MAX_BPM = 130
    MIN_INTERVAL_MS = int((60 / MAX_BPM) * 1000)
    MIN_BPM = 60
    #READING_BUFFER_SIZE = CHECK_FREQUENCY_HZ * 2
    READING_BUFFER_SIZE = CHECK_FREQUENCY_HZ // 2 
    ROLLING_AVERAGE_SIZE = 4

    ## State
    check_deadline_ms = 0
    readings = []
    prev_corrected = 0

    def setup(self):
        sensor.setup_sensor()
        #sensor.set_active_leds_amplitude(MAX30105_PULSE_AMP_MEDIUM)
        #sensor.set_active_leds_amplitude(MAX30105_PULSE_AMP_LOW)
        sensor.set_pulse_amplitude_red(MAX30105_PULSE_AMP_LOW)
        #sensor.set_pulse_amplitude_it(MAX30105_PULSE_AMP_LOW)
        sensor.set_pulse_amplitude_it(MAX30105_PULSE_AMP_HIGH)
        #sensor.set_pulse_width(118)
        #sensor.set_sample_rate(1600)

    def _set_led_percent(self, numerator: int, denominator: int) -> None:
        if numerator < 0 or denominator <= 0:
            return
        percent = numerator / denominator
        #print(f"{numerator} -- {denominator} -- {percent} -- {int(percent*65000)}")
        pwm_led.duty_u16(int(percent*65_535))

    def _enque_reading(self, reading: tuple[int, int]) -> None:
        self.readings.append(reading)
        while len(self.readings) > self.READING_BUFFER_SIZE:
            self.readings.pop(0)

    def run(self):
        if (self.check_deadline_ms is 0):
            self.check_deadline_ms = time.ticks_add(time.ticks_ms(), self.CHECK_INTERVAL_MS)
        while(True):
            ## Skip if we aren't ready to take a reading
            if time.ticks_diff(self.check_deadline_ms, time.ticks_ms()) < 0:
                continue
            ## Ok, we are going to do the thing. Set deadline for next sample before spending
            ## time with the rest of the processing
            self.check_deadline_ms = time.ticks_add(time.ticks_ms(), self.CHECK_INTERVAL_MS)

            ## Check sensor and take sample if there is one
            sensor.clear_fifo()
            sensor.check()
            reading = None
            if sensor.available():
                red_sample = sensor.pop_red_from_storage()
                ir_sample = sensor.pop_ir_from_storage()
                reading = (red_sample, ir_sample)

            if reading:
                self._enque_reading(reading)

            ## Update lamp
            min_ir = 999999
            max_ir = 0
            for (red, ir) in self.readings:
                if ir < min_ir:
                    min_ir = ir
                if ir > max_ir:
                    max_ir = ir
            if (max_ir < 1000): ## Make sure we are picked up and not floating
                self._set_led_percent(10, 100)
            else:
                min_corrected = 999999
                max_corrected = 0
                for (red, ir) in self.readings:
                    corrected = ir - min_ir
                    if corrected < min_corrected:
                        min_corrected = corrected
                    if corrected > max_corrected:
                        max_corrected = corrected
                window_sum = 0
                for (red, ir) in self.readings[-self.ROLLING_AVERAGE_SIZE:]:
                    window_sum += ir - min_ir
                rolling_average = window_sum // self.ROLLING_AVERAGE_SIZE
                new_corrected = (self.prev_corrected + max_corrected) // 2
                #print(f"{rolling_average} -- {max_corrected} -- {new_corrected}")
                self.prev_corrected = new_corrected
                self._set_led_percent(rolling_average, new_corrected)

            


monitor = HeartrateMonitor()
monitor.setup()
monitor.run()
