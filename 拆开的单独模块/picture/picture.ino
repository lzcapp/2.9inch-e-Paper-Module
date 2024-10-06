
#include <WiFi.h>
//#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <U8g2_for_Adafruit_GFX.h>  //甘草修改的u8g2图像库
#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>  //图像库

#include <string>
//以上是库文件
#include "gb2312.c"  //字体文件
//以上是程序文件
;
//我自己的屏幕的引脚，因人而异改
//GxEPD2_BW<GxEPD2_260, GxEPD2_260::HEIGHT>display(GxEPD2_260(/*CS=D8*/ 15, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4));
GxEPD2_BW<GxEPD2_154, GxEPD2_154::HEIGHT> display(GxEPD2_154(/*CS=D8*/ 6, /*DC=D3*/ 1, /*RST=D4*/ 2, /*BUSY=D2*/ 3));  // 2.9黑白   GDEH029A1   128x296, SSD1608 (IL3820)

//GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=D8*/ 15, /*DC=D3*/ 27, /*RST=D4*/ 26, /*BUSY=D2*/ 25));
//GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4));  // 2.9黑白   GDEH029A1   128x296, SSD1608 (IL3820)
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
int state = 0;
#define FOUT_ON HIGH
#define FOUT_OFF LOW

const byte frequencyPin = 3;



//巴法云服务器地址默认即可
#define TCP_SERVER_ADDR "bemfa.com"
//服务器端口，tcp创客云端口8344
#define TCP_SERVER_PORT "8344"

//********************需要修改的部分*******************//
#define DEFAULT_STASSID "dht"      //WIFI名称，区分大小写，不要写错
#define DEFAULT_STAPSW "12345678"  //WIFI密码
String UID = "7eb1ac59940547489d7a779e2111a151";
String TOPIC = "pic";  //主题名字，可在控制台新建


//**************************************************//



//最大字节数
#define MAX_PACKETSIZE 10000
//设置心跳值30s
#define KEEPALIVEATIME 60 * 1000



//tcp客户端相关初始化，默认即可
WiFiClient TCPclient;
String TcpClient_Buff = "";
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0;     //心跳
unsigned long preTCPStartTick = 0;  //连接
bool preTCPConnected = false;
int tick = 0;
bool firsttime = true;
String pic = "";
//相关函数初始化
//连接WIFI
void doWiFiTick();
void startSTA();

//TCP初始化连接
void doTCPClientTick();
void startTCPClient();
void sendtoTCPServer(String p);

//led 控制函数
void turnOnLed();
void turnOffLed();

String HexToBin(char strHex) {
  if (strHex == '0') {
    String a = "0000";
    return a;
  } else if (strHex == '1') {
    String a = "0001";
    return a;
  } else if (strHex == '2') {
    String a = "0010";
    return a;
  } else if (strHex == '3') {
    String a = "0011";
    return a;
  } else if (strHex == '4') {
    String a = "0100";
    return a;
  } else if (strHex == '5') {
    String a = "0101";
    return a;
  } else if (strHex == '6') {
    String a = "0110";
    return a;
  } else if (strHex == '7') {
    String a = "0111";
    return a;
  } else if (strHex == '8') {
    String a = "1000";
    return a;
  } else if (strHex == '9') {
    String a = "1001";
    return a;
  } else if (strHex == 'a') {
    String a = "1010";
    return a;
  } else if (strHex == 'b') {
    String a = "1011";
    return a;
  } else if (strHex == 'c') {
    String a = "1100";
    return a;
  } else if (strHex == 'd') {
    String a = "1101";
    return a;
  } else if (strHex == 'e') {
    String a = "1110";
    return a;
  } else if (strHex == 'f') {
    String a = "1111";
    return a;
  }
}


void nextPage(int n = 10) /*屏幕刷新（每10次局刷进行一次全局刷新）*/ {
  //n=(theme==0)?n/2:n;
  if (tick < n) {
    display.setPartialWindow(0, 0, 296, 128);  //设置局部刷新窗口
    display.nextPage();
    tick += 1;
  } else {
    display.setFullWindow();
    display.nextPage();
    tick = 0;
  }
}


