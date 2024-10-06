#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <string>
#include <WiFi.h>

#include <stdio.h>

//#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <U8g2_for_Adafruit_GFX.h>  //甘草修改的u8g2图像库
#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>  //图像库
#include "gb2312.c"        //字体文件
//以上是程序文件
using std::string;
SPIClass SPIsd;
;
//我自己的屏幕的引脚，因人而异改
GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=D8*/ 15, /*DC=D3*/ 27, /*RST=D4*/ 26, /*BUSY=D2*/ 25));
//GxEPD2_BW<GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4));  // 2.9黑白   GDEH029A1   128x296, SSD1608 (IL3820)
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
//****** TXT阅读器相关 ******
File file;                // 本次打开的txt文件系统对象
File referfile;           // 本次打开的索引文件系统对象
String txtName[10] = {};  // 存储书本的名字
String indexesName = "";  // 创建索引文件名称，页数的位置从第二页开始记录，文件末尾倒数7个字节用来记录当前看到的页数

#define button1 16  //右侧按钮
#define button2 12  //??中间按钮，与屏幕输入公用一个引脚，所以显示时要调成output 检测按键状态时要调成input
#define button3 0
int tick = 0;
const unsigned char nkb[2048] = {
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x80,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x01,
  0xFF,
  0xFE,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x7F,
  0xF8,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x1F,
  0xF8,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x1F,
  0xF0,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x0F,
  0xE0,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x07,
  0xE0,
  0x7F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x03,
  0xC0,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x03,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x83,
  0xC0,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x03,
  0xE0,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x07,
  0xE0,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x07,
  0xF0,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x0F,
  0xF0,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x0F,
  0xFC,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x1F,
  0xFE,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x7F,
  0xFF,
  0x80,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x01,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xF0,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x0F,
  0xFF,
  0xFF,
  0xC0,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x03,
  0xFF,
  0xFF,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0xFF,
  0xFE,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0xFF,
  0xFE,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x7F,
  0xFC,
  0x07,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xE0,
  0x3F,
  0xFC,
  0x1F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xF8,
  0x1F,
  0xF8,
  0x1F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xF8,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xE1,
  0xFF,
  0xFF,
  0xC0,
  0x3F,
  0xFF,
  0xF0,
  0x07,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xE3,
  0xFF,
  0xFF,
  0xFF,
  0x1F,
  0xFF,
  0xFF,
  0xE3,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF1,
  0xFF,
  0xFF,
  0xFE,
  0x3F,
  0xFF,
  0xFF,
  0xC7,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF3,
  0xFF,
  0xFF,
  0xFF,
  0x3F,
  0xFF,
  0xFF,
  0xE7,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF1,
  0xFF,
  0xFF,
  0xFF,
  0x1F,
  0xFF,
  0xFF,
  0xE3,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF1,
  0xFF,
  0xFF,
  0xFE,
  0x3F,
  0xFF,
  0xFF,
  0xC7,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF3,
  0xFF,
  0xFF,
  0xC0,
  0x3F,
  0xFF,
  0xF0,
  0x07,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF1,
  0xFF,
  0xFF,
  0xCF,
  0xFF,
  0xFF,
  0xFF,
  0xC7,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF1,
  0xFF,
  0xFF,
  0x8F,
  0xFF,
  0xFF,
  0xFF,
  0xE3,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF1,
  0xFF,
  0xFF,
  0xCF,
  0xFF,
  0xFF,
  0xFF,
  0xE7,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF3,
  0xFF,
  0xFF,
  0xCF,
  0xFF,
  0xFF,
  0xFF,
  0xE7,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xE1,
  0xFF,
  0xFF,
  0x8F,
  0xFF,
  0xFF,
  0xFF,
  0xC3,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF1,
  0xFF,
  0xFF,
  0xC5,
  0xBF,
  0xFF,
  0xFF,
  0xC7,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xE5,
  0xFF,
  0xFF,
  0xD4,
  0xBF,
  0xFF,
  0xF8,
  0x2F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF3,
  0x1F,
  0xFF,
  0xC0,
  0x1F,
  0xFF,
  0xF0,
  0x07,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xE3,
  0x1F,
  0xFF,
  0x8F,
  0xFF,
  0xFF,
  0xF1,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF3,
  0x1F,
  0xFF,
  0xCF,
  0xFF,
  0xFF,
  0xF3,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xE3,
  0x1F,
  0xFF,
  0xCF,
  0xFF,
  0xFF,
  0xF1,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xE3,
  0x1F,
  0xFF,
  0x8F,
  0xFF,
  0xFF,
  0xF3,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xE3,
  0x1F,
  0xFF,
  0xCF,
  0xFF,
  0xFF,
  0xF1,
  0xB7,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xE3,
  0x1F,
  0xFF,
  0xC0,
  0x1F,
  0xFF,
  0xF0,
  0x47,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xE7,
  0x1F,
  0xFF,
  0xDF,
  0x1F,
  0xFF,
  0xF3,
  0xC7,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xC7,
  0x1F,
  0xFF,
  0xFF,
  0x1F,
  0xFF,
  0xF1,
  0xE7,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xE0,
  0x07,
  0xFF,
  0xFF,
  0x1F,
  0xFF,
  0xE3,
  0xC7,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xEF,
  0x1F,
  0xFF,
  0xFF,
  0x1F,
  0xFF,
  0xF3,
  0xE7,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0x1F,
  0xFF,
  0xFF,
  0x1F,
  0xFF,
  0xF1,
  0xC7,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0x1F,
  0xFF,
  0xFD,
  0x3F,
  0xFF,
  0xF3,
  0x47,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0x3F,
  0xFF,
  0xC2,
  0x7F,
  0xFF,
  0xF0,
  0xAF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xC0,
  0x1F,
  0xFF,
  0xC0,
  0x1F,
  0xFF,
  0xE0,
  0x0F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0x3F,
  0xFF,
  0xC7,
  0x1F,
  0xFF,
  0xC7,
  0x8F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFE,
  0x1F,
  0xFF,
  0xC7,
  0x9F,
  0xFF,
  0xE7,
  0x8F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0x3F,
  0xFF,
  0xCF,
  0x9F,
  0xFF,
  0xE7,
  0xCF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFE,
  0x3F,
  0xFF,
  0xC7,
  0x1F,
  0xFF,
  0xC7,
  0x8F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFE,
  0x7F,
  0xFF,
  0xC7,
  0x9F,
  0xFF,
  0xE7,
  0x8F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFC,
  0x7F,
  0xFF,
  0xC0,
  0x3F,
  0xFF,
  0xE3,
  0x8F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFC,
  0x7F,
  0xFF,
  0xC7,
  0x0F,
  0xFF,
  0xC8,
  0x0F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFC,
  0x7F,
  0xFF,
  0xCF,
  0x9F,
  0xFF,
  0xFF,
  0xCF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFC,
  0xFF,
  0xFF,
  0x8F,
  0x9F,
  0xFF,
  0xFF,
  0xCF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF8,
  0xFF,
  0xFF,
  0xCF,
  0x8F,
  0xFF,
  0xFF,
  0x8F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF8,
  0xFF,
  0xFF,
  0xC7,
  0x9F,
  0xFF,
  0xFF,
  0x8F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF9,
  0xFF,
  0xFF,
  0x8A,
  0x1F,
  0xFF,
  0xFF,
  0x1F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFA,
  0xFF,
  0xFF,
  0xD5,
  0x7F,
  0xFF,
  0xE0,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x1E,
  0x3F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xE3,
  0xFF,
  0xFF,
  0x1C,
  0x7F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xE1,
  0xFF,
  0xFF,
  0x8C,
  0x7F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xF1,
  0xFF,
  0xFF,
  0xC8,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFD,
  0xFF,
  0xFF,
  0xF8,
  0xFF,
  0xFF,
  0xC0,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF8,
  0xFF,
  0xFF,
  0xF8,
  0xFF,
  0xFF,
  0xC1,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xE0,
  0x01,
  0xFF,
  0xFC,
  0x7F,
  0xFF,
  0xE3,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xF9,
  0xFF,
  0xFF,
  0xFC,
  0x7F,
  0xFF,
  0xE1,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFC,
  0xFF,
  0xFF,
  0xF8,
  0xFF,
  0xFF,
  0xE1,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xF8,
  0xFF,
  0xFF,
  0xC0,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xF1,
  0xFF,
  0xFF,
  0x88,
  0xFF,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x3F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xE3,
  0xFF,
  0xFF,
  0x8C,
  0x7F,
  0xFF,
  0xFF,
  0xFC,
  0x1F,
  0xF8,
  0x1F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xEB,
  0xFF,
  0xFF,
  0x1C,
  0x7F,
  0xFF,
  0xFF,
  0xF8,
  0x1F,
  0xFC,
  0x1F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0x5F,
  0x7F,
  0xFF,
  0xFF,
  0xF0,
  0x3F,
  0xFC,
  0x07,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xE0,
  0x3F,
  0xFE,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x7F,
  0xFE,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x7F,
  0xFF,
  0x80,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x01,
  0xFF,
  0xFF,
  0xC0,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x03,
  0xFF,
  0xFF,
  0xF0,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x0F,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
  0xFF,
};

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
int8_t getCharLength(char zf)  //获取ascii字符的长度
{
  if (zf == 0x20) return 4;  //空格
  else if (zf == '!') return 4;
  else if (zf == '"') return 5;
  else if (zf == '#') return 5;
  else if (zf == '$') return 6;
  else if (zf == '%') return 7;
  else if (zf == '&') return 7;
  else if (zf == '\'') return 3;
  else if (zf == '(') return 5;
  else if (zf == ')') return 5;
  else if (zf == '*') return 7;
  else if (zf == '+') return 7;
  else if (zf == ',') return 3;
  else if (zf == '.') return 3;

  else if (zf == '1') return 5;
  else if (zf == ':') return 4;
  else if (zf == ';') return 4;
  else if (zf == '@') return 9;

  else if (zf == 'A') return 8;
  else if (zf == 'D') return 7;
  else if (zf == 'G') return 7;
  else if (zf == 'H') return 7;
  else if (zf == 'I') return 3;
  else if (zf == 'J') return 3;
  else if (zf == 'M') return 8;
  else if (zf == 'N') return 7;
  else if (zf == 'O') return 7;
  else if (zf == 'Q') return 7;
  else if (zf == 'T') return 7;
  else if (zf == 'U') return 7;
  else if (zf == 'V') return 7;
  else if (zf == 'W') return 11;
  else if (zf == 'X') return 7;
  else if (zf == 'Y') return 7;
  else if (zf == 'Z') return 7;

  else if (zf == '[') return 5;
  else if (zf == ']') return 5;
  else if (zf == '`') return 5;

  else if (zf == 'c') return 5;
  else if (zf == 'f') return 5;
  else if (zf == 'i') return 1;
  else if (zf == 'j') return 2;
  else if (zf == 'k') return 5;
  else if (zf == 'l') return 2;
  else if (zf == 'm') return 9;
  else if (zf == 'o') return 7;
  else if (zf == 'r') return 4;
  else if (zf == 's') return 5;
  else if (zf == 't') return 4;
  else if (zf == 'v') return 7;
  else if (zf == 'w') return 9;
  else if (zf == 'x') return 5;
  else if (zf == 'y') return 7;
  else if (zf == 'z') return 5;

  else if (zf == '{') return 5;
  else if (zf == '|') return 4;
  else if (zf == '}') return 5;

  else if ((zf >= 0 && zf <= 31) || zf == 127) return -1;  //没有实际显示功能的字符

  else return 6;
}
void espsleep() {
  gpio_wakeup_enable(GPIO_NUM_12, GPIO_INTR_HIGH_LEVEL);
  gpio_wakeup_enable(GPIO_NUM_16, GPIO_INTR_LOW_LEVEL);
  gpio_wakeup_enable(GPIO_NUM_0, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  esp_light_sleep_start();
}
String byteConversion(size_t zijie)  //字节换算
{
  String zijie_str;
  if (zijie >= 1073741824) {
    zijie_str = String(float(zijie) / 1073741824.0) + " GB";
  }
  if (zijie >= 1048576) {
    zijie_str = String(float(zijie) / 1048576.0) + " MB";
  } else if (zijie >= 1024) {
    zijie_str = String(float(zijie) / 1024.0) + " KB";
  } else zijie_str = String(zijie) + " B";
  return zijie_str;
}
void showstr(String& text1, int line, int mod = 1) {
  const char* text = text1.c_str();
  u8g2Fonts.setCursor(2, line * 16 + 13);  //文本居中
  u8g2Fonts.print(text);
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
    Serial.println(dtext);
    u8g2Fonts.setCursor(x0 + ((dx - text_width) / 2), ((MOD == "完全居中") ? y - (d * 16 / 2) : y) + 16 * i);  //文本居中
    u8g2Fonts.print(dtext);
  }
}
void centerx(const char* text1, int y, int mod = 1, int x0 = 0, int dx = 296) {  //自动居中函数
  const char* text = text1;
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
    Serial.println(dtext);
    u8g2Fonts.setCursor(x0 + ((dx - text_width) / 2), y + 16 * i);  //文本居中
    u8g2Fonts.print(dtext);
  }
}

