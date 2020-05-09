
class GeometryFeature:

    def __init__(self, img):
        self.img = img
        
    @staticmethod
    def trans_line_format(line):
        '''
        将原来由两点坐标确定的直线，转换为 y = ax + b 的格式
        '''
        x1 = line.x1()
        y1 = line.y1()
        x2 = line.x2()
        y2 = line.y2()

        if x1 == x2:
            # 避免完全垂直，x坐标相等的情况
            x1 += 0.1
        # 计算斜率 a
        a = (y2 - y1) / (x2 - x1)
        # 计算常数项 b
        # y = a*x + b -> b = y - a*x
        b = y1 - a * x1
        return a,b

    @staticmethod    
    def calculate_angle(line1, line2):
        '''
        利用四边形的角公式， 计算出直线夹角
        '''
        angle  = (180 - abs(line1.theta() - line2.theta()))
        if angle > 90:
            angle = 180 - angle
        return angle

    @staticmethod
    def find_verticle_lines(lines, angle_threshold=(70, 90)):
        '''
        寻找相互垂直的两条线
        '''
        return GeometryFeature.find_interserct_lines(lines, angle_threshold=angle_threshold)
    
    @staticmethod
    def find_interserct_lines(lines, angle_threshold=(10,90), window_size=None):
        '''
        根据夹角阈值寻找两个相互交叉的直线， 且交点需要存在于画面中
        '''
        line_num = len(lines)
        for i in range(line_num -1):
            for j in range(i, line_num):
                # 判断两个直线之间的夹角是否为直角
                angle = GeometryFeature.calculate_angle(lines[i], lines[j])
                # 判断角度是否在阈值范围内
                if not(angle >= angle_threshold[0] and angle <=  angle_threshold[1]):
                    continue
                
                # 判断交点是否在画面内
                if window_size is not None:
                    # 获取串口的尺寸 宽度跟高度
                    win_width, win_height = window_size
                    # 获取直线交点
                    intersect_pt = GeometryFeature.calculate_intersection(lines[i], lines[j])
                    if intersect_pt is None:
                        # 没有交点
                        continue
                    x, y = intersect_pt
                    if not(x >= 0 and x < win_width and y >= 0 and y < win_height):
                        # 交点如果没有在画面中
                        continue
                return (lines[i], lines[j])
        return None

    @staticmethod
    def calculate_intersection(line1, line2):
        '''
        计算两条线的交点
        '''
        a1 = line1.y2() - line1.y1()
        b1 = line1.x1() - line1.x2()
        c1 = line1.x2()*line1.y1() - line1.x1()*line1.y2()

        a2 = line2.y2() - line2.y1()
        b2 = line2.x1() - line2.x2()
        c2 = line2.x2() * line2.y1() - line2.x1()*line2.y2()

        if (a1 * b2 - a2 * b1) != 0 and (a2 * b1 - a1 * b2) != 0:
            cross_x = int((b1*c2-b2*c1)/(a1*b2-a2*b1))
            cross_y = int((c1*a2-c2*a1)/(a1*b2-a2*b1))
            return (cross_x, cross_y)
        return None