# Untitled - By: Kevin - 周二 10月 10 2017

import sensor, image, time,lcd
import ubinascii,m_lcd
from pyb import UART,Timer,LED

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.VGA)

clock = time.clock()
uart = UART(3,115200)

lcd.init()
while True:
    clock.tick()
    img = sensor.snapshot()
    cframe = img.compressed(quality=50)
    aaa=ubinascii.b2a_base64(cframe)
    uart.write(aaa+"\r\n")
    m_lcd.lcd2()
    lcd.display(img)
    print(clock.fps())
