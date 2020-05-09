import sensor, image, time, clock, lcd
#from pyb import LED
#from pyb import UART,Timer
from machine import UART,Timer
#uart = UART(3,115200)#初始化串口 波特率 115200

fm.register(board_info.PIN15,fm.fpioa.UART1_TX)
fm.register(board_info.PIN17,fm.fpioa.UART1_RX)

uart = UART(UART.UART1,115200,8,0,0,timeout=1000, read_buf_len=4096)

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQQVGA) # 80x60 (4,800 pixels)

sensor.run(1)

sensor.skip_frames(30)     # WARNING: If you use QQVGA it may take seconds
sensor.set_auto_gain(False) # 在进行颜色追踪时，必须关闭
sensor.set_auto_whitebal(False) # 在进行颜色追踪时，必须关闭
#clock = time.clock()                # to process a frame sometimes.

clock = clock.clock()                # 初始化时钟用于计算FPS
lcd.init() #初始化LCD

up_roi   = [0,   0, 80, 15]#上采样区0
down_roi = [0, 55, 80, 15]#下采样区0
left_roi = [0,   0, 25, 60]#左采样区0
righ_roi = [55, 0,  25, 40]#右采样区0
thresholds = [(30, 0, -128, 127, -128, 127)] #检测黑色物体的颜色阈值，根绝不同的环境，需要有适当的修改
THRESHOLD = (0,100) # Grayscale threshold for dark things...

class Dot(object):
    x = 0
    y = 0
    pixels = 0
    num = 0
    ok = 0
    flag = 0

class singleline_check():
    ok = 0
    flag1 = 0
    flag2 = 0
    rho_err = 0
    theta_err = 0

class receive(object):
    uart_buf = []
    _data_len = 0
    _data_cnt = 0
    state = 0
Receive=receive()

class ctrl(object):
    work_mode = 0x02 #工作模式.默认是点检测，可以通过串口设置成其他模式

ctrl=ctrl()

dot  = Dot()
up   = singleline_check()
down = singleline_check()
left = singleline_check()
righ = singleline_check()
line = singleline_check()
singleline_check = singleline_check()

#线检测数据打包
def pack_linetrack_data():

    pack_data=bytearray([0xAA,0xAF,0xF3,0x00,
        singleline_check.rho_err>>8,singleline_check.rho_err,
        singleline_check.theta_err>>8,singleline_check.theta_err,
        line.flag,0x00,0X00,0X00])

    #清零线检测偏移数据和倾角数据，使得在没有检测到线时，输出为零
    singleline_check.rho_err = 0
    singleline_check.theta_err = 0

    lens = len(pack_data)#数据包大小
    pack_data[3] = lens-5;#有效数据个数

    i = 0
    sum = 0

    #和校验
    while i<(lens-1):
        sum = sum + pack_data[i]
        i = i+1
    pack_data[lens-1] = sum;

    return pack_data

#物块检测数据打包
def pack_block_data():

    pack_data=bytearray([0xAA,0xAF,0xF2,0x00,
        dot.x>>8,dot.x,
        dot.y>>8,dot.y,dot.num>>8,dot.num,
        dot.flag,0x00])

    #清零点检测偏移数据和倾角数据，使得在没有检测到点时，输出为零
    dot.x = 0
    dot.y = 0

    lens = len(pack_data)#数据包大小
    pack_data[3] = lens-5;#有效数据个数

    i = 0
    sum = 0

    #和校验
    while i<(lens-1):
        sum = sum + pack_data[i]
        i = i+1
    pack_data[lens-1] = sum;

    return pack_data

#串口数据解析 如果是线检测请发送数据：AA AF F1 01 02 4D 如果是点检测请发送数据：AA AF F1 01 01 4C
def Receive_Anl(data_buf,num):

    #和校验
    sum = 0
    i = 0
    while i<(num-1):
        sum = sum + data_buf[i]
        i = i + 1

    sum = sum%256 #求余
    if sum != data_buf[num-1]:
        return
    #和校验通过

    if data_buf[2]==0x01:
        print("receive 1 ok!")

    if data_buf[2]==0x02:
        print("receive 2 ok!")

    if data_buf[2]==0xFC:

        #设置模块工作模式
        ctrl.work_mode = data_buf[4]
        #print("Set work mode success!")


#串口通信协议接收
def Receive_Prepare(data):

    if Receive.state==0:

        if data == 0xAA:#帧头
            Receive.state = 1
            Receive.uart_buf.append(data) #将数据保存到数组里面
        else:
            Receive.state = 0

    elif Receive.state==1:
        if data == 0xAF:#帧头
            Receive.state = 2
            Receive.uart_buf.append(data) #将数据保存到数组里面
        else:
            Receive.state = 0

    elif Receive.state==2:
        if data <= 0xFF:#数据个数
            Receive.state = 3
            Receive.uart_buf.append(data) #将数据保存到数组里面
        else:
            Receive.state = 0

    elif Receive.state==3:
        if data <= 33:
            Receive.state = 4
            Receive.uart_buf.append(data) #将数据保存到数组里面
            Receive._data_len = data
            Receive._data_cnt = 0
        else:
            Receive.state = 0

    elif Receive.state==4:
        if Receive._data_len > 0:
            Receive. _data_len = Receive._data_len - 1
            Receive.uart_buf.append(data) #将数据保存到数组里面
            if Receive._data_len == 0:
                Receive.state = 5
        else:
            Receive.state = 0

    elif Receive.state==5:
        Receive.state = 0
        Receive.uart_buf.append(data) #将数据保存到数组里面
        Receive_Anl(Receive.uart_buf,Receive.uart_buf[3]+5) #还原数据个数，数据的总个数为6
        Receive.uart_buf=[]#清空缓冲区，准备下次接收数据
    else:
        Receive.state = 0

