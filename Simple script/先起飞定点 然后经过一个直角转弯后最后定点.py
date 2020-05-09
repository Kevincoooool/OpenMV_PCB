
import sensor,time
from pyb import Pin, Timer, UART, ExtInt
#黑色点阈值
black_threshold = [(0, 128)]
#xy平面误差数据
err_x = 0
err_y = 0
#计数器
timercnt = 0
timerflag = 1
stopcnt = 0
stopflag = 0
stopflagflag = 0
#发送数据
uart_buf = bytearray([0x55,0xAA,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xAA])
stop_buf = bytearray([0x55,0xAA,0x10,0x03,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xAA])
#串口三配置
uart = UART(3, 115200)
uart.init(115200, bits=8, parity=None, stop=1)

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)#设置灰度信息
sensor.set_framesize(sensor.QQVGA)#设置图像大小
sensor.skip_frames(10)#相机自检几张图片
sensor.set_auto_whitebal(False)#关闭白平衡
while(True):
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
                #起飞悬停
                if(timercnt < 150):
                    [x,y,w,h]=blobs[largest_blob].rect()
                    err_x = int(60 - y - h/2)
                    err_y = int(x + w - 80 - h/2)
                #前进
                else:
                    [x,y,w,h]=blobs[largest_blob].rect()
                    if(x == 0 and y != 0 and y+h != 119):#第一段直线
                        err_x = int(60 - y - h/2)
                        err_y = -10
                    if(x != 0 and y == 0 and y+h != 119):#左转
                        err_x = 25
                        err_y = int(x - 80)
                    if(x != 0 and y == 0 and y+h == 119):#第二段直线
                        err_x = 5
                        err_y = int(x - 80)
                    if(x != 0 and y != 0 and y+h != 119):#停止
                        err_x = int(60 - y - h/2)
                        err_y = int(x + w - 80 - h/2)
                        if(stopcnt<100):stopcnt = stopcnt + 1
                        else:stopflag = 1

    else:
       err_x = 0
       err_y = 0
    #数组中数据写入
    uart_buf = bytearray([0x55,0xAA,err_x>>8,err_x,err_y>>8,err_y,0xAA])

    print(err_x,err_y)
    uart.write(uart_buf)