void txtCount(const char* path) {
  File file = SD.open(path);
  string r = "/[r]" + string(path).substr(1);
  const char* rf = r.c_str();
  String txt[8 + 1] = {};  // 0-7行为一页 共8行
  int8_t line = 0;         // 当前行
  char c;                  // 中间数据
  uint16_t en_count = 0;   // 统计ascii和ascii扩展字符 1-2个字节
  uint16_t ch_count = 0;   // 统计中文等 3个字节的字符
  uint8_t line_old = 0;    //记录旧行位置
  boolean hskgState = 0;   //行首4个空格检测 0-检测过 1-未检测

  uint32_t pageCount = 1;               // 页数计数
  boolean line0_state = 1;              // 每页页首记录状态位
  uint32_t yswz_count = 0;              // 待写入文件统计
  String yswz_str = "";                 // 待写入的文件
  uint32_t txtTotalSize = file.size();  //记录该TXT文件的大小，插入到索引的倒数14-8位
  uint32_t timestart = millis();
  display.firstPage();
  if (!file) {
    centerx("文件打开失败", 64);
    nextPage();
    return;
  }
  while (file.available()) {
    if (line_old != line)  //行首4个空格检测状态重置
    {
      line_old = line;
      hskgState = 1;
    }

    if (line0_state == 1 && line == 0 && pageCount > 1) {
      line0_state = 0;
      uint32_t yswz_uint32 = file.position();  //获取当前位置 yswz=页数位置
      //页数位置编码处理
      if (yswz_uint32 >= 1000000) yswz_str += String(yswz_uint32);
      else if (yswz_uint32 >= 100000) yswz_str += "0" + String(yswz_uint32);
      else if (yswz_uint32 >= 10000) yswz_str += "00" + String(yswz_uint32);
      else if (yswz_uint32 >= 1000) yswz_str += "000" + String(yswz_uint32);
      else if (yswz_uint32 >= 100) yswz_str += "0000" + String(yswz_uint32);
      else if (yswz_uint32 >= 10) yswz_str += "00000" + String(yswz_uint32);
      else yswz_str += "000000" + String(yswz_uint32);
      yswz_count++;
      if (yswz_count == 1000)  //每500页控制屏幕显示一下当前进度
      {
        File referfile = SD.open(rf, FILE_APPEND);
        referfile.print(yswz_str);  //将待写入的缓存 写入索引文件中
        referfile.close();

        yswz_str = "";   // 待写入文件清空
        yswz_count = 0;  // 待写入计数清空

        //计算剩余量,进度条
        uint32_t shengyu_int = txtTotalSize - file.available();
        float shengyu_float = (float(shengyu_int) / float(txtTotalSize)) * 100.0;
        display.firstPage();
        centerx(String(shengyu_float) + "%", 34);
        nextPage();
        //Serial.println("写入索引文件");
      }
      //Serial.print("第"); Serial.print(pageCount); Serial.print("页，页首位置："); Serial.println(yswz_uint32);
    }

    c = file.read();                // 读取一个字节
    while (c == '\n' && line <= 7)  // 检查换行符,并将多个连续空白的换行合并成一个
    {
      // 检测到首行并且为空白则不需要插入换行
      if (line == 0)  //等于首行，并且首行不为空，才插入换行
      {
        if (txt[line].length() > 0) line++;  //换行
        else txt[line].clear();
      } else  //非首行的换行检测
      {
        //连续空白的换行合并成一个
        if (txt[line].length() > 0) line++;
        else if (txt[line].length() == 0 && txt[line - 1].length() > 0) line++;
      }
      if (line <= 7) c = file.read();
      en_count = 0;
      ch_count = 0;
    }
    if (c == '\t')  //检查水平制表符 tab
    {
      if (txt[line].length() == 0) txt[line] += "    ";  //行首的一个水平制表符 替换成4个空格
      else txt[line] += "       ";                       //非行首的一个水平制表符 替换成7个空格
    } else if ((c >= 0 && c <= 31) || c == 127)          //检查没有实际显示功能的字符
    {
    } else txt[line] += c;

    //检查字符的格式 + 数据处理 + 长度计算
    boolean asciiState = 0;
    byte a = B11100000;
    byte b = c & a;

    if (b == B11100000)  //中文等 3个字节
    {
      ch_count++;
      c = file.read();
      txt[line] += c;
      c = file.read();
      txt[line] += c;
    } else if (b == B11000000)  //ascii扩展 2个字节
    {
      en_count += 14;
      c = file.read();
      txt[line] += c;
    } else if (c == '\t')  //水平制表符，代替两个中文位置，14*2
    {
      if (txt[line] == "    ") en_count += 20;  //行首，因为后面会检测4个空格再加8所以这里是20
      else en_count += 28;                      //非行首
    } else if (c >= 0 && c <= 255) {
      en_count += getCharLength(c) + 1;  //getCharLength=获取ascii字符的像素长度
      asciiState = 1;
    }
    uint16_t StringLength = en_count + (ch_count * 14);  //一个中文14个像素长度

    if (StringLength >= 260 && hskgState)  //检测到行首的4个空格预计的长度再加长一点
    {
      if (txt[line][0] == ' ' && txt[line][1] == ' ' && txt[line][2] == ' ' && txt[line][3] == ' ') {
        en_count += 8;
      }
      hskgState = 0;
    }
    if (StringLength >= 283)  //283个像素检查是否已填满屏幕 ，填满一行
    {
      if (asciiState == 0) {
        line++;
        en_count = 0;
        ch_count = 0;
      } else if (StringLength >= 286) {
        char t = file.read();
        file.seek(-1, SeekCur);  //往回移
        int8_t cz = 294 - StringLength;
        int8_t t_length = getCharLength(t);
        byte a = B11100000;
        byte b = t & a;
        if (b == B11100000 || b == B11000000)  //中文 ascii扩展
        {
          line++;
          en_count = 0;
          ch_count = 0;
        } else if (t_length > cz) {
          line++;
          en_count = 0;
          ch_count = 0;
        }
      }
    }
    if (line == 8) {
      line0_state = 1;
      pageCount++;
      line = 0;
      en_count = 0;
      ch_count = 0;
      for (uint8_t i = 0; i < 9; i++) txt[i].clear();
    }
  }

  //剩余的字节写入索引文件，并在末尾加入文件大小校验位14-8 页数记录位7-1
  uint32_t size_uint32 = txtTotalSize;  //获取当前TXT文件的大小
  String size_str = "";
  //TXT文件大小编码处理
  if (size_uint32 >= 1000000) size_str += String(size_uint32);
  else if (size_uint32 >= 100000) size_str += String("0") + String(size_uint32);
  else if (size_uint32 >= 10000) size_str += String("00") + String(size_uint32);
  else if (size_uint32 >= 1000) size_str += String("000") + String(size_uint32);
  else if (size_uint32 >= 100) size_str += String("0000") + String(size_uint32);
  else if (size_uint32 >= 10) size_str += String("00000") + String(size_uint32);
  else size_str += String("000000") + String(size_uint32);

  if (yswz_count != 0)  //还有剩余页数就在末尾加入 剩余的页数+文件大小位+当前位置位（初始0）
  {
    referfile = SD.open(rf, FILE_APPEND);
    referfile.print(yswz_str + size_str + "0000000");
  } else  //没有剩余页数了就在末尾加入文件大小位+当前位置位
  {
    referfile = SD.open(rf, FILE_APPEND);
    referfile.print(size_str + "0000000");
  }
  referfile.close();
  referfile = SD.open(rf);
  uint32_t indexes_size = referfile.size();
  referfile.close();
  Serial.print("索引文件大小：");
  Serial.println(indexes_size);
  Serial.print("索引文件理论大小：");
  Serial.println(7 * ((pageCount - 1) + 1 + 1));

  Serial.print("yswz_count：");
  Serial.println(yswz_count);
  Serial.print("pageCount：");
  Serial.println(pageCount);

  // 校验索引是否正确建立
  // 算法：一页为7个字节（从第二页开始记录所以要总页数-1），加上文件大小位7个字节，加上当前页数位7个字节
  // 所以为：7*((总页数-1)+1+1))

  if (indexes_size == 7 * ((pageCount - 1) + 1 + 1)) {
    Serial.println("校验通过，索引文件有效");
    centerx("100.00%", 34);
    centerx("校验通过，索引文件有效", 54);
    nextPage();
  } else {
    Serial.println("校验失败，索引文件无效，请重新创建");
    centerx("索引文件创建失败，校验失败或空间不足", 54);
    nextPage();
    //SD.remove(rf);  //删除无效的索引
  }

  centerx("索引文件大小：" + byteConversion(indexes_size), 94);
  nextPage();
  referfile.close();

  yswz_str = "";
  yswz_count = 0;

  file.close();
  file = SD.open(path);

  uint32_t timeend = millis() - timestart;
  Serial.print("计算完毕：");
  Serial.print(pageCount);
  Serial.println("页");
  Serial.print("耗时：");
  Serial.print(timeend);
  Serial.println("毫秒");
  centerx("耗时：" + String(float(timeend) / 1000.0) + "秒", 74);
  nextPage();
  delay(500);
  line = 0;
  en_count = 0;
  ch_count = 0;
  for (uint8_t i = 0; i < 9; i++) txt[i].clear();
}
void markpage(const char* path, int Page) {
  string r = "/[r]" + string(path).substr(1);
  const char* rf = r.c_str();
  uint32_t yswz_uint32 = Page;
  String yswz_str = "";
  if (yswz_uint32 >= 1000000) yswz_str += String(yswz_uint32);
  else if (yswz_uint32 >= 100000) yswz_str += String("0") + String(yswz_uint32);
  else if (yswz_uint32 >= 10000) yswz_str += String("00") + String(yswz_uint32);
  else if (yswz_uint32 >= 1000) yswz_str += String("000") + String(yswz_uint32);
  else if (yswz_uint32 >= 100) yswz_str += String("0000") + String(yswz_uint32);
  else if (yswz_uint32 >= 10) yswz_str += String("00000") + String(yswz_uint32);
  else yswz_str += String("000000") + String(yswz_uint32);

  referfile = SD.open(rf, "r+");  //打开索引文件，可读可写
  referfile.seek(-7, SeekEnd);    //从末尾开始偏移7位
  referfile.print(yswz_str);      //写入数据
  //referfile.close();
}
void displaytxt(const char* path, int map) {
  file = SD.open(path);
  file.seek(map, SeekSet);

  String txt[8 + 1] = {};  // 0-7行为一页 共8行
  int8_t line = 0;         // 当前行
  char c;                  // 中间数据
  uint16_t en_count = 0;   // 统计ascii和ascii扩展字符 1-2个字节
  uint16_t ch_count = 0;   // 统计中文等 3个字节的字符
  uint8_t line_old = 0;    //记录旧行位置
  boolean hskgState = 0;   //行首4个空格检测 0-检测过 1-未检测

  uint32_t pageCount = 1;               // 页数计数
  boolean line0_state = 1;              // 每页页首记录状态位
  uint32_t yswz_count = 0;              // 待写入文件统计
  String yswz_str = "";                 // 待写入的文件
  uint32_t txtTotalSize = file.size();  //记录该TXT文件的大小，插入到索引的倒数14-8位
  uint32_t timestart = millis();
  display.firstPage();
  if (!file) {
    centerx("文件打开失败", 64);
    nextPage();
    return;
  }
  while (file.available()) {
    if (line_old != line)  //行首4个空格检测状态重置
    {
      line_old = line;
      hskgState = 1;
    }

    c = file.read();                // 读取一个字节
    while (c == '\n' && line <= 7)  // 检查换行符,并将多个连续空白的换行合并成一个
    {
      // 检测到首行并且为空白则不需要插入换行
      if (line == 0)  //等于首行，并且首行不为空，才插入换行
      {
        if (txt[line].length() > 0) line++;  //换行
        else txt[line].clear();
      } else  //非首行的换行检测
      {
        //连续空白的换行合并成一个
        if (txt[line].length() > 0) line++;
        else if (txt[line].length() == 0 && txt[line - 1].length() > 0) line++;
      }
      if (line <= 7) c = file.read();
      en_count = 0;
      ch_count = 0;
    }
    if (c == '\t')  //检查水平制表符 tab
    {
      if (txt[line].length() == 0) txt[line] += "    ";  //行首的一个水平制表符 替换成4个空格
      else txt[line] += "       ";                       //非行首的一个水平制表符 替换成7个空格
    } else if ((c >= 0 && c <= 31) || c == 127)          //检查没有实际显示功能的字符
    {
    } else txt[line] += c;

    //检查字符的格式 + 数据处理 + 长度计算
    boolean asciiState = 0;
    byte a = B11100000;
    byte b = c & a;

    if (b == B11100000)  //中文等 3个字节
    {
      ch_count++;
      c = file.read();
      txt[line] += c;
      c = file.read();
      txt[line] += c;
    } else if (b == B11000000)  //ascii扩展 2个字节
    {
      en_count += 14;
      c = file.read();
      txt[line] += c;
    } else if (c == '\t')  //水平制表符，代替两个中文位置，14*2
    {
      if (txt[line] == "    ") en_count += 20;  //行首，因为后面会检测4个空格再加8所以这里是20
      else en_count += 28;                      //非行首
    } else if (c >= 0 && c <= 255) {
      en_count += getCharLength(c) + 1;  //getCharLength=获取ascii字符的像素长度
      asciiState = 1;
    }
    uint16_t StringLength = en_count + (ch_count * 14);  //一个中文14个像素长度

    if (StringLength >= 260 && hskgState)  //检测到行首的4个空格预计的长度再加长一点
    {
      if (txt[line][0] == ' ' && txt[line][1] == ' ' && txt[line][2] == ' ' && txt[line][3] == ' ') {
        en_count += 8;
      }
      hskgState = 0;
    }
    if (StringLength >= 283)  //283个像素检查是否已填满屏幕 ，填满一行
    {
      if (asciiState == 0) {
        line++;
        en_count = 0;
        ch_count = 0;
      } else if (StringLength >= 286) {
        char t = file.read();
        file.seek(-1, SeekCur);  //往回移
        int8_t cz = 294 - StringLength;
        int8_t t_length = getCharLength(t);
        byte a = B11100000;
        byte b = t & a;
        if (b == B11100000 || b == B11000000)  //中文 ascii扩展
        {
          line++;
          en_count = 0;
          ch_count = 0;
        } else if (t_length > cz) {
          line++;
          en_count = 0;
          ch_count = 0;
        }
      }
    }
    if (line == 8) {
      line0_state = 1;
      pageCount++;
      line = 0;
      en_count = 0;
      ch_count = 0;
      break;
    }
  }

  for (int i = 0; i < 8; i++) {
    showstr(txt[i], i);
  }

  nextPage();
}
int pagetopoint(File& referfile) {

  String point_str = "";
  for (int j = 0; j < 7; j++) {
    char c = referfile.read();
    point_str += c;
  }
  int point_int = atoi(point_str.c_str());  // 转换成int格式
  return point_int;
}
////////////////////////////////
void nkbl(int& i, int& j) {
  i -= 1;
  if (i == -1) {
    i = 2;
    j -= 1;
  }
  if (j == -1) {
    j = 3;
  }
}
void nkbr(int& i, int& j) {
  i += 1;
  if (i == 3) {
    i = 0;
    j += 1;
  }
  if (j == 4) {
    j = 0;
  }
}
void nkbd(int& i, int& j) {
  j += 1;
  if (j == 4) j = 0;
}
int nkeyb() {  //键盘
  display.fillRect(168, 128,128,128, GxEPD_WHITE);
  String textb = "";
  display.drawInvertedBitmap(168, 128, nkb, 128, 128, GxEPD_BLACK);
  
  String say = "";
  String keyvalue = "1234567890";
  int i = 0;
  int j = 0;
  int num = 0;
  int len = 0;
  display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, GxEPD_BLACK);
  nextPage(50);
  while (1) {
    display.fillRect(168 + 9, 9, 110, 20, GxEPD_WHITE);
    centerx(textb, 20, 1,168, 128);
    espsleep();
    pinMode(button2, INPUT_PULLDOWN);
    pinMode(button3, INPUT_PULLUP);
    if (digitalRead(button2) == HIGH && digitalRead(button1) == HIGH) {
      pinMode(button2, OUTPUT);
      //pinMode(button3, OUTPUT);
      display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, GxEPD_WHITE);
      nkbl(i, j);
      display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, GxEPD_BLACK);
      nextPage(50);
    }
    if (digitalRead(button1) == LOW && digitalRead(button2) == LOW) {
      pinMode(button2, OUTPUT);
      //(button3, OUTPUT);
      display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, GxEPD_WHITE);
      nkbr(i, j);
      display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, GxEPD_BLACK);
      nextPage(50);
    }
    if (digitalRead(button2) == HIGH && digitalRead(button1) == LOW) {
      pinMode(button2, OUTPUT);
      //pinMode(button3, OUTPUT);
      display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, GxEPD_WHITE);
      nkbd(i, j);
      display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, GxEPD_BLACK);
      nextPage(50);
    }
    num = i + 3 * j;
    //if (digitalRead(button2) == LOW && digitalRead(button1) == HIGH && digitalRead(button3) == HIGH) {
    //delay(20);
    if (/*digitalRead(button2) == LOW && digitalRead(button1) == HIGH && */ digitalRead(button3) == LOW && num < 9) {
      textb += keyvalue[num];
      display.fillRect(168 + 9, 9, 110, 20, GxEPD_WHITE);
      centerx(textb, 20, 1, 168, 128);
      nextPage(50);
    } else if (/*digitalRead(button2) == LOW && digitalRead(button1) == HIGH && */ digitalRead(button3) == LOW && num == 10) {
      display.fillScreen(GxEPD_WHITE);
      centerx("键入完成", 64);
      nextPage();
      delay(50);
      int N = atoi(textb.c_str());
      return N;
      break;
    } else if (/*digitalRead(button2) == LOW && digitalRead(button1) == HIGH && */ digitalRead(button3) == LOW && num == 9) {
      textb = textb.substring(0, textb.length() - 1);
      display.fillRect(168 + 9, 9, 110, 20, GxEPD_WHITE);
      centerx(textb, 20, 1, 168, 128);
      nextPage(50);
    } else if (digitalRead(button3) == LOW && num == 11) {
      return -3;
      break;
    }
    //}
  }
}
////////////////////////////////

