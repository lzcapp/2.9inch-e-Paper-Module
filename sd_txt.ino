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
//以上是程序文件
using std::string;


//****** TXT阅读器相关 ******
File txtfile;             // 本次打开的txt文件系统对象
File referfile;           // 本次打开的索引文件系统对象
string txtName[50] = {};  // 存储书本的名字
string txtSize[50] = {};  // 存储书本的大小
String indexesName = "";  // 创建索引文件名称，页数的位置从第二页开始记录，文件末尾倒数7个字节用来记录当前看到的页数

#define button1 16  //右侧按钮
#define button2 12  //??中间按钮，与屏幕输入公用一个引脚，所以显示时要调成output 检测按键状态时要调成input
#define button3 0


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
void txtCount(const char* path) {
  File txtfile = SD.open(path);
  string r = "/[r]" + string(path).substr(1);
  const char* rf = r.c_str();
  String txt[8 + 1] = {};  // 0-7行为一页 共8行
  int8_t line = 0;         // 当前行
  char c;                  // 中间数据
  uint16_t en_count = 0;   // 统计ascii和ascii扩展字符 1-2个字节
  uint16_t ch_count = 0;   // 统计中文等 3个字节的字符
  uint8_t line_old = 0;    //记录旧行位置
  boolean hskgState = 0;   //行首4个空格检测 0-检测过 1-未检测

  uint32_t pageCount = 1;                  // 页数计数
  boolean line0_state = 1;                 // 每页页首记录状态位
  uint32_t yswz_count = 0;                 // 待写入文件统计
  String yswz_str = "";                    // 待写入的文件
  uint32_t txtTotalSize = txtfile.size();  //记录该TXT文件的大小，插入到索引的倒数14-8位
  uint32_t timestart = millis();
  display.firstPage();
  if (!txtfile) {
    centerx("文件打开失败", 64);
    nextPage(10, 1);
    return;
  }
  while (txtfile.available()) {
    if (line_old != line)  //行首4个空格检测状态重置
    {
      line_old = line;
      hskgState = 1;
    }

    if (line0_state == 1 && line == 0 && pageCount > 1) {
      line0_state = 0;
      uint32_t yswz_uint32 = txtfile.position();  //获取当前位置 yswz=页数位置
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
        uint32_t shengyu_int = txtTotalSize - txtfile.available();
        float shengyu_float = (float(shengyu_int) / float(txtTotalSize)) * 100.0;
        display.firstPage();
        centerx(String(shengyu_float) + "%", 34);
        nextPage(10, 1);
        //Serial.println("写入索引文件");
      }
      //Serial.print("第"); Serial.print(pageCount); Serial.print("页，页首位置："); Serial.println(yswz_uint32);
    }

    c = txtfile.read();             // 读取一个字节
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
      if (line <= 7) c = txtfile.read();
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
      c = txtfile.read();
      txt[line] += c;
      c = txtfile.read();
      txt[line] += c;
    } else if (b == B11000000)  //ascii扩展 2个字节
    {
      en_count += 14;
      c = txtfile.read();
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
        char t = txtfile.read();
        txtfile.seek(-1, SeekCur);  //往回移
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
    nextPage(10, 1);
  } else {
    Serial.println("校验失败，索引文件无效，请重新创建");
    centerx("索引文件创建失败，校验失败或空间不足", 54);
    nextPage(10, 1);
    //SD.remove(rf);  //删除无效的索引
  }

  centerx("索引文件大小：" + byteConversion(indexes_size), 94);
  nextPage(10, 1);
  referfile.close();

  yswz_str = "";
  yswz_count = 0;

  txtfile.close();
  txtfile = SD.open(path);

  uint32_t timeend = millis() - timestart;
  Serial.print("计算完毕：");
  Serial.print(pageCount);
  Serial.println("页");
  Serial.print("耗时：");
  Serial.print(timeend);
  Serial.println("毫秒");
  centerx("耗时：" + String(float(timeend) / 1000.0) + "秒", 74);
  nextPage(10, 1);
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
  txtfile = SD.open(path);
  txtfile.seek(map, SeekSet);

  String txt[8 + 1] = {};  // 0-7行为一页 共8行
  int8_t line = 0;         // 当前行
  char c;                  // 中间数据
  uint16_t en_count = 0;   // 统计ascii和ascii扩展字符 1-2个字节
  uint16_t ch_count = 0;   // 统计中文等 3个字节的字符
  uint8_t line_old = 0;    //记录旧行位置
  boolean hskgState = 0;   //行首4个空格检测 0-检测过 1-未检测

  uint32_t pageCount = 1;                  // 页数计数
  boolean line0_state = 1;                 // 每页页首记录状态位
  uint32_t yswz_count = 0;                 // 待写入文件统计
  String yswz_str = "";                    // 待写入的文件
  uint32_t txtTotalSize = txtfile.size();  //记录该TXT文件的大小，插入到索引的倒数14-8位
  uint32_t timestart = millis();
  display.fillScreen(BackC);
  if (!txtfile) {
    centerx("文件打开失败", 64);
    nextPage(10, 1);
    return;
  }
  while (txtfile.available()) {
    if (line_old != line)  //行首4个空格检测状态重置
    {
      line_old = line;
      hskgState = 1;
    }

    c = txtfile.read();             // 读取一个字节
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
      if (line <= 7) c = txtfile.read();
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
      c = txtfile.read();
      txt[line] += c;
      c = txtfile.read();
      txt[line] += c;
    } else if (b == B11000000)  //ascii扩展 2个字节
    {
      en_count += 14;
      c = txtfile.read();
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
        char t = txtfile.read();
        txtfile.seek(-1, SeekCur);  //往回移
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

  nextPage(10, 1);
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

  String textb = "";


  String say = "";
  String keyvalue = "1234567890";
  int i = 0;
  int j = 0;
  int num = 0;
  int len = 0;
  display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, FormC);
  nextPage(50, 1);
  while (1) {
    display.fillRect(168 + 9, 9, 110, 20, BackC);
    centerx(textb, 20, 1, 168, 128);
    espsleep();
    pinMode(button2, INPUT_PULLDOWN);
    pinMode(button3, INPUT_PULLUP);
    if (digitalRead(button2) == HIGH && digitalRead(button1) == HIGH) {
      pinMode(button2, OUTPUT);
      //pinMode(button3, OUTPUT);
      display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, BackC);
      nkbr(i, j);
      display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, FormC);
      nextPage(50, 1);
    }
    if (digitalRead(button1) == LOW && digitalRead(button2) == LOW) {
      pinMode(button2, OUTPUT);
      //(button3, OUTPUT);
      display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, BackC);
      nkbl(i, j);
      display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, FormC);
      nextPage(50, 1);
    }
    if (digitalRead(button2) == HIGH && digitalRead(button1) == LOW) {
      pinMode(button2, OUTPUT);
      //pinMode(button3, OUTPUT);
      display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, BackC);
      nkbd(i, j);
      display.drawRoundRect(168 + 30 + 25 * i, 46 + 18 * j, 17, 16, 2, FormC);
      nextPage(50, 1);
    }
    num = i + 3 * j;
    //if (digitalRead(button2) == LOW && digitalRead(button1) == HIGH && digitalRead(button3) == HIGH) {
    //delay(20);
    if (/*digitalRead(button2) == LOW && digitalRead(button1) == HIGH && */ digitalRead(button3) == LOW && num < 10) {
      textb += keyvalue[num];
      display.fillRect(168 + 9, 9, 110, 20, BackC);
      centerx(textb, 20, 1, 168, 128);
      nextPage(50, 1);
    } else if (/*digitalRead(button2) == LOW && digitalRead(button1) == HIGH && */ digitalRead(button3) == LOW && num == 11) {
      display.fillScreen(BackC);
      centerx("键入完成", 64);
      nextPage(10, 1);
      delay(50);
      int N = atoi(textb.c_str());
      return N;
      break;
    } else if (/*digitalRead(button2) == LOW && digitalRead(button1) == HIGH && */ digitalRead(button3) == LOW && num == 10) {
      textb = textb.substring(0, textb.length() - 1);
      display.fillRect(168 + 9, 9, 110, 20, BackC);
      centerx(textb, 20, 1, 168, 128);
      nextPage(50, 1);
    }
    //}
  }
}
////////////////////////////////
int txtdesk(int Page, int pagetotal) {
  display.fillRoundRect(48, 2, 200, 64, 10, BackC);
  display.drawRoundRect(48, 2, 200, 64, 10, FormC);
  String hadread = "第" + String(Page + 1) + "/" + pagetotal + "页";
  centerx(hadread, 20, 1, 48, 110);
  centerx("继续阅读", 35, 1, 53, 30);
  centerx("跳转页面", 35, 1, 53 + 40, 30);
  centerx("退出阅读", 35, 1, 53 + 40 * 2, 30);
  display.fillRoundRect(53, 52, 34, 5, 2, FormC);
  time1(33, 155);
  int txtmode = 0;
  nextPage(10, 1);
  String no = "";
  while (1) {
    espsleep();
    if (digitalRead(button1) == LOW) {
      txtmode = (txtmode == 2) ? 0 : txtmode + 1;
      display.fillRoundRect(53 + 40 * ((txtmode == 0) ? 2 : (txtmode - 1)), 52, 34, 5, 2, BackC);  //画外框
      display.fillRoundRect(53 + 40 * txtmode, 52, 34, 5, 2, FormC);
      nextPage(10, 1);
    }
    if (digitalRead(button2) == HIGH) {
      if (txtmode == 0) return -1;
      if (txtmode == 1) {
        display.fillRoundRect(168, 0, 128, 128, 10, BackC);
        display.drawInvertedBitmap(168, 0, nkb, 128, 128, FormC);
        nextPage(50, 1);
        int a = nkeyb();
        return (a == 0) ? 1 : a;
      }
      if (txtmode == 2) return -2;
    }
  }
}
void Readtxt(const char* path) {
  //txtfile = SD.open(path);
  display.fillScreen(BackC);
  string r = "/[r]" + string(path).substr(1);
  const char* rf = r.c_str();
  if (!SD.exists(rf)) {
    Serial.println("Failed to open rfile for reading");
    centerx("首次打开将创建索引，请等待", 64);
    nextPage(10, 1);
    txtCount(path);
    //return;
  } else {
    referfile = SD.open(rf, "r");
    uint32_t txtsize = txtfile.size();
    uint32_t rfsize = referfile.size();
    Serial.println("pagetotal");
    int pagetotal = (rfsize / 7) - 2;
    Serial.println((rfsize / 7) - 2);
    Serial.println("=========");
    uint32_t txtmarksize = 0;
    String txtmarksizestr = "";
    while (txtfile.available()) {
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
      nextPage(10, 1);
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
        if (digitalRead(button2) != LOW) {
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
        if (digitalRead(button1) != HIGH) {
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
              txtfile.close();
              display.firstPage();
              display.fillScreen(BackC);

              centerx("退出阅读", 64);
              nextPage(10, 1);
              return;
            case -1:
              displaytxt(path, point_int);
              break;

            default:
              Page = (pagetotal <= a - 1) ? pagetotal - 1 : a - 1;
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
      nextPage(10, 1);
      SD.remove(rf);  //删除无效的索引
      txtCount(path);
      return;
    }
  }
  if (!txtfile) {
    Serial.println("Failed to open txtfile for reading");
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

  File txtfile = root.openNextFile();
  while (txtfile) {
    if (txtfile.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(txtfile.name());
      if (levels) {
        listDir(fs, txtfile.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(txtfile.name());
      Serial.print("  SIZE: ");
      Serial.println(txtfile.size());
    }
    txtfile = root.openNextFile();
  }
}
void txtmod(const char* dirname, uint8_t levels) {
  for (int i = 0; i < 50; i++) txtName[i] = "";
  display.fillScreen(BackC);
  File root = SD.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  //File txtfile = root.openNextFile();
  int i = 0;
  //File root = SD.open(dirname);
  File txtfile = root.openNextFile();
  while (txtfile) {
    if (txtfile.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(txtfile.name());
      if (levels) {
        listDir(SD, txtfile.path(), levels - 1);
      }
    } else {
      if (i < 50) {
        string fname(txtfile.name());
        if (String(txtfile.name()).substring(0, 3) != "[r]") {
          txtName[i] = txtfile.name();
          txtSize[i] = std::__cxx11::to_string(((txtfile.size() / 1024) > 1023) ? (txtfile.size() / (1024 * 1024)) : (txtfile.size() / 1024)) + (((txtfile.size() / 1024) > 1023) ? "MB" : "KB");
          i++;
        }
      }
    }
    txtfile = root.openNextFile();
  }
  root.close();

  int m = 0;
  int n = 0;
  display.drawRoundRect(48, 18, 200, 18, 20, FormC);  //画外框
  while (1) {

    (m > 1) ? m : 1;
    for (int i = 0; i < 4; i++) {
      string fname = txtName[5 * m + i];
      string fsize = txtSize[5 * m + i];
      string f = fname + "   " + fsize;
      const char* ff = f.c_str();
      centerx(ff, 30 + i * 20);
    }

    String p = "第" + String(m + 1) + "页" + "  返回";
    centerx(p, 30 + 4 * 20);
    nextPage(10, 1);
    n = 0;
    bool left = 0;
    while (1) {
      espsleep();
      if (digitalRead(button1) == LOW && digitalRead(button3) == HIGH) {
        n = (n == 4) ? 0 : n + 1;
        display.drawRoundRect(48, 18 + 20 * ((n == 0) ? 4 : (n - 1)), 200, 18, 20, BackC);  //画外框
        display.drawRoundRect(48, 18 + 20 * n, 200, 18, 20, FormC);                         //画外框
        nextPage(10, 1);
      }
      if (digitalRead(button2) == HIGH) {
        if (n < 4) {
          Readtxt(("/" + txtName[5 * m + n]).c_str());
          display.firstPage();
          display.drawRoundRect(48, 18, 200, 18, 20, FormC);
          break;
        } else if (n == 4) {
          left = 1;
          break;
        }
      }

      if (digitalRead(button3) == LOW) {
        delay(200);
        if (digitalRead(button3) == LOW) {
          m += 1;
          display.fillScreen(BackC);
          display.drawRoundRect(48, 18, 200, 18, 20, FormC);
          break;  //画外框  break;
        } else {
          m = (m == 0) ? m : m - 1;
          display.fillScreen(BackC);
          display.drawRoundRect(48, 18, 200, 18, 20, FormC);
          break;  //画外框break;
        }
      }
    }
    if (left == 1) break;
  }
  mod0setup();
}
