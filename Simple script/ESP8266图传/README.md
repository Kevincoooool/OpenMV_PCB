


1.程序功能：

使用esp8266给openmv增加图传功能
此代码实现的功能与 openmv IDE里的代码例子 14-wifi-shield/mjpeg_streamer 完全一样，区别是硬件使用esp8266，而不是官方的wifi shield.

2.硬件要求：

openmv3或4

esp8266 (wemos D1 Mini,其它版本应该没问题，但这个版本的引脚与openmv连接更合适)

3.硬件连接：

   GPIO    NodeMCU   Name   |   openmv
   
   ===================================
   
 GPIO16       D0      SS    |   P3  (PB12) 
 
 GPIO13       D7      MOSI  |   P0  (PB15) 
 
 GPIO12       D6      MISO  |   P1  (PB14) 
 
 GPIO14       D5      SCK   |   P2  (PB13) 
 
 GPIO2        D4 (低电压触发)  (P7) 
 
   GND                          GND 
   
<img src="https://github.com/lixy123/openmv-MJPEG-by-esp8266/blob/master/IMG_190907.jpg">

4.使用：
  
  A.spi_camera_stream3.py 改名main.py放入openmv目录,开机会自动运行
  
  B.openmv_spi_cam4.ino 文件用arduino 1.8.9软件打开  
  安装esp8266 for arduino 库。
  板选择： nodemcu 1.0(ESP32-12e Module)
  编译选择:Flash size:4M (3M SPIFFS), 其它默认
  进行编译，烧写固件.
  
  openmv与esp8266连接，openmv通过usb供电
  
  谷歌浏览器打开如下地址:
  
  http://192.168.1.41/capture 显示单张图片
  
  http://192.168.1.41/stream  显示图片流
  
5.实测效果：

  320*240 黑白 3KB  传图速度 0.1秒/张 
  
  640*480 黑白 17KB 传图速度 0.4秒/张 
  
  传图速度和openmv WiFi Shield 差不多，但偶尔会有卡顿，
  如果esp8266 不使用 ESP8266WebServer做浏览服务器，直接采用socket协议传图可能会解决卡顿问题,但需要一个树莓派或PC承担网页服务功能.
