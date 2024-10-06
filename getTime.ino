//using namespace std;
#include <AHT20.h>
#define USE_SERIAL Serial
const char* dayOfWeekName[] = {
  "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
};
#include <AHT20.h>
using std::string;
String Year;
String Mon;
String Day;
int week;
int day0;
int date0;   //日期（int格式，便于计算）
int hour0;   //时
int min0;    //分
int sec0;    //秒
int dhour0;  //时增量
int dmin0;   //分增量
int dsec0;   //秒增量
const char* thetime1;
const char* theday1="";
int time00;  //离线时钟计时1
int time01;  //离线时钟计时2
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
#include <NTPClient.h>
NTPClient timeClient(ntpUDP, "ntp.aliyun.com", 60 * 60 * 8, 60000);  //时间（参考ntp获取时间https://blog.csdn.net/veray/article/details/121940328）
#include <string>


void gettime111(int y = 80, int x = 0, int dx = 198, string mod = "small") {
  //Serial.print(".");

  timeClient.begin();
  timeClient.update();
  //Serial.println(timeClient.getFormattedTime()) ;
  String thetime = timeClient.getFormattedTime();  //时刻
  thetime = thetime.substring(0, 5);               //截取了00：00（时和分）
  Serial.println(thetime);
  thetime1 = thetime.c_str();
  Serial.println(thetime1);

  getweather(2);  //借用心知天气api里的日期
  Year = "23";
  int Y = 23;
  Serial.println("Date1:");
  Serial.println(Date1);
  if (Date1 != "2023-12-31 SAT" && thetime.substring(0, 2) != "08") {
    hour0 = atoi(thetime.substring(0, 2).c_str());  //分配时分秒到各自的变量，并将string转为char再转回int（可能有更简单的转换方法，但我没找到）
    min0 = atoi(thetime.substring(3, 5).c_str());
    sec0 = atoi(thetime.substring(6, 8).c_str());
    Mon = Date1.substring(5, 7);
    Day = Date1.substring(8, 10);
    setTime(hour0, min0, sec0, atoi(Day.c_str()), atoi(Mon.c_str()), 23);
    RTC.set(now());
  }  //10:23.30  2022.10.25
     //else {
  RTC.read(tm1);
  Serial.println("thetime1");
  string h = (hour0 < 10) ? ("0" + std::__cxx11::to_string(hour0)) : (std::__cxx11::to_string(hour0));
  string m = (min0 < 10) ? ("0" + std::__cxx11::to_string(min0)) : (std::__cxx11::to_string(min0));
  string xx = h + ":" + m;        //拼凑出离线时间
  const char* text = xx.c_str(); 
  //thetime1 = (std::__cxx11::to_string(tm.Hour) + ":" + std::__cxx11::to_string(tm.Minute)).c_str();
  if (mod == "big") {
    Serial.println("big");
    Serial.println(text);

    u8g2Fonts.setFont(u8g2_font_fub30_tf);
    centerx(text, y, 1, x, dx);
    Serial.println("done");} 
  else if(mod == "huge"){
    Serial.println("huge");
    Serial.println(text);

    u8g2Fonts.setFont(u8g2_font_logisoso62_tn);
    centerx_B(text, y, 1, x, dx);
    Serial.println("done");
  }
  else {
    Serial.println("small");
    Serial.println(text);

    u8g2Fonts.setFont(u8g2_font_helvB24_tf);
    centerx(text, y, 1, x, dx);
    Serial.println("done");
  }
  Serial.println(thetime1);RTC.read(tm1);
  //}
  string theday /*星期x*/ = "2023-" + std::__cxx11::to_string(tm1.Month) + "-" + std::__cxx11::to_string(tm1.Day) + " " + dayOfWeekName[tm1.Wday - 1];  //拼接"星期x"
  theday1 = theday.c_str();
  //day0 = atoi(theday0.c_str());

  //大字体
  u8g2Fonts.setFont(chinese_gb2312);
  if (mod == "big") {centerx(theday1, y + 20, 0, x, dx);}else if(mod=="huge"){centerx_B(theday1, y + 18, 1, x+4, dx);}else{centerx(theday1, y + 20,1, x, dx);}
  //centerx(theday1, y + 20, 1, x, dx);  //theday1见gettime里的变量
  nextPage(10, 1);
}

void gettime0() /*获取离线时间程序*/ {
  RTC.read(tm1);
  hour0 = tm1.Hour;
  min0 = tm1.Minute;
  sec0 = tm1.Second;
  Serial.print("hour0");
  Serial.print(hour0);
  Serial.print("min0");
  Serial.print(min0);
  Serial.print("sec0");
  Serial.print(sec0);
  Serial.print("--------------");
  /*通过两次记录的程序运行时间millis()差来确定时间增量，然后再加回去原来的时间，以此获得新时间*/
  /*以下是毫秒转时分秒的运算*/
  /*time01 = millis() - time00;
  dhour0 = (time01 % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60);
  dmin0 = (time01 % (1000 * 60 * 60)) / (1000 * 60);
  dsec0 = (time01 % (1000 * 60)) / 1000;
if(dsec0>=60){dsec0=dsec0%(60);dmin0+=dsec0/(60);}
if(dmin0>=60){dmin0=dmin0%(60);dhour0+=dmin0/(60);}
if(dhour0>=24){dhour0=dhour0%(24);}
  hour0 = hour0 + dhour0;
  min0 = min0 + dmin0;
  sec0 = sec0 + dsec0;
if(sec0>=60){sec0=sec0%(60);min0+=sec0/(60);}
if(min0>=60){min0=min0%(60);hour0+=min0/(60);}
if(hour0>=24){hour0=hour0%(24);week=(week+hour0/24>7)?(week+hour0/24)-7:week+hour0/24;}
*/
}
