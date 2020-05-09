#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "FS.h"

//https://github.com/dmainmon/ArduCAM-mini-ESP8266-12E-Camera-Server/blob/master/ArduCam_ESP8266_FileCapture.ino

/*
    GPIO    NodeMCU   Name  |   openmv
   ===================================
 GPIO16       D0      SS   |   P3  (PB12) 26
 GPIO13       D7      MOSI  |   P0  (PB15) 23
 GPIO12       D6      MISO  |   P1  (PB14) 24
 GPIO14       D5      SCK   |   P2  (PB13) 25
 GPIO2        D4 指令信号(低触发)  (P7) 
   GND                          GND 22

 本版本改成适合openmv4的引脚，同时ip变成了1.41

 编译设置：Flash size:4M (3M SPIFFS)
*/

int spi_ss=16;   //D0
int spi_snap=2 ; //D4

File file;
bool filewriteok = false;
char *   recordfile = "/openmv.jpg";
ESP8266WebServer server(80);

uint8_t file_buff[1024 * 30]; //用于缓存JPG文件  30k足够，40k易重启  
int32_t file_buff_len = 0;
boolean work_flag = false;
int32_t jpg_len = 0;

/*
  esp8266做webserver 不支持多线程并发
  esp8266兼网页服务器, 每隔5-10秒会卡1-2秒，原因不明,如果esp8266只实现socket传图，可能效果会更好
  带可外接天线的esp8266 用不了，原因不明，可能芯片不一样，
  天线没有增加稳定性
  1.因为spi传输数据非常快，如果传过来实时wifi传输，会造成数据丢失.
  2.如果暂时存在SPIFFS,速度也跟不上,所以最终用内存缓存，待数据收集完成再统一wifi发送
  速度 0.4秒/张  17kb
  http://192.168.1.41/capture
  http://192.168.1.41/stream
*/



void readData(byte * data, int32_t len)
{
  for (int32_t i = 0; i < len; i++) {
    yield();  //防止超时重启
    data[i] = SPI.transfer(0);
    if ( i % 400 == 0 && i > 0)
    {
      Serial.println(i);
    }
    //Serial.print(char(data[i]));
  }
}

int32_t rece_spi()
{
  //int32_t rece_num = 0;
  byte val = 0;
  jpg_len=0;
  digitalWrite(spi_ss, LOW);
  delay(1); // Give the OpenMV Cam some time to setup to send data.
  //非阻塞式
  val = SPI.transfer(1);
  if ( val == 85) { // saw sync char?
    SPI.transfer(&jpg_len, 4); // get stream length
    //Serial.print();
    Serial.println(String("len=") + String(jpg_len));
    if (jpg_len)
      SPI.transfer(&file_buff, jpg_len);
    Serial.println("transfer ok");
    file_buff[jpg_len] = 0;
  }
  digitalWrite(spi_ss, HIGH);
  delay(1); // Don't loop to quickly.
  return jpg_len;
}

//读取图像命令
bool do_cmd(WiFiClient client, String cmd)
{

  if (work_flag == true)
    return true;

  work_flag = true;

  unsigned long starttime = 0;
  unsigned long stoptime = 0;
  bool sendok = false;
  uint32_t read_len = 0;

  //触发一下抓图
  digitalWrite(spi_snap, LOW);
  delay(50);
  digitalWrite(spi_snap, HIGH);


  starttime = millis() / 1000;
  file_buff_len = 0;

  //2秒内收不到85信息，则放弃
  while (true)
  {
    yield();
    stoptime = millis() / 1000;
    //2秒内无数据，则认为数据读完
    if (stoptime - starttime > 2)
    {
      Serial.println("timeout");
      break;
    }
    file_buff_len = rece_spi();
    if (file_buff_len > 0)
      break;

    delay(100);
     Serial.print(".");
  }


  //检查文件读入是否正确
  if (file_buff_len > 0 && client.connected()  )
  {

    client.write(file_buff , file_buff_len);

    if (filewriteok == false)
    {
      file = SPIFFS.open(recordfile, "w");
      file.write(file_buff , file_buff_len);
      file.close();
      filewriteok = true;
    }
    sendok = true;

     Serial.println("sendok");
  }


  //如果没成功获取图片文件，发送每一次有效的文件
  //如果不这样处理的话，IE显示图片会中断！
  if (client.connected() && sendok == false && filewriteok)
  {
    file = SPIFFS.open(recordfile, "r");
    while (file.available() > 0) {
      read_len = file.read(file_buff, 512);
      if (read_len > 0)
      {
        client.write(file_buff, read_len);
      }
    }
    file.close();

    Serial.print("read from SPIFFS");
  }

  work_flag = false;

    Serial.print("return success");
  return sendok;
}


