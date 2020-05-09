
import sensor, image, time
from pyb import Pin, SPI
import pyb


####################################MOSI:  P0 (PB15)
####################################MISO:  P1 (PB14)
####################################SCK :  P2 (PB13)
####################################IRQ :  P7 (PD12)
####################################CSN :  P8 (PD13)
####################################CE  :  P9 (PD14)




#####################################[NRF寄存器配置及接收发送函数]#######################################
#####################################[NRF寄存器配置及接收发送函数]#######################################
#####################################[NRF寄存器配置及接收发送函数]#######################################




ERROR = 0


TX_ADR_WIDTH 	=  5        #发射地址宽度
TX_PLOAD_WIDTH  =  32       #发射数据通道有效数据宽度0~32Byte
RX_ADR_WIDTH    =  5        #接收地址宽度
RX_PLOAD_WIDTH  =  32       #接收数据通道有效数据宽度0~32Byte



CHANAL          =  41                         #频道选择
TX_ADDRESS = [0x34,0x43,0x10,0x10,0x01]       #定义一个静态发送地址
RX_ADDRESS = [0x34,0x43,0x10,0x10,0x01]       #定义一个静态接收地址



# commands
NRF_READ_REG  = const(0x00)
NRF_WRITE_REG = const(0x20)
RD_RX_PLOAD   = const(0x61)
WR_TX_PLOAD   = const(0xA0)
FLUSH_TX      = const(0xE1)
FLUSH_RX      = const(0xE2)
REUSE_TX_PL   = const(0xE3)
NOP           = const(0xFF)


# register
CONFIG        = const(0x00)
EN_AA         = const(0x01)
EN_RXADDR     = const(0x02)
SETUP_AW      = const(0x03)
SETUP_RETR    = const(0x04)
RF_CH         = const(0x05)
RF_SETUP      = const(0x06)
STATUS        = const(0x07)
OBSERVE_TX    = const(0x08)
CD            = const(0x09)
RX_ADDR_P0    = const(0x0A)
RX_ADDR_P1    = const(0x0B)
RX_ADDR_P2    = const(0x0C)
RX_ADDR_P3    = const(0x0D)
RX_ADDR_P4    = const(0x0E)
RX_ADDR_P5    = const(0x0F)
TX_ADDR       = const(0x10)
RX_PW_P0      = const(0x11)
RX_PW_P1      = const(0x12)
RX_PW_P2      = const(0x13)
RX_PW_P3      = const(0x14)
RX_PW_P4      = const(0x15)
RX_PW_P5      = const(0x16)
FIFO_STATUS   = const(0x17)



# important
TX_FULL       = const(0x01)
MAX_RT        = const(0x10)
TX_DS         = const(0x20)
RX_DR         = const(0x40)



irq =  Pin('P7', Pin.IN, Pin.PULL_UP)
CSN =  Pin('P8', Pin.OUT_PP, Pin.PULL_DOWN)
CE  =  Pin('P9', Pin.OUT_PP, Pin.PULL_DOWN)


buf_R=[1,1,1,1,1]




def NRF_CE_HIGH():
    CE.high()

def NRF_CE_LOW():
    CE.low()

def NRF_CSN_HIGH():
    CSN.high()

def NRF_CSN_LOW():
    CSN.low()


def nrf_writereg(reg,dat):
    NRF_CSN_LOW()                      #置低CSN是能SPI传输
    NRF_CE_LOW()
    status=1
    status=spi.send_recv(reg)          #发送寄存器
    spi.send(dat)                      #向寄存器写入数据
    NRF_CSN_HIGH()                     #CSN拉高，完成
    a=bytearray(status)
    return a[0]                        #返回状态寄存器的值


def nrf_readreg(reg):
    NRF_CE_LOW()
    NRF_CSN_LOW()                      #置低CSN是能SPI传输
    spi.send(reg)                      #发送寄存器
    reg_val=1
    reg_val=spi.send_recv(NOP)         #读取寄存器的值
    NRF_CSN_HIGH()                     #CSN拉高，完成
    a=bytearray(reg_val)
    return a[0]                        #返回值

def nrf_writebuf(reg,buf,num):
    i=0
    NRF_CE_LOW()
    NRF_CSN_LOW()                      #置低CSN是能SPI传输
    status=1
    status=spi.send_recv(reg)          #发送寄存器
    for i in range(0,num):
        spi.send(buf[i])
    NRF_CSN_HIGH()                     #CSN拉高，完成
    a=bytearray(status)
    return a[0]                        #返回状态寄存器的值


def nrf_readbuf(reg,buf,num):
    i=0
    NRF_CE_LOW()
    NRF_CSN_LOW()                      #置低CSN是能SPI传输
    status=1
    status=spi.send_recv(reg)          #发送寄存器
    for i in range(0,num):
        b=spi.send_recv(NOP)
        a=bytearray(b)
        buf[i]=a[0]
    NRF_CSN_HIGH()                     #CSN拉高，完成
    a=bytearray(status)
    return a[0]                        #返回状态寄存器的值




def NRF_link_Check():
    check_num=0
    check_add=0
    retun_flag=0
    buf1=[0XC2,0XC2,0XC2,0XC2,0XC2]
    reg1 = NRF_WRITE_REG+TX_ADDR
    nrf_writebuf(reg1,buf1,5)
    nrf_readbuf(TX_ADDR,buf_R,5)
    for check_num in range(0,5):
        if buf_R[check_num]==0xc2:
            check_add=check_add+1
    if check_add==5:
        retun_flag=1
    else:
        retun_flag=0
    return retun_flag