int txtdesk(int Page, int pagetotal) {
  display.fillRoundRect(48, 2, 200, 64, 10, GxEPD_WHITE);
  display.drawRoundRect(48, 2, 200, 64, 10, GxEPD_BLACK);
  String hadread = "第" + String(Page + 1) + "/" + pagetotal + "页";
  centerx(hadread, 20, 1, 48, 110);
  centerx("继续阅读", 35, 1, 53, 30);
  centerx("跳转页面", 35, 1, 53 + 40, 30);
  centerx("退出阅读", 35, 1, 53 + 40 * 2, 30);
  display.fillRoundRect(53, 52, 34, 5, 2, GxEPD_BLACK);
  int txtmode = 0;
  nextPage();
  String no = "";
  while (1) {
    espsleep();
    if (digitalRead(button1) == LOW) {
      txtmode = (txtmode == 2) ? 0 : txtmode + 1;
      display.fillRoundRect(53 + 40 * ((txtmode == 0) ? 2 : (txtmode - 1)), 52, 34, 5, 2, GxEPD_WHITE);  //画外框
      display.fillRoundRect(53 + 40 * txtmode, 52, 34, 5, 2, GxEPD_BLACK);
      nextPage();
    }
    if (digitalRead(button2) == HIGH) {
      if (txtmode == 0) return -1;
      if (txtmode == 1) return nkeyb();
      if (txtmode == 2) return -2;
    }
  }
}
void Readtxt(const char* path) {
  //file = SD.open(path);
  display.firstPage();
  string r = "/[r]" + string(path).substr(1);
  const char* rf = r.c_str();
  if (!SD.exists(rf)) {
    Serial.println("Failed to open rfile for reading");
    centerx("首次打开将创建索引，请等待", 64);
    nextPage();
    txtCount(path);
    //return;
  } else {
    referfile = SD.open(rf, "r");
    uint32_t txtsize = file.size();
    uint32_t rfsize = referfile.size();
    Serial.println("pagetotal");
    int pagetotal = (rfsize / 7) - 2;
    Serial.println((rfsize / 7) - 2);
    Serial.println("=========");
    uint32_t txtmarksize = 0;
    String txtmarksizestr = "";
    while (file.available()) {
      referfile.seek(-14, SeekEnd);    // 从末尾开始偏移14位
      for (uint8_t i = 0; i < 7; i++)  // 获取索引末14-8位，记录txt文件大小用
      {
        char c = referfile.read();
        txtmarksizestr += c;
      }

      txtmarksize = atoi(txtmarksizestr.c_str());  // 转换成int格式
      Serial.println("文件理论大小");
      Serial.println(txtmarksize);
      Serial.println(txtmarksizestr);
      break;
    }
    referfile.close();

    Serial.println("文件实际大小");
    Serial.println(txtsize);
    if (txtmarksize == txtsize) {
      centerx("索引正确", 64);
      nextPage();
      referfile = SD.open(rf, "r+");
      String point_str = "";
      int Page = 0;
      int point_int = 0;
      String pagemark = "";
      referfile.seek(-7, SeekEnd);     //从末尾开始偏移7位
      for (uint8_t i = 0; i < 7; i++)  // 获取索引末14-8位，记录txt文件大小用
      {
        char c = referfile.read();
        pagemark += c;
      }
      Serial.print("pageMark");

      Page = atoi(pagemark.c_str());
      Serial.println(Page);
      referfile.seek(Page * 7, SeekSet);
      if (Page == 0) {
        displaytxt(path, 0);
        markpage(path, Page);
      } else {
        point_int = pagetopoint(referfile);  // 转换成int格式
        displaytxt(path, point_int);
        markpage(path, Page);
      }
      //referfile.close();
      while (1) {

        espsleep();
        if (digitalRead(button1) == LOW) {
          if (Page < pagetotal - 1) {
            Page++;
            Serial.print(Page);
            referfile.seek(Page * 7, SeekSet);
            if (Page == 0) {
              displaytxt(path, 0);
              markpage(path, Page);
            } else {
              point_int = pagetopoint(referfile);  // 转换成int格式
              Serial.print("referfilepoint");
              Serial.println(point_int);
              displaytxt(path, point_int);
              markpage(path, Page);
            }
          }
        }
        if (digitalRead(button2) == HIGH) {
          if (Page != 0) {

            Page--;
            Serial.print(Page);
            referfile.seek(Page * 7, SeekSet);
            if (Page == 0) {
              displaytxt(path, 0);
              markpage(path, Page);
            } else {
              point_int = pagetopoint(referfile);  // 转换成int格式
              Serial.print("referfilepoint");
              Serial.println(point_int);
              displaytxt(path, point_int);
              markpage(path, Page);
            }
          }
        }
        if (digitalRead(button3) == LOW) {
          int a = txtdesk(Page, pagetotal);
          switch (a) {
            case -3:
              txtdesk(Page, pagetotal);
              break;
            case -2:
              referfile.close();
              file.close();
              display.firstPage();
              centerx("退出阅读", 64);
              nextPage();
              return;
            case -1:
            displaytxt(path, point_int);
              break;
              
            default:
              Page = (pagetotal <= a - 1) ? pagetotal : a - 1;
              referfile.seek(Page * 7, SeekSet);
              if (Page == 0) {
                displaytxt(path, 0);
                markpage(path, Page);
              } else {
                point_int = pagetopoint(referfile);  // 转换成int格式
                Serial.print("referfilepoint");
                Serial.println(point_int);
                displaytxt(path, point_int);
                markpage(path, Page);
              }

              break;
          }
        }


        point_str = "";
      }





      return;
    } else {
      Serial.println("校验失败，索引文件无效，请重新创建");
      centerx("索引文件不匹配，将重新创建", 54);
      nextPage();
      SD.remove(rf);  //删除无效的索引
      txtCount(path);
      return;
    }
  }
  if (!file) {
    Serial.println("Failed to open file for reading");
    centerx("文件打开失败", 64);
    return;
  }
}
void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
void txtmod(const char* dirname, uint8_t levels) {
  for (int i = 0; i < 10; i++) txtName[i] = "";
  display.firstPage();
  File root = SD.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  int i = 0;
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(SD, file.path(), levels - 1);
      }
    } else {
      if (i < 5) {
        string fname(file.name());
        txtName[i] = String(file.name());
        if (String(file.name()).substring(0, 3) != "[r]") {
          string f = fname + "   " + std::__cxx11::to_string(((file.size() / 1024) > 1023) ? (file.size() / (1024 * 1024)) : (file.size() / 1024)) + (((file.size() / 1024) > 1023) ? "MB" : "KB");
          const char* ff = f.c_str();
          centerx(ff, 30 + i * 20);
          i++;
          Serial.print("  FILE: ");
          Serial.print(file.name());
          Serial.print("  SIZE: ");
          Serial.println(file.size());
        }
      }
    }
    file = root.openNextFile();
  }
  root.close();
  //nextPage();
  display.drawRoundRect(48, 18, 200, 18, 20, GxEPD_BLACK);  //画外框
  nextPage();
  int txtn = 0;
  while (1) {
    espsleep();
    if (digitalRead(button1) == LOW) {
      txtn = (txtn == 4) ? 0 : txtn + 1;
      Serial.println(txtn);
      display.drawRoundRect(48, 18 + 20 * ((txtn == 0) ? 4 : (txtn - 1)), 200, 18, 20, GxEPD_WHITE);  //画外框
      display.drawRoundRect(48, 18 + 20 * txtn, 200, 18, 20, GxEPD_BLACK);                            //画外框
      nextPage();
    }
    if (digitalRead(button2) == HIGH) {

      Readtxt(("/" + txtName[txtn]).c_str());
      break;
    }
  }
}


