from machine import SoftI2C, Pin, I2C, PWM
from max30102 import MAX30102, MAX30105_PULSE_AMP_MEDIUM, MAX30105_PULSE_AMP_LOW
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


class HeartrateMonitor:
    
    ## Constants
    CHECK_FREQUENCY_HZ = 50
    CHECK_INTERVAL_MS = int(1000 / CHECK_FREQUENCY_HZ)
    MAX_BPM = 130
    MIN_INTERVAL_MS = int((60 / MAX_BPM) * 1000)
    MIN_BPM = 60
    READING_BUFFER_SIZE = CHECK_FREQUENCY_HZ * 2

    check_deadline_ms = 0

    def setup(self):
        sensor.setup_sensor()
        sensor.set_active_leds_amplitude(MAX30105_PULSE_AMP_MEDIUM)
        #sensor.set_active_leds_amplitude(MAX30105_PULSE_AMP_LOW)

    def run(self):
        if (self.check_deadline_ms is 0):
            self.check_deadline_ms = time.ticks_add(time.ticks_ms(), self.CHECK_INTERVAL_MS)
        while(True):
            if time.ticks_diff(self.check_deadline_ms, time.ticks_ms()) < 0:
                continue
            sensor.check()
            if (sensor.available()):
                # Access the storage FIFO and gather the readings (integers)
                red_sample = sensor.pop_red_from_storage()
                ir_sample = sensor.pop_ir_from_storage()

                # Print the acquired data (can be plot with Arduino Serial Plotter)
                print(f"red: {red_sample}, ir: {ir_sample}")
            self.check_deadline_ms = time.ticks_add(time.ticks_ms(), self.CHECK_INTERVAL_MS)
            


monitor = HeartrateMonitor()
monitor.setup()
monitor.run()

# lamp = Pin(15, mode=Pin.OUT)
# pwm_led = PWM(lamp)
# pwm_led.freq(100000)
# while True:
#     for duty in range(101): # Duty from 0 to 100 %
#         pwm_led.duty_u16(int((duty/100)*65_535))
#         time.sleep_ms(25)