def NRF_TX_Mode():                                                       #配置进入发送模式
    NRF_CE_LOW()
    nrf_writebuf(NRF_WRITE_REG|TX_ADDR,TX_ADDRESS,TX_ADR_WIDTH)
    nrf_writebuf(NRF_WRITE_REG|RX_ADDR_P0,RX_ADDRESS,RX_ADR_WIDTH)
    nrf_writereg(NRF_WRITE_REG|EN_AA,0x01)                               #使能通道0的自动应答
    nrf_writereg(NRF_WRITE_REG|EN_RXADDR,0x01)                           #使能通道0的接收地址
    nrf_writereg(NRF_WRITE_REG|SETUP_RETR,0x0a)                          #设置自动重发间隔时间：500us+86us； 最大重发次数：10次
    nrf_writereg(NRF_WRITE_REG|RF_CH,CHANAL)                             #设置RF通道为CHANAL
    nrf_writereg(NRF_WRITE_REG|RF_SETUP,0x0f)                            #设置TX发射参数，0db增益，2Mbps，低噪声增益开启
    nrf_writereg(NRF_WRITE_REG|CONFIG,0x0e)                              #配置基本工作模式的参数：PWR_UP,EN_CRC,16BIT_CRC,发射模式，开启所有中断
    NRF_CE_HIGH()                                                        #CE拉高，进入发送模式
    i=0XFF
    while i:
        i=i-1


def NRF_Tx_Dat(buf):                                                     #用于向nrf的发送缓冲区写入数据
    NRF_CE_LOW()
    nrf_writebuf(WR_TX_PLOAD,buf,TX_PLOAD_WIDTH)
    NRF_CE_HIGH()
    i=irq.value()
    while i==1:
        i=irq.value()
    state = nrf_readreg(STATUS)
    nrf_writereg(NRF_WRITE_REG|STATUS,state)
    nrf_writereg(FLUSH_TX,NOP)
    m=state*MAX_RT
    n=state*TX_DS
    #print(state)
    if   m!=1:
        return_num = MAX_RT
    elif n!=1:
        return_num = TX_DS
    else:
        return_num = ERROR                                               #发送失败

    return return_num


def NRF_RX_Mode():
    NRF_CE_LOW()
    nrf_writebuf(NRF_WRITE_REG+RX_ADDR_P0,RX_ADDRESS,RX_ADR_WIDTH)       #写RX节点地址
    nrf_writereg(NRF_WRITE_REG+EN_AA,0x01)                               #使能通道0自动应答
    nrf_writereg(NRF_WRITE_REG+EN_RXADDR,0x01)                           #使能通道0接收地址
    nrf_writereg(NRF_WRITE_REG+RF_CH,CHANAL)                             #设置RF通信频率
    nrf_writereg(NRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH)                  #选择通道0有效数据宽度
    nrf_writereg(NRF_WRITE_REG+RF_SETUP,0x0f)                            #设置TX发射参数，0db增益，2Mbps，低噪声增益开启
    nrf_writereg(NRF_WRITE_REG+CONFIG, 0x0f)                             #配置基本工作模式的参数：PWR_UP,EN_CRC,16BIT_CRC,发射模式，开启所有中断
    NRF_CE_HIGH()                                                        #CE拉高，进入接收模式


def NRF_Rx_Dat(buf):                                                     #用于从nrf的接收缓冲区读入数据
    NRF_CE_HIGH()                                                        #进入接收状态
    i=irq.value()
    while i==1:
        i=irq.value()
    NRF_CE_LOW()                                                         #进入待机状态
    state = nrf_readreg(STATUS)                                          #读取state寄存器的值
    nrf_writereg(NRF_WRITE_REG+STATUS,state)                             #清除中断标志位
    judge_date=state&RX_DR
    if judge_date!=0:
        nrf_readbuf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH)                    #读取数据
        nrf_writereg(FLUSH_RX,NOP)                                       #清除RX FIFO寄存器
        return RX_DR
    else:
        return 0


def num_right(num):                                                      #返回16位数的低8位
    a=num & 0x00ff
    return a


def num_left(num):                                                       #返回16位数的高8位
    b=num >> 8
    return b

def num_left_add_right(left_num,right_num):                              #合并高8位与低8位
    c=left_num<<8|right_num
    return c





#####################################[NRF寄存器配置及接收发送函数]#######################################
#####################################[NRF寄存器配置及接收发送函数]#######################################
#####################################[NRF寄存器配置及接收发送函数]#######################################








sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QQCIF)   # Set frame size to QVGA (320x240)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.

spi = SPI(2, SPI.MASTER, baudrate=int(12500*1000), polarity=0, phase=0)
NRF_CSN_HIGH()
status =NRF_link_Check()            #返回nrf是否连接正常
NRF_TX_Mode()                       #发送模式


buf_test=[12,8,0,1,8,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,6,6,2]

while(True):

    #clock.tick()                    # Update the FPS clock.
    img = sensor.snapshot()         # Take a picture and return the image.
    #print(clock.fps())             # Note: OpenMV Cam runs about half as fast when connected




    NRF_Tx_Dat(buf_test)                 #发送一个包（包内的有效数据宽度0~32可调，这里用的是32byte）