void centerx(char* text, int y, int mod = 1, int x0 = 0, int dx = 296) {  //自动居中函数
  int text_width = u8g2Fonts.getUTF8Width(text);                          //计算文本长度
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  int d = (text_width / dx) + 1;
  text_width /= d;
  for (int i = 0; i <= d - 1; i++) {
    const char* dtext = "";
    String s = text;
    s = s.substring(i * (s.length() / d), (1 + i) * (s.length() / d));
    //dtext = s.c_str();
    Serial.println(dtext);
    u8g2Fonts.setCursor(x0 + ((dx - text_width) / 2), y + 16 * i);  //文本居中
    u8g2Fonts.print(dtext);
  }
}
void centerx(String& text1, int y, int mod = 1, int x0 = 0, int dx = 296, char* MOD = "否") {

  const char* text = text1.c_str();
  int text_width = u8g2Fonts.getUTF8Width(text);  //计算文本长度
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  int d = (text_width / dx) + 1;
  text_width /= d;
  for (int i = 0; i <= d - 1; i++) {
    const char* dtext = "";
    String s = text;
    s = s.substring(i * (s.length() / d), (1 + i) * (s.length() / d));
    dtext = s.c_str();
    //Serial.println(dtext);
    u8g2Fonts.setCursor(x0 + ((dx - text_width) / 2), ((MOD == "完全居中") ? y - (d * 16 / 2) : y) + 16 * i);  //文本居中
    u8g2Fonts.print(dtext);
  }
}
////////////////////////////////////////////以下暂时不要动，似乎官方例程的，我只改了一点点，/////
/*
  *发送数据到TCP服务器
 */
void sendtoTCPServer(String p) {

  if (!TCPclient.connected()) {
    Serial.println("Client is not readly");

    return;
  }
  TCPclient.print(p);
  Serial.println(p);
  preHeartTick = millis();  //心跳计时开始，需要每隔60秒发送一次数据
}


/*
  *初始化和服务器建立连接
*/
void startTCPClient() {
  if (TCPclient.connect(TCP_SERVER_ADDR, atoi(TCP_SERVER_PORT))) {
    Serial.print("\nConnected to server:");
    Serial.printf("%s:%d\r\n", TCP_SERVER_ADDR, atoi(TCP_SERVER_PORT));
    String tcpTemp = "";                                        //初始化字符串
    tcpTemp = "cmd=1&uid=" + UID + "&topic=" + TOPIC + "\r\n";  //构建订阅指令
    sendtoTCPServer(tcpTemp);                                   //发送订阅指令
    tcpTemp = "";                                               //清空
    preTCPConnected = true;
    TCPclient.setNoDelay(true);
  } else {
    Serial.print("Failed connected to server:");
    Serial.println(TCP_SERVER_ADDR);
    TCPclient.stop();
    preTCPConnected = false;
  }
  preTCPStartTick = millis();
}


/*
  *检查数据，发送心跳
*/

void doTCPClientTick() {

  //检查是否断开，断开后重连
  if (WiFi.status() != WL_CONNECTED) return;

  if (!TCPclient.connected()) {  //断开重连

    if (preTCPConnected == true) {

      preTCPConnected = false;
      preTCPStartTick = millis();
      Serial.println();
      Serial.println("TCP Client disconnected.");
      TCPclient.stop();
    } else if (millis() - preTCPStartTick > 1 * 1000)  //重新连接
      startTCPClient();
  } else {

    if (TCPclient.available()) {  //收数据
      char c = TCPclient.read();
      TcpClient_Buff += c;
      TcpClient_BuffIndex++;
      TcpClient_preTick = millis();

      if (TcpClient_BuffIndex >= MAX_PACKETSIZE - 1) {
        TcpClient_BuffIndex = MAX_PACKETSIZE - 2;
        TcpClient_preTick = TcpClient_preTick - 200;
      }
      preHeartTick = millis();
    }
    if (millis() - preHeartTick >= KEEPALIVEATIME) {  //保持心跳

      preHeartTick = millis();
      Serial.println("--Keep alive:");
      sendtoTCPServer("cmd=0&msg=keep\r\n");
    }
  }
  if ((TcpClient_Buff.length() >= 1) && (millis() - TcpClient_preTick >= 200)) {  //data ready

    TCPclient.flush();
    //Serial.print("Rev string: ");
    TcpClient_Buff.trim();  //去掉首位空格
    //Serial.println(TcpClient_Buff);  //打印接收到的消息
    String getTopic = "";
    String getPic = "";
    String upload = "cmd=3&uid=" + UID + "&topic=" + TOPIC + "\r\n";
    if (firsttime == true) {
      sendtoTCPServer(upload);  //返回上次最后一条消息
      firsttime = false;
    }
    if (TcpClient_Buff.length() > 15) {  //注意TcpClient_Buff只是个字符串，在上面开头做了初始化 String TcpClient_Buff = "";

      //此时会收到推送的指令，指令大概为 cmd=2&uid=xxx&topic=light002&msg=off
      int topicIndex = TcpClient_Buff.indexOf("&topic=") + 7;     //c语言字符串查找，查找&topic=位置，并移动7位，不懂的可百度c语言字符串查找
      int msgIndex = TcpClient_Buff.indexOf("&msg=");             //c语言字符串查找，查找&msg=位置
      getTopic = TcpClient_Buff.substring(topicIndex, msgIndex);  //c语言字符串截取，截取到topic,不懂的可百度c语言字符串截取
      getPic = TcpClient_Buff.substring(msgIndex + 5);            //c语言字符串截取，截取到消息
      Serial.println(getPic);                                     //打印截取到的消息值
      int l = getPic.length();
      //centerx(getPic, 10);
      //String len="";len+=l;
      //centerx(len, 50);
      //nextPage();
      
      if (l == 4736) {
        if (state == 0) {
          for (int i = 0; i < 4736; i++) {
            pic += HexToBin(getPic.charAt(i));
          }
          for (int y = 0; y < 128; y++) {
            for (int x = 0; x < 296; x++) {
              if (pic.charAt(x + y * 296) == '1') {
                display.drawPixel(x, y, GxEPD_BLACK);
              } else if (pic.charAt(x + y * 296) == '0') {
                display.drawPixel(x, y, GxEPD_WHITE);
              }
            }
          }
        }else if (state == 1) {
          for (int i = 0; i < 4736; i++) {
            pic += HexToBin(getPic.charAt(i));
          }
          for (int y = 0; y < 128; y++) {
            for (int x = 0; x < 296; x++) {
              if (pic.charAt(x + y * 296) == '1') {
                display.drawPixel(x, y+64, GxEPD_BLACK);
              } else if (pic.charAt(x + y * 296) == '0') {
                display.drawPixel(x, y+64, GxEPD_WHITE);
              }
            }
          }
        }
        state++;
      }else{
              for (int i = 0; i < 9472; i++) {
            pic += HexToBin(getPic.charAt(i));
          }
          for (int y = 0; y < 128; y++) {
            for (int x = 0; x < 296; x++) {
              if (pic.charAt(x + y * 296) == '1') {
                display.drawPixel(x, y, GxEPD_BLACK);
              } else if (pic.charAt(x + y * 296) == '0') {
                display.drawPixel(x, y, GxEPD_WHITE);
              }
            }
          }
      }

    }
    Serial.println(getPic);
    Serial.println(getPic);
    Serial.println(getPic);
    Serial.println(getPic);
    nextPage();
    pic = "";
    TcpClient_Buff = "";
    TcpClient_BuffIndex = 0;
  }
}

