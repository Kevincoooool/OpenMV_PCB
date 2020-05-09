
import sensor,time,pyb,math,time
from pyb import Pin, Timer, LED, UART
#黑色点阈值
black_threshold = [(0, 64)]
#xy平面误差数据
err_x = 0
err_y = 0
#计数器
timercnt = 0
timerflag = 1
#发送数据
uart_buf = bytearray([0x55,0xAA,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xAA])
#串口三配置
uart = UART(3, 115200)
uart.init(115200, bits=8, parity=None, stop=1)

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)#设置灰度信息
sensor.set_framesize(sensor.QQVGA)#设置图像大小
sensor.skip_frames(20)#相机自检几张图片
sensor.set_auto_whitebal(False)#关闭白平衡
clock = time.clock()#打开时钟
while(True):
    clock.tick()
    img = sensor.snapshot()
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
                if(timerflag == 1):timercnt = timercnt + 1
                #位置环用到的变量
                if(timercnt < 200):
                    [x,y,w,h]=blobs[largest_blob].rect()
                    err_x = int(60 - y - h/2)
                    err_y = int(x + w - 80)
                else:
                    [x,y,w,h]=blobs[largest_blob].rect()
                    if(x == 0):
                        err_x = int(60 - y - h/2)
                        err_y = -23
                    else:
                        err_x = int(60 - y - h/2)
                        err_y = int(x  - 80)
                img.draw_cross(blobs[largest_blob].cx(),blobs[largest_blob].cy())#调试使用
                img.draw_rectangle(blobs[largest_blob].rect())
    else:
       err_x = 0
       err_y = 0
    #数组中数据写入
    uart_buf = bytearray([0x55,0xAA,0x10,err_x>>8,err_x,err_y>>8,err_y,0xAA])
    uart.write(uart_buf)
    print(err_x,err_y)