#读取串口缓存
def uart_read_buf():
    i = 0
    buf_size = uart.any() #判断是否有串口数据
    while i<buf_size:
        Receive_Prepare(uart.readchar()) #读取串口数据
        i = i + 1

#点检测函数
def check_dot(img):
    #thresholds为黑色物体颜色的阈值，是一个元组，需要用括号［ ］括起来可以根据不同的颜色阈值更改；pixels_threshold 像素个数阈值，
    #如果色块像素数量小于这个值，会被过滤掉area_threshold 面积阈值，如果色块被框起来的面积小于这个值，会被过滤掉；merge 合并，如果
    #设置为True，那么合并所有重叠的blob为一个；margin 边界，如果设置为5，那么两个blobs如果间距5一个像素点，也会被合并。
    for blob in img.find_blobs(thresholds, pixels_threshold=150, area_threshold=150, merge=True, margin=5):
        if dot.pixels<blob.pixels():#寻找最大的黑点
            ##先对图像进行分割，二值化，将在阈值内的区域变为白色，阈值外区域变为黑色
            img.binary(thresholds)
            #对图像边缘进行侵蚀，侵蚀函数erode(size, threshold=Auto)，size为kernal的大小，去除边缘相邻处多余的点。threshold用
            #来设置去除相邻点的个数，threshold数值越大，被侵蚀掉的边缘点越多，边缘旁边白色杂点少；数值越小，被侵蚀掉的边缘点越少，边缘
            #旁边的白色杂点越多。
            img.erode(2)
            dot.pixels=blob.pixels() #将像素值赋值给dot.pixels
            dot.x = blob.cx() #将识别到的物体的中心点x坐标赋值给dot.x
            dot.y = blob.cy() #将识别到的物体的中心点x坐标赋值给dot.x
            dot.ok= 1
            #在图像中画一个十字；x,y是坐标；size是两侧的尺寸；color可根据自己的喜好设置
            img.draw_cross(dot.x, dot.y, color=127, size = 10)
            #在图像中画一个圆；x,y是坐标；5是圆的半径；color可根据自己的喜好设置
            img.draw_circle(dot.x, dot.y, 5, color = 127)

    #判断标志位 赋值像素点数据
    dot.flag = dot.ok
    dot.num = dot.pixels

    #清零标志位
    dot.pixels = 0
    dot.ok = 0

    #发送数据
    uart.write(pack_block_data())

def fine_border(img,area,area_roi):
    #roi是“感兴趣区”通过设置不同的感兴趣区，可以判断线段是一条还是两条，是T型线，还是十字、还是7字线
    singleline_check.flag1 = img.get_regression([(255,255)],roi=area_roi, robust = True)
    if (singleline_check.flag1):
        area.ok=1

#找线
def found_line(img):
    #对图像所有阈值像素进行线性回归计算。这一计算通过最小二乘法进行，通常速度较快，但不能处理任何异常值。 若 robust 为True，则将
    #使用泰尔指数。泰尔指数计算图像中所有阈值像素间的所有斜率的中值。thresholds：追踪的颜色范围
    singleline_check.flag2 = img.get_regression([(255,255)], robust = True)
    if (singleline_check.flag2):
        #print(clock.fps())
        singleline_check.rho_err = abs(singleline_check.flag2.rho())-0 #求解线段偏移量的绝对值
        if singleline_check.flag2.theta()>90: #求解角度的偏移量
            singleline_check.theta_err = singleline_check.flag2.theta()-0
        else:
            singleline_check.theta_err = singleline_check.flag2.theta()-0
        #在图像中画一条直线。singleline_check.flag2.line()意思是(x0, y0)到(x1, y1)的直线；颜色可以是灰度值(0-255)，或者是彩色值
        #(r, g, b)的tupple，默认是白色
        img.draw_line(singleline_check.flag2.line(), color = 127)
        #print(singleline_check.theta_err)

def check_line(img):
    fine_border(img,up,up_roi) #上边界区域检测
    fine_border(img,down,down_roi) #下边界区域检测
    fine_border(img,left,left_roi) #左边界区域检测
    fine_border(img,righ,righ_roi) #右边界区域检测

    line.flag = 0
    if up.ok:
        line.flag = line.flag | 0x01 #将line.flag最低位置1
    if down.ok:
        line.flag = line.flag | 0x02 #将line.flag第2位置1
    if left.ok:
        line.flag = line.flag | 0x04 #将line.flag第3位置1
    if righ.ok:
        line.flag = line.flag | 0x08 #将line.flag第4位置1
    #print(line.flag)     #做测试用，在正常检测时最好屏蔽掉

    found_line(img) #线检测
    #清零标志位
    up.ok = down.ok = left.ok = righ.ok = 0
    up.num = down.num = left.num = righ.num = 0
    up.pixels = down.pixels = left.pixels = righ.pixels = 0

    #发送数据
    uart.write(pack_linetrack_data())

while(True):
    clock.tick()
    if (ctrl.work_mode&0x01)!=0:
        sensor.set_pixformat(sensor.RGB565)
        img = sensor.snapshot()
        check_dot(img)
        #LED(1).toggle()      #亮灯

    #线检测
    if (ctrl.work_mode&0x02)!=0:
        sensor.set_pixformat(sensor.GRAYSCALE)
        img = sensor.snapshot().binary([THRESHOLD])
        check_line(img)
        #LED(3).toggle()  #亮灯
    #接收串口数据
    uart_read_buf()
    fps = clock.fps()
    img.draw_string(2, 2, ("%2.1ffps" % (fps)), color=(0, 128, 0), scale=2)
    lcd.display(img)
    #print("FPS：%s"%clock.fps())#打印帧率


