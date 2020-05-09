# LCD Example
#
# Note: To run this example you will need a LCD Shield for your OpenMV Cam.
#
# The LCD Shield allows you to view your OpenMV Cam's frame buffer on the go.

import sensor, image, lcd
from pyb import Pin, SPI

cs  = Pin("P3", Pin.OUT_OD)
rst = Pin("P7", Pin.OUT_PP)
rs  = Pin("P8", Pin.OUT_PP)
# The hardware SPI bus for your OpenMV Cam is always SPI bus 2.
spi = SPI(2, SPI.MASTER, baudrate=int(1000000000/66), polarity=0, phase=0)
def write_command_byte(c):
    cs.low()
    rs.low()
    spi.send(c)
    cs.high()

def write_data_byte(c):
    cs.low()
    rs.high()
    spi.send(c)
    cs.high()

def write_command(c, *data):
    write_command_byte(c)
    if data:
        for d in data:
            write_data_byte(d)

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QQVGA2) # Special 128x160 framesize for LCD Shield.
lcd.init() # Initialize the lcd screen.

while(True):
    write_command(0x2a)
    write_data_byte(0x00)
    write_data_byte(0x02)
    write_data_byte(0x00)
    write_data_byte(0x81)
    write_command(0x2b)
    write_data_byte(0x00)
    write_data_byte(0x01)
    write_data_byte(0x00)
    write_data_byte(0xa0)
    write_command(0x2c)
    lcd.display(sensor.snapshot()) # Take a picture and display the image.
