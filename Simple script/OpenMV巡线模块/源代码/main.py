

import sensor
import image
import time
import math
import pyb
from pyb import Pin, Timer, UART,LED
from GeometryFeature import GeometryFeature

# 当光线弱的时候，用于补光
LED(4).on()

# 是否开启debug模式
# 如果是False就不print, 不进行可视化绘制，可以提高帧率
is_debug = True
#--------------感光芯片配置  START -------------------

DISTORTION_FACTOR = 1.5 # 设定畸变系数
IMG_WIDTH  = 64
IMG_HEIGHT = 64
def init_sensor():
    '''
    初始化感光芯片
    '''
    sensor.reset()
    sensor.set_pixformat(sensor.GRAYSCALE)
    sensor.set_framesize(sensor.B64X64)                  # 分辨率为B64X64
    sensor.skip_frames(time=2000)
    sensor.set_auto_gain(False)                         # 颜色追踪关闭自动增益
    sensor.set_auto_whitebal(False)                     # 颜色追踪关闭白平衡

init_sensor()
#--------------感光芯片配置  END -------------------



#--------------定时器部分 START -------------------

is_need_send_data = False # 是否需要发送数据的信号标志
def uart_time_trigger(timer):
    '''
    串口发送数据的定时器，定时器的回调函数
    '''
    global is_need_send_data
    is_need_send_data = True

# 初始化定时器 频率为20HZ 每秒执行20次
tim = Timer(4, freq=20)
# 设定定时器的回调函数
tim.callback(uart_time_trigger)
#--------------定时器部分 END -------------------


#--------------直线与直角检测部分 START -------------------

INTERSERCT_ANGLE_THRESHOLD = (45,90)

# 直线灰度图颜色阈值
LINE_COLOR_THRESHOLD = [(0, 120)]
# 如果直线是白色的，阈值修改为：
# LINE_COLOR_THRESHOLD = [(128, 255)]

# 取样窗口
ROIS = {
    'down': (0, 55, 64, 8), # 横向取样-下方 1
    'middle': (0, 28, 64, 8), # 横向取样-中间 2
    'up': (0, 0, 64, 8), # 横向取样-上方 3
    'left': (0, 0, 8, 64), # 纵向取样-左侧 4
    'right': (56, 0, 8, 64) # 纵向取样-右侧 5
}


BLOB_MAX_WIDTH = 15 # 色块的最大宽度
BLOB_MIN_WIDTH = 5 # 色块的最小宽度
BLOB_MAX_HEIGHT = 15 # 色块的最大高度
BLOB_MIN_HEIGHT = 5 # 色块的最小宽度


def find_blobs_in_rois(img):
    '''
    在ROIS中寻找色块，获取ROI中色块的中心区域与是否有色块的信息
    '''
    global ROIS
    global is_debug

    roi_blobs_result = {}  # 在各个ROI中寻找色块的结果记录
    for roi_direct in ROIS.keys():
        roi_blobs_result[roi_direct] = {
            'cx': -1,
            'cy': -1,
            'blob_flag': False
        }
    for roi_direct, roi in ROIS.items():
        blobs=img.find_blobs(LINE_COLOR_THRESHOLD, roi=roi, merge=True, pixels_area=10)
        if len(blobs) == 0:
            continue

        largest_blob = max(blobs, key=lambda b: b.pixels())
        x,y,width,height = largest_blob[:4]

        if not(width >= BLOB_MIN_WIDTH and width <= BLOB_MAX_WIDTH and height >= BLOB_MIN_HEIGHT and height <= BLOB_MAX_HEIGHT):
            # 根据色块的宽度进行过滤
            continue

        roi_blobs_result[roi_direct]['cx'] = largest_blob.cx()
        roi_blobs_result[roi_direct]['cy'] = largest_blob.cy()
        roi_blobs_result[roi_direct]['blob_flag'] = True

        if is_debug:
            img.draw_rectangle((x,y,width, height), color=(255))

    return roi_blobs_result

