# Snapshot Example
#
# Note: You will need an SD card to run this example.
#
# You can use your OpenMV Cam to save image files.

import sensor
import image
import pyb
import time
from pyb import UART

RED_LED_PIN = 1
BLUE_LED_PIN = 3

uart = UART(3, 115200)


sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.RGB565) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.VGA) # or sensor.QQVGA (or others)
pyb.LED(RED_LED_PIN).on()
sensor.skip_frames(time = 2000) # Give the user time to get ready.

pyb.LED(RED_LED_PIN).off()
pyb.LED(BLUE_LED_PIN).off()

# print("You're on camera!")

count = 0


while True:

    if (uart.any()):
        data = uart.readall()
		pyb.LED(BLUE_LED_PIN).on()
        sensor.snapshot().save("%d.jpg"%pyb.rng()) # or "example.bmp" (or others)
        print("Take Photo %d"%(count))
        pyb.LED(BLUE_LED_PIN).off()

        count = count + 1
    else:
        # 啥事儿不干
        sensor.snapshot()
        time.sleep(100)
