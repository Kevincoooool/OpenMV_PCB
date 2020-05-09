# MAVLink OpticalFlow Script.
#
# This script sends out OpticalFlow detections using the MAVLink protocol to
# an LIGHT/PixHawk UAV controller for position control using your OpenMV Cam.
#
# 发送端口P4 = TXD 115200,8,N,1


import sensor, image, time, pyb, struct, math

sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.B64x64)
clock = time.clock() # Tracks FPS.

old = sensor.snapshot()

uart = pyb.UART(3, 115200, timeout_char = 1000)

#定义发送数据的函数
def send_optical_flow_packet(x, y, c):
    temp = struct.pack("<bbiii", #bbiii b是C语言的Unisgned char i是C语言的int
                       0xAA,
                       0xAE,
                       int(x * 100000 ),
                       int(y * 100000 ),
                       int(c * 100000))
    uart.write(temp)


while(True):
    clock.tick() # 获取时间
    img = sensor.snapshot() # 获取一帧图像
    [delta_x, delta_y, response] = old.find_displacement(img) #获取前面一张图像与刚捕获的图像之间的偏移
    old = img.copy() #保存用于下次比较
    if (not (math.isnan(delta_x) or math.isnan(delta_y) or math.isnan(response))):
        send_optical_flow_packet(delta_x, delta_y, response)
