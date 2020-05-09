

'''
 SPI  openmv4+esp8266
    GPIO    NodeMCU   Name  |   openmv
 ===================================
 GPIO16       D0      SS    |   P3  (PB12) 26
 GPIO13       D7      MOSI  |   P0  (PB15) 23
 GPIO12       D6      MISO  |   P1  (PB14) 24
 GPIO14       D5      SCK   |   P2  (PB13) 25
 GPIO2        D4  (低电压触发)    P7
   GND                          GND 22
'''

import pyb, ustruct
import sensor, image, time
from pyb import Pin


'''
  320*480 黑白 3-4k  0.1秒/张
  640*480 黑白 15k   0.3秒/张
  用电260ma
'''

def sendblob(spi,pin,data,frame):
  print("Waiting for Arduino...")
  while(pin.value()): pass
  print("begin Sent Data!")
  spi.send(data)
  spi.send(frame)
  while(not pin.value()): pass
  print("end Sent Data!")

def sendimage(spi,pin):
  global last_time
  frame = sensor.snapshot()
  frame.draw_string(0, 0, str(time.ticks()-last_time), color=(255, 0, 0),scale=2)
  last_time= time.ticks()
  #frame = frame.compressed(quality=45) 用此语句图片过大时会内存报错
  frame = frame.compress(45)
  print("size=", str(frame.size()),"time=",time.ticks()-last_time)

  #减少生成data内存占用
  #data = ustruct.pack("<bi%ds" % frame.size(), 85, frame.size(), frame) # 85 is a sync char.
  data = ustruct.pack("<bi" , 85, frame.size()) # 85 is a sync char.
  sendblob(spi,pin,data,frame)


sensor.reset()
sensor.set_contrast(1)
sensor.set_brightness(1)
sensor.set_framesize(sensor.VGA)
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_vflip(True)   #openmv4 垂直翻转
sensor.skip_frames(20)

spi = pyb.SPI(2, pyb.SPI.SLAVE, polarity=0, phase=0)
pin = pyb.Pin("P3", pyb.Pin.IN, pull=pyb.Pin.PULL_UP)
print("start...")
last_time= time.ticks()
# key state
key_state = 0
key_event = 0
#key = Pin('PD13', Pin.IN, Pin.PULL_UP)
key = Pin('P7', Pin.IN, Pin.PULL_UP)

#如果key检测到低电压,则进行esp8266传图
while(True):
  # remove key jitter
  if (key_state == 0):
    if (key.value() == 0):
        key_state = key_state + 1
  elif (key_state < 2):
    key_state = key_state + 1
  elif (key_state == 2):
    key_state = key_state + 1
    key_event = 1
  else:
    if (key.value() == 1):
      key_state = 0
  if (key_event == 1):
    try:
      sendimage(spi,pin)
    except OSError as e:
      print("socket error: ", e)
      #sys.print_exception(e)
  key_event = 0