def visualize_result(canvas, cx_mean, cx, cy, is_turn_left, is_turn_right, is_t, is_cross):
    '''
    可视化结果
    '''
    if not(is_turn_left or is_turn_right or is_t or is_cross):
        mid_x = int(canvas.width()/2)
        mid_y = int(canvas.height()/2)
        # 绘制x的均值点
        canvas.draw_circle(int(cx_mean), mid_y, 5, color=(255))
        # 绘制屏幕中心点
        canvas.draw_circle(mid_x, mid_y, 8, color=(0))
        canvas.draw_line((mid_x, mid_y, int(cx_mean), mid_y), color=(255))

    turn_type = 'N' # 啥转角也不是

    if is_t or is_cross:
        # 十字形或者T形
        canvas.draw_cross(int(cx), int(cy), size=10, color=(255))
        canvas.draw_circle(int(cx), int(cy), 5, color=(255))

    if is_t:
        turn_type = 'T' # T字形状
    elif is_cross:
        turn_type = 'C' # 十字形
    elif is_turn_left:
        turn_type = 'L' # 左转
    elif is_turn_right:
        turn_type = 'R' # 右转

    canvas.draw_string(0, 0, turn_type, color=(0))




#--------------直线与直角检测部分 END -------------------


#---------------------MAIN-----------------------
last_cx = 0
last_cy = 0

while True:
    if not is_need_send_data:
        # 不需要发送数据
        continue
    is_need_send_data = False

    # 拍摄图片
    img = sensor.snapshot()
    # 去除图像畸变
    img.lens_corr(DISTORTION_FACTOR)
    # 创建画布
    # canvas = img.copy()
    # 为了IDE显示方便，直接在代码结尾 用IMG绘制

    # 注意：林林的代码里 计算直线之间的交点的代码没有用到
    lines = img.find_lines(threshold=1000, theta_margin = 50, rho_margin = 50)
    # 寻找相交的点 要求满足角度阈值
    intersect_pt = GeometryFeature.find_interserct_lines(lines, angle_threshold=(45,90), window_size=(IMG_WIDTH, IMG_HEIGHT))
    if intersect_pt is None:
        # 直线与直线之间的夹角不满足阈值范围
        intersect_x = 0
        intersect_y = 0
    else:
        intersect_x, intersect_y = intersect_pt

    reslut = find_blobs_in_rois(img)

    # 判断是否需要左转与右转
    is_turn_left = False
    is_turn_right = False


    if (not reslut['up']['blob_flag'] ) and reslut['down']['blob_flag']:
        if reslut['left']['blob_flag']:
            is_turn_left = True
        if reslut['right']['blob_flag']:
            is_turn_right = True


    # 判断是否为T形的轨道
    is_t = False
    # 判断是否十字形轨道
    is_cross = False

    cnt = 0
    for roi_direct in ['up', 'down', 'left', 'right']:
        if reslut[roi_direct]['blob_flag']:
            cnt += 1
    is_t = cnt == 3
    is_cross = cnt == 4

    # cx_mean 用于确定视角中的轨道中心
    # 用于表示左右偏移量
    cx_mean = 0
    for roi_direct in ['up', 'down', 'middle']:
        if reslut[roi_direct]['blob_flag']:
            cx_mean += reslut[roi_direct]['cx']
        else:
            cx_mean += IMG_WIDTH / 2
    cx_mean /= 3

    # cx, cy 只有在T形区域检测出来的时候才有用，
    # 用于确定轨道中圆形的大致区域， 用于定点， 是计算圆心的一种近似方法

    cx = 0
    cy = 0

    if is_cross or is_t:
        # 只在出现十字形或者T字形才计算圆心坐标
        cnt = 0
        for roi_direct in ['up', 'down']:
            if reslut[roi_direct]['blob_flag']:
                cnt += 1
                cx += reslut[roi_direct]['cx']
        if cnt == 0:
            cx = last_cx
        else:
            cx /= cnt

        cnt = 0
        for roi_direct in ['left', 'right']:
            if reslut[roi_direct]['blob_flag']:
                cnt += 1
                cy += reslut[roi_direct]['cy']
        if cnt == 0:
            cy = last_cy
        else:
            cy /= cnt


    last_cx = cx
    last_cy = cy

    if is_debug:
        visualize_result(img, cx_mean, cx, cy, is_turn_left, is_turn_right, is_t, is_cross)