void setup() {
  SPIsd.begin(18, 19, 23, 5);
  SPI.end();                  // release standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  SPI.begin(13, 12, 14, 15);  // map and init SPI pins SCK(13), MISO(12), MOSI(14), SS(15)
  //hspi mosi 13,miso 12,clk 14,cs 15
  Wire.begin(21, 22);  //GPIO 21（SDA）GPIO 22（SCL）  Wire.begin(SDA, SCL);
  Serial.begin(115200);
  u8g2Fonts.setFont(chinese_gb2312);
  display.init(115200);
  u8g2Fonts.begin(display);  //将u8g2连接到display
  display.display(1);
  display.setRotation(1);
  display.firstPage();
  Serial.println("Beginning...");
  Serial.begin(115200);

  /////////////////////////////////////////////////////////////////////////////
  if (!SD.begin(SS, SPIsd)) {
    Serial.println("Card Mount Failed");
    centerx("SD卡接入失败", 64);
    nextPage();
    return;
  } else {
    centerx("SD卡接入成功", 54);
    nextPage();
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");

    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  int cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %dMB\n", cardSize);
  string sdsize = "SD卡大小" + std::__cxx11::to_string(cardSize) + "MB";
  const char* s = sdsize.c_str();

  centerx(s, 74);
  nextPage();
  delay(1000);
  display.fillScreen(GxEPD_WHITE);
}
void loop() {
  txtmod("/", 0);
  delay(500);
}