void startSTA() {
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(DEFAULT_STASSID, DEFAULT_STAPSW);
}



/**************************************************************************
                                 WIFI
***************************************************************************/
/*
  WiFiTick
  检查是否需要初始化WiFi
  检查WiFi是否连接上，若连接成功启动TCP Client
  控制指示灯
*/
void doWiFiTick() {

  static bool startSTAFlag = false;
  static bool taskStarted = false;
  static uint32_t lastWiFiCheckTick = 0;

  /*if (!startSTAFlag) {
    startSTAFlag = true;
    startSTA();
    Serial.printf("Heap size:%d\r\n", ESP.getFreeHeap());
  }*/

  //未连接1s重连
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - lastWiFiCheckTick > 1000) {
      lastWiFiCheckTick = millis();
    }
  }
  //连接成功建立
  else {
    if (taskStarted == false) {
      taskStarted = true;
      Serial.print("\r\nGet IP Address: ");
      Serial.println(WiFi.localIP());
      startTCPClient();
    }
  }
}
// 初始化，相当于main 函数
void setup() {

  //SPI.end();                  // release standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  SPI.begin(7, 8, 8, 6);  // map and init SPI pins SCK(13), MISO(12), MOSI(14), SS(15)
  //hspi mosi 13,miso 12,clk 14,cs 15
  //Wire.begin(8, 7);  //GPIO 21（SDA）GPIO 22（SCL）  Wire.begin(SDA, SCL);
  //   初始化时钟，并写入时间，如果需要把下面的注释去掉
  pinMode(frequencyPin, OUTPUT);
  digitalWrite(frequencyPin, LOW);
  /////////////////////////////////////////////////////////////////////////////
  u8g2Fonts.setFont(chinese_gb2312);
  display.init(115200);
  u8g2Fonts.begin(display);  //将u8g2连接到display
  display.display(1);
  display.setRotation(1);
  display.firstPage();
  Serial.println("Beginning...");
  Serial.begin(115200);
  WiFi.begin(DEFAULT_STASSID, DEFAULT_STAPSW);
  while (WiFi.status() != WL_CONNECTED) {  //未连接上
    delay(500);
    Serial.println("正在连接...");
  }
  Serial.println("连接成功！");  //连接上
  display.fillScreen(GxEPD_WHITE);
  display.nextPage();
}

//循环
void loop() {
  

  doWiFiTick();
  while (state < 2) {
    doTCPClientTick();
  }
  state=0;
}
