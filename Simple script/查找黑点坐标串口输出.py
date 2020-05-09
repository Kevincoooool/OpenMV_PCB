import sensor,time,pyb,math, lcd
#import m_lcd
from pyb import Pin, Timer, LED, UART
err_min = 120
err_max = 160
#黑色点阈值
black_threshold = [(0,100)]
#xy平面误差数据
err_x = 0
err_y = 0

pin0 = Pin('P0', Pin.OUT_PP, Pin.PULL_NONE)

pin1 = Pin('P1', Pin.OUT_PP, Pin.PULL_NONE)

#发送数据
uart_buf = bytearray([0x55,0xAA,0x00,0x00,0x00,0x00,0xAA])
red_led   = LED(1)
green_led = LED(2)
blue_led  = LED(3)
ir_led    = LED(4)
uart_data = 0
#读取串口缓存
def uart_read_buf():
    uart_data = 0
    if(uart.any()):#判断是否有串口数据
        uart_data = uart.readchar()
    if uart_data == 0XAA:
        err_set = err_set + 1
    if uart_data == 0XBB:
        err_set = err_set - 1
#串口三配置
uart = UART(3,115200)
uart.init(115200, bits=8, parity=None, stop=1)

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # or sensor.GRAYSCALE
sensor.set_framesize(sensor.QVGA) # Special 128x160 framesize for LCD Shield.
sensor.skip_frames(100)#相机自检几张图片
sensor.set_auto_whitebal(False)#关闭白平衡
clock = time.clock()#打开时钟
#lcd.init()
while(True):
    clock.tick()
    img = sensor.snapshot()
    #m_lcd.lcd2()
    #寻找blob
    blobs = img.find_blobs(black_threshold)
    if blobs:
        most_pixels = 0
        largest_blob = 0
        for i in range(len(blobs)):
            #目标区域找到的颜色块可能不止一个，找到最大的一个
            if blobs[i].pixels() > most_pixels:
                most_pixels = blobs[i].pixels()
                largest_blob = i
                #位置环用到的变量
                [x,y,w,h]=blobs[largest_blob].rect()
                err_x = int(120 - blobs[largest_blob].cy())
                err_y = int(blobs[largest_blob].cx() -160)
        img.draw_cross(blobs[largest_blob].cx(),blobs[largest_blob].cy())#调试使用
        img.draw_rectangle(blobs[largest_blob].rect())
        #lcd.display(img)
    else:
        err_x = 0
        err_y = 0
    if(uart.any()):#判断是否有串口数据
        uart_data = uart.readchar()
    if uart_data == 0XAA:
        err_min = err_min + 1
    elif uart_data == 0XBB:
        err_max = err_min - 1
    elif uart_data == 0XCC:
        err_max = err_max + 1
    elif uart_data == 0XDD:
        err_max = err_max - 1
    if err_x > err_min and err_x < err_max:
        red_led.on()
        blue_led.off()
        pin0.value(0)
        pin1.value(0)
    elif err_x < err_min:
        red_led.off()
        blue_led.on()
        pin0.value(1)
        pin1.value(0)
    elif err_x > err_max:
        red_led.off()
        blue_led.on()
        pin0.value(0)
        pin1.value(1)

    #数组中数据写入
    print(err_x,err_min,err_max)