void connectwifi()
{
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.disconnect();
  delay(100);

  WiFi.config(IPAddress(192, 168, 4, 1), //设置静态IP位址
              IPAddress(192, 168, 1, 1),
              IPAddress(255, 255, 255, 0),
              IPAddress(192, 168, 1, 1)
             );
  /*
      WiFi.config(IPAddress(10, 1, 199, 200), //设置静态IP位址
                 IPAddress(10, 1, 199, 1),
                 IPAddress(255, 255, 255, 0),
                 IPAddress(10, 1, 199, 1)
                );

    WiFi.config(IPAddress(192, 168, 2, 40), //设置静态IP位址
                IPAddress(192, 168, 2, 1),
                IPAddress(255, 255, 255, 0),
                IPAddress(192, 168, 2, 1)
               );
  */
  WiFi.mode(WIFI_STA);

  WiFi.begin("KKKK", "88888888");
  //WiFi.begin("tao", "9999900000");
  while ((!(WiFi.status() == WL_CONNECTED))) {
    delay(1000);
    // Serial.print("..");
  }
}



/////////////////////////////////////////////
// 返回单张图片
/////////////////////////////////////////////
void serverCapture() {
  WiFiClient client = server.client();
  do_cmd(client, "");
}

/////////////////////////////
// 返回图片流 
/////////////////////////////
void serverStream() {
  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame2019\r\n\r\n";
  server.sendContent(response);
  while (client.connected()) {
    //sendContent 原理：
    //https://www.xuebuyuan.com/zh-tw/1462571.html
    response = "--frame2019\r\n";
    response += "Content-Type: image/jpeg\r\n\r\n";
    server.sendContent(response);
    do_cmd(client, "");
    //传图时,每隔5-10秒会停顿1-2秒，原因不明，加delay不管用
    // delay(50);
  }
}

void handleNotFound() {
  String ipStr = WiFi.localIP().toString();
  String message = "<html><head>\n";
  message += "</head><body>\n";
  message += "<form >";
  message += "<h1>ESP8266-12E Arducam Mini</h1>\n";
  message += "<div><h2>Video Streaming</h2></div> \n";
  message += "<div><a  href=\"http://" + ipStr + "/capture\"  > capture </a></div>\n";
  message += "<div><a  href=\"http://" + ipStr + "/stream\"  > stream </a></div>\n";


  message += "</form> \n";
  message += "</body></html>\n";
  server.send(200, "text/html", message);
}


void setup() {

  pinMode(spi_snap, OUTPUT);  
  digitalWrite(spi_snap, HIGH);

  Serial.begin(115200);
  pinMode(spi_ss, OUTPUT);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.setDataMode(SPI_MODE0);


  connectwifi();
  SPIFFS.begin();
  filewriteok = false;
  if (SPIFFS.exists(recordfile))
  {
    SPIFFS.remove(recordfile);
    //Serial.println(String(recordfile) + " remove");
  }

  server.on("/capture", HTTP_GET, serverCapture);
  server.on("/stream", HTTP_GET, serverStream);

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}
