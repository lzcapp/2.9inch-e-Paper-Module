#include <Adafruit_GFX.h> //图像库
#include <EEPROM.h>
#include <GxEPD2_BW.h>
#include <RTC_RX8025T.h>
#include <U8g2_for_Adafruit_GFX.h> //甘草修改的u8g2图像库
#include <WiFi.h>
#include <string>
#include "FS.h"
#include "SPIFFS.h"
// #include <TimeLib.h>
#include <Wire.h>
// 以上是库文件
#include "gb2312.c" //字体文件
#include "pictures.h" //图片图标文件
// #include "Get_bat_vcc.h"  //获取电池状态
// 以上是程序文件
;
// 我自己的屏幕的引脚，因人而异改
#define FORMAT_SPIFFS_IF_FAILED true
// GxEPD2_BW<GxEPD2_290_T5D, GxEPD2_290_T5D::HEIGHT> display(GxEPD2_290_T5D(/*CS=D8*/ 15, /*DC=D3*/ 27, /*RST=D4*/ 26,
// /*BUSY=D2*/ 25));  // 2.9黑白   GDEH029A1   128x296, UC8151D
extern void checkbat();
GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT>
        display(GxEPD2_290(/*CS=D8*/ 15, /*DC=D3*/ 27, /*RST=D4*/ 26,
                           /*BUSY=D2*/ 25)); // 2.9黑白   GDEH029A1   128x296, SSD1608 (IL3820)
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
// extern WiFiClient client;
// using namespace std;
int batpin = 34;
using std::string;
#define button1 16 // 右侧按钮
#define button2 12 //??中间按钮，与屏幕输入公用一个引脚，所以显示时要调成output 检测按键状态时要调成input
#define button3 0
#define FOUT_ON HIGH
#define FOUT_OFF LOW
tmElements_t tm1;
const byte frequencyPin = 2;
extern String autostart;
/*按键逻辑：左边按钮为复位键，中间时选确认择键，右边是切换键
重启/开机时按下切换键，将进入配网模式，如果已经配过， 将会自动连接，如果没有配置或要连接新的wifi,
通过连接8266热点进行wifi名称和密码输入，（断电后不会消失），设置过配网后将自动连接*/
int displaymod = 0; // 显示模式：0选择模式（home)；1天气；2微信消息；3备忘录；
int displaypage = 1; // 各个模式中给都不同页面
extern const uint8_t chinese_gb2312[253023] U8G2_FONT_SECTION("chinese_gb2312");
int tick = 0;
void showpicloop(int);

void setup() {
    SPI.end(); // release standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
    SPI.begin(13, 12, 14, 15); // map and init SPI pins SCK(13), MISO(12), MOSI(14), SS(15)
    // hspi mosi 13,miso 12,clk 14,cs 15
    Wire.begin(21, 22); // GPIO 21（SDA）GPIO 22（SCL）  Wire.begin(SDA, SCL);
    //   初始化时钟，并写入时间，如果需要把下面的注释去掉
    pinMode(frequencyPin, OUTPUT);
    digitalWrite(frequencyPin, HIGH);
    // first update should be full refresh
    RTC.init();
    //  setTime(10, 23, 30, 25, 10, 22);//10:23.30  2022.10.25
    // RTC.initFOUT(FOUT_1);
    pinMode(button3, INPUT_PULLUP);
    analogReadResolution(12);
    start();
    batpin = EEPROM.read(1110);
    Serial.println(batpin);
    EEPROM.writeString(300, "无城市");
    if (digitalRead(button3) == LOW) /*未在初始化时按左3按钮则进入电池adc引脚校准模式*/ {
        checkbat();
    }
    if (digitalRead(button2) == LOW) /*未在初始化时按右侧按钮则直接开机*/ {
        mod0setup();
    } else if (digitalRead(button2) != LOW) /*初始化时按右侧按钮进入配网*/ {
        PeiWangStart();
    }

    digitalWrite(frequencyPin, LOW);
    autostart = EEPROM.readString(800);

    Serial.println(autostart);
    if (autostart == "weatherclockon") {
        displaymod = 1;
        displaypage = 1;
        weatherclock();
    } else if (autostart == "picclockon") {
        displaymod = 1;
        displaypage = 2;
        showpicloop(2);
    } else if (autostart == "quoteclockon") {
        displaymod = 1;
        displaypage = 3;
        quoteclock();
    }
}

void loop() {
    displaymod0(); // 进入home
}
