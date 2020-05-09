import sensor, image, time
import network, usocket, sys

#import utils
from pyb import UART
from pyb import LED
from pyb import Servo
from pyb import Pin, Timer

blue_led  = LED(3)
green_led = LED(2)
red_led   = LED(1)

sx = Servo(1) # P7
sy = Servo(2) # P8
gainx = -5
gainy = -5

uart = UART(3, 9600)
uart.init(9600, bits=8, parity=None, stop=1)

EXPOSURE_TIME_SCALE = 0.05
thresholds = (65, 255)
roi1 = (0,0,320,240)
tim = Timer(4, freq=1)

def find_max(blobs):
    max_size=0
    for blob in blobs:
        if blob.pixels() > max_size:
            max_blob=blob
            max_size = blob.pixels()
    return max_blob

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QVGA)
sensor.set_windowing((320, 240))
sensor.skip_frames(time = 1000)
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)
sensor.set_brightness(0)
clock = time.clock()
exposure_time = sensor.get_exposure_us()
sensor.set_auto_exposure(False, \
       exposure_us = int(exposure_time * EXPOSURE_TIME_SCALE))

xPositionNow = 0
yPositionNow = 0
xPositionLast = 0
yPositionLast = 0
imageSize = 128


while(True):
    clock.tick()
    img = sensor.snapshot()
    blobs = img.find_blobs([thresholds], roi=roi1, pixels_threshold=5, area_threshold=5, merge=True)
    for b in blobs:
        max_blob=find_max(blobs)
        img.draw_rectangle(max_blob.rect())
        img.draw_cross(max_blob.cx(), max_blob.cy())
        x = -(50-max_blob.cx())
        y = 50-max_blob.cy()
        xPositionNow = x * 1
        yPositionNow = y * 1
        xPositionLast = int(xPositionNow)
        yPositionLast = int(yPositionNow)
