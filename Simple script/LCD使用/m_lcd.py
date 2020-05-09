
import time
from pyb import Pin, SPI
#lcd引脚对应接口
# SCL    P2
# SDA    P0
# CS     P3
# DC/RS  P8
# RESET  P7
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

def lcd2():
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
