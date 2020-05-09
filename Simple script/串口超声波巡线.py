import sensor,pyb,math,time, image
from pyb import LED,Pin,Timer,UART

sensor.reset() #初始化摄像头
sensor.set_pixformat(sensor.GRAYSCALE)#设置图像色彩格式，有RGB565色彩图和GRAYSCALE灰度图两种
sensor.set_framesize(sensor.QQVGA)
#设置图像像素大小，sensor.QQVGA: 160x120，sensor.QQVGA2: 128x160 (一般用于LCD
#扩展板)，sensor.QVGA: 320x240，sensor.QQCIF: 88x72，sensor.QCIF: 176x144，sensor.CIF: 352x288
sensor.skip_frames(20)#设置跳过的帧数 稳定设置，默认300
#指示灯
LED_Red = pyb.LED(1)
LED_Green = pyb.LED(1)
LED_Blue = pyb.LED(3)
#黑色点阈值
black_threshold = [(0, 64)]
#xy平面误差数据
err_x = 0
err_y = 0
#高度
high = 0
scrc = 0
rebuf_temp = 0
#计数器
m = 0
error_sta = 0
timercnt = 0
timerflag = 0
#发送数据
uart_buf = bytearray([0x55,0xAA,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xAA])
#接收数据
uart_rebuf = bytearray(2)
uart_testbuf = bytearray([0x55,0x88,0xA1,0x02,high>>8,high,scrc])

#串口三配置
uart = UART(3, 9600)
uart.init(9600, bits=8, parity=None, stop=1)
uart.write(uart_testbuf)
clock = time.clock()#打开时钟
while(True):
    clock.tick()
    img = sensor.snapshot()
    #画坐标
    img.draw_line([0, 60, 160, 60])
    img.draw_line([80, 0, 80, 120])
    img.draw_string(80, 0, "X")
    img.draw_string(150, 50, "Y")
    #寻找blob
    blobs = img.find_blobs(black_threshold)

    if(high>2000):timerflag = 1
    else:
        timerflag = 0
        timercnt = 0
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
                if(timercnt < 200):#200内定高
                    [x,y,w,h]=blobs[largest_blob].rect()
                    err_x = int(60 - y - h/2)
                    err_y = int(x + w/2 - 80)
                else:  #寻线
                    [x,y,w,h]=blobs[largest_blob].rect()
                    if(x == 0):
                        err_x = int(60 - y - h/2)  #坐标转换
                        err_y = -23
                    else:
                        err_x = int(60 - y - h/2)
                        err_y = int(x  - 80)
        #画跟踪线
        img.draw_line([int(err_y+80), int(60-err_x), int(err_y+80), 60])
        img.draw_line([int(err_y+80), int(60-err_x), 80, int(60-err_x)])
        img.draw_cross(int(x+w/2),int(y+h/2))
        img.draw_rectangle([x, y, w, h])
    else:
       err_x = 0
       err_y = 0
    m = m + 1  #数据发送频率
    num = uart.any()#读取串口接收到的字符数
    #串口接收超声波数据
    if(num == 2):
      uart_rebuf[0] = uart.readchar()
      uart_rebuf[1] = uart.readchar()
      rebuf_temp = uart_rebuf[0] = int(uart_rebuf[0] << 8 | uart_rebuf[1])
      if(rebuf_temp <= 4000):
        high = rebuf_temp
    if(num > 2):
      error_sta = error_sta + 1
      for i in range(num):
           uart.readchar()
    #数组中数据写入
    uart_buf = bytearray([0x55,0xAA,0x10,0x01,0x00,high>>8,high,err_x>>8,err_x,
                                    err_y>>8,err_y,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xAA,0x0D,0x0a])

    scrc = 0x88 + 0xA1 + 0x02 + (high >> 8) + high
    uart_testbuf = bytearray([0x55,0x88,0xA1,0x02,high>>8,high,scrc])
    if(m >= 2):
       m = 0
       print(error_sta)
       print(num)
       print(uart_rebuf)
       print("high=",high)
       print('err_x=',err_x,'err_y=',err_y)
       print('\n')
       LED_Green.toggle()
       uart.write(uart_testbuf)




