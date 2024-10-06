#include <string>
#include <WiFi.h>
#include <string>
#include <ArduinoJson.h>
using std::string;

//extern GxEPD2_BW<GxEPD2_290_T94, GxEPD2_290_T94::HEIGHT> display;
const char* host = "api.seniverse.com";  // 将要连接的心知天气服务器地址
const int httpPort = 80;                 // 将要连接的服务器端口
const char* hostmsg = "1.12.222.56";     // 将要连接的服务器地址（微信信息中转）
const char* host2 = "dht.dht3218.xyz";   // 将要连接的服务器地址（备忘录，未完成）
const int httpPort1 = 81;                // 将要连接的服务器端口
const int httprmd = 444;                 // 将要连接的服务器端口
int results_0_now_temperature_int1 = 999;
int results_0_now_code_int1 = 999;
//extern const unsigned char zhihu[], yiyanp[], jitang[],toutiao[],baidu[],douban[],weizhi_[],guancha[],wendu[],shidu[],weibo[],qing[],duoyun[], yin[],zhenyu[],leizhenyu[],xiaoyu[],zhongyu[],dayu[],mai[],baoyu[],dongyu[],yujiaxue[],zhenxue[],xiaoxue[],zhongxue[],daxue[],baoxue[],fuchen[],shachenbao[],dawu[],weizhi[],ling[],yi[],er[],san[],si[],wu[],liu[],qi[],ba[],jiu[];
WiFiClient client;
String Date1;  //日期（String格式，便于拼接字符）

// 心知天气HTTP请求所需信息
String reqUserKey = "";            // 私钥，自己申请呗，很简单的,申请到了就填进去
String reqLocation = "Guangzhou";  // 城市
String reqUnit = "c";              // 摄氏/华氏


using std::string;




//连接云服务器，参考：https://blog.csdn.net/weixin_46182244/article/details/119565456
void getmsg() /*从云服务器获取消息，可能并不能进行正确的http请求，每次访问到都是404页面，只能出此下策测：将我的消息数据编写成404页面的源码*/ {
  WiFiClient client;

  // 建立http请求信息
  String httpRequest = String("GET /") + "HTTP/1.1\r\n" + "Host:" + hostmsg + "\r\n" + "Connection:close\r\n\r\n";
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.print(hostmsg);

  // 尝试连接服务器
  if (!client.connect(hostmsg, httpPort1))  //tcp连接失败
  {
    Serial.println("connection failed.");
    return;
  }

  //  向服务器发送一个request请求
  Serial.println(String("GET /") + "HTTP/1.1\r\n" + "Host:" + hostmsg + "\r\n" + "Connection:close\r\n\r\n");
  client.print(String("GET /") + "HTTP/1.1\r\n" + "Host:" + hostmsg + "\r\n" + "Connection:close\r\n\r\n");
  delay(3000);
  //extern String xiaoxi[40];
  //  读取服务器返回的所用行，并打印到串口监视器
  int a = 0;
  while (client.available()) {
    String line = client.readStringUntil('\r');  //此处可以看出，他是一行行的获取信息的，
    Serial.print(line);
    xiaoxi[a] = line;  //所以不妨把消息一行行存在一个String组里面，之后再一行行打印出来到屏幕上
    a = a + 1;
  }
}



void getreminder() /*从云服务器获取备忘录*/ {
  WiFiClient client;

  // 建立http请求信息
  String httpRequest = String("GET /") + "HTTP/1.1\r\n" + "Host:" + host2 + "\r\n" + "Connection:close\r\n\r\n";
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.print(host2);

  // 尝试连接服务器
  if (!client.connect(host2, httprmd))  //tcp连接失败
  {
    Serial.println("connection failed.");
    return;
  }

  //  向服务器发送一个request请求
  Serial.println(String("GET /") + "HTTP/1.1\r\n" + "Host:" + host2 + "\r\n" + "Connection:close\r\n\r\n");
  client.print(String("GET /") + "HTTP/1.1\r\n" + "Host:" + host2 + "\r\n" + "Connection:close\r\n\r\n");
  delay(3000);
  extern String reminder[30];
  //  读取服务器返回的所用行，并打印到串口监视器
  int a = 0;
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
    reminder[a] = line;
    a = a + 1;
  }
}

//  天气///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 利用ArduinoJson库解析心知天气响应信息//我直接搬过来用了，我也不会
void parseInfo(WiFiClient client, int mod = 1) {
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 230;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, client);

  JsonObject results_0 = doc["results"][0];

  JsonObject results_0_now = results_0["now"];
  const char* results_0_now_text = results_0_now["text"];                // "Sunny"
  const char* results_0_now_code = results_0_now["code"];                // "0"
  const char* results_0_now_temperature = results_0_now["temperature"];  // "32"

  const char* results_0_last_update = results_0["last_update"];  // "2020-06-02T14:40:00+08:00"

  // 通过串口监视器显示以上信息
  String results_0_now_text_str = results_0_now["text"].as<String>();
  int results_0_now_code_int = results_0_now["code"].as<int>();
  int results_0_now_temperature_int = results_0_now["temperature"].as<int>();
  results_0_now_code_int1 = results_0_now_code_int;
  results_0_now_temperature_int1 = results_0_now_temperature_int;
  String results_0_last_update_str = results_0["last_update"].as<String>();
  if (mod == 1) {
    Serial.println(F("======Weahter Now======="));
    Serial.print(F("Weather Now: "));
    Serial.print(results_0_now_text_str);
    Serial.print(F(" "));
    Serial.println(results_0_now_code_int);
    Serial.print(F("Temperature: "));
    Serial.println(results_0_now_temperature_int);
    Serial.print(F("Last Update: "));
    Serial.println(results_0_last_update_str);
    Date1 = results_0_last_update_str;
    Serial.println(F("========================"));
    switch (results_0_now_code_int) {
      //我重新绘制了各个天气的图标，大小是98*128，通过判断results_0_now_code_int的值来选择绘制哪一个天气图标（看拼音)
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
        {
          display.drawInvertedBitmap(198, 0, duoyun, 98, 128, FormC);  //画图
          break;
        }
      case 0:
      case 1:
      case 2:
      case 3:
        {
          display.drawInvertedBitmap(198, 0, qing, 98, 128, FormC);  //画图
          break;
        }
      case 9:
        {
          display.drawInvertedBitmap(198, 0, yin, 98, 128, FormC);  //画图
          break;
        }
      case 10:
        {
          display.drawInvertedBitmap(198, 0, zhenyu, 98, 128, FormC);  //画图
          break;
        }
      case 11:
      case 12:
        {
          display.drawInvertedBitmap(198, 0, leizhenyu, 98, 128, FormC);  //画图
        }
      case 13:
        {
          display.drawInvertedBitmap(198, 0, xiaoyu, 98, 128, FormC);  //画图
          break;
        }
      case 14:
        {
          display.drawInvertedBitmap(198, 0, zhongyu, 98, 128, FormC);  //画图
          break;
        }
      case 15:
      case 16:
      case 17:
      case 18:
        {
          display.drawInvertedBitmap(198, 0, dayu, 98, 128, FormC);  //画图
          break;
        }
      case 19:
      case 20:
        {
          display.drawInvertedBitmap(198, 0, dongyu, 98, 128, FormC);  //画图
          break;
        }
      case 21:
        {
          display.drawInvertedBitmap(198, 0, zhenxue, 98, 128, FormC);  //画图
          break;
        }
      case 22:
        {
          display.drawInvertedBitmap(198, 0, xiaoxue, 98, 128, FormC);  //画图
          break;
        }
      case 23:
        {
          display.drawInvertedBitmap(198, 0, zhongxue, 98, 128, FormC);  //画图
          break;
        }
      case 24:
      case 25:
        {
          display.drawInvertedBitmap(198, 0, daxue, 98, 128, FormC);  //画图
          break;
        }
      case 26:
      case 27:
        {
          display.drawInvertedBitmap(198, 0, yangsha, 98, 128, FormC);  //画图
          break;
        }
      case 28:
      case 29:
        {
          display.drawInvertedBitmap(198, 0, shachenbao, 98, 128, FormC);  //画图
          break;
        }
      case 30:
      case 31:
        {
          display.drawInvertedBitmap(198, 0, dawu, 98, 128, FormC);  //画图
          break;
        }
      case 32:
      case 33:
        {
          display.drawInvertedBitmap(198, 0, feng, 98, 128, FormC);  //画图
          break;
        }

      case 34:
      case 35:
        {
          display.drawInvertedBitmap(198, 0, taifeng, 98, 128, FormC);  //画图
          break;
        }
      default:
        {
          display.drawInvertedBitmap(198, 0, unknown, 98, 128, FormC);  //画图
          break;
        }
    }
    display.drawRoundRect(198, 0, 98, 128, 20, FormC);  //画外框
    u8g2Fonts.setFont(chinese_gb2312);
    u8g2Fonts.setForegroundColor(FormC);  // 设置前景色
    u8g2Fonts.setBackgroundColor(BackC);  // 设置背景色
    u8g2Fonts.setCursor(45, 120);
    string text0 = std::to_string(results_0_now_temperature_int);
    string text1 = "Temperature:" + text0 + "℃";
    const char* p = text1.data();
    u8g2Fonts.print(p);
  }
}


// 向心知天气服务器服务器请求信息并对信息进行解析
void httpRequest(String reqRes, int mod = 1) {
  WiFiClient client;

  // 建立http请求信息
  String httpRequest = String("GET ") + reqRes + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n";
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.print(host);

  // 尝试连接服务器
  if (client.connect(host, 80)) {
    Serial.println(" Success!");

    // 向服务器发送http请求信息
    client.print(httpRequest);
    Serial.println("Sending request: ");
    Serial.println(httpRequest);

    // 获取并显示服务器响应状态行
    String status_response = client.readStringUntil('\n');
    Serial.print("status_response: ");
    Serial.println(status_response);

    // 使用find跳过HTTP响应头
    if (client.find("\r\n\r\n")) {
      Serial.println("Found Header End. Start Parsing.");
    }

    // 利用ArduinoJson库解析心知天气响应信息
    parseInfo(client, mod);

  } else {
    Serial.println(" connection failed!");
    if (mod == 1) {
          switch (results_0_now_code_int1) {
      //我重新绘制了各个天气的图标，大小是98*128，通过判断results_0_now_code_int的值来选择绘制哪一个天气图标（看拼音)
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
        {
          display.drawInvertedBitmap(198, 0, duoyun, 98, 128, FormC);  //画图
          break;
        }
      case 0:
      case 1:
      case 2:
      case 3:
        {
          display.drawInvertedBitmap(198, 0, qing, 98, 128, FormC);  //画图
          break;
        }
      case 9:
        {
          display.drawInvertedBitmap(198, 0, yin, 98, 128, FormC);  //画图
          break;
        }
      case 10:
        {
          display.drawInvertedBitmap(198, 0, zhenyu, 98, 128, FormC);  //画图
          break;
        }
      case 11:
      case 12:
        {
          display.drawInvertedBitmap(198, 0, leizhenyu, 98, 128, FormC);  //画图
        }
      case 13:
        {
          display.drawInvertedBitmap(198, 0, xiaoyu, 98, 128, FormC);  //画图
          break;
        }
      case 14:
        {
          display.drawInvertedBitmap(198, 0, zhongyu, 98, 128, FormC);  //画图
          break;
        }
      case 15:
      case 16:
      case 17:
      case 18:
        {
          display.drawInvertedBitmap(198, 0, dayu, 98, 128, FormC);  //画图
          break;
        }
      case 19:
      case 20:
        {
          display.drawInvertedBitmap(198, 0, dongyu, 98, 128, FormC);  //画图
          break;
        }
      case 21:
        {
          display.drawInvertedBitmap(198, 0, zhenxue, 98, 128, FormC);  //画图
          break;
        }
      case 22:
        {
          display.drawInvertedBitmap(198, 0, xiaoxue, 98, 128, FormC);  //画图
          break;
        }
      case 23:
        {
          display.drawInvertedBitmap(198, 0, zhongxue, 98, 128, FormC);  //画图
          break;
        }
      case 24:
      case 25:
        {
          display.drawInvertedBitmap(198, 0, daxue, 98, 128, FormC);  //画图
          break;
        }
      case 26:
      case 27:
        {
          display.drawInvertedBitmap(198, 0, yangsha, 98, 128, FormC);  //画图
          break;
        }
      case 28:
      case 29:
        {
          display.drawInvertedBitmap(198, 0, shachenbao, 98, 128, FormC);  //画图
          break;
        }
      case 30:
      case 31:
        {
          display.drawInvertedBitmap(198, 0, dawu, 98, 128, FormC);  //画图
          break;
        }
      case 32:
      case 33:
        {
          display.drawInvertedBitmap(198, 0, feng, 98, 128, FormC);  //画图
          break;
        }

      case 34:
      case 35:
        {
          display.drawInvertedBitmap(198, 0, taifeng, 98, 128, FormC);  //画图
          break;
        }
      default:
        {
          display.drawInvertedBitmap(198, 0, unknown, 98, 128, FormC);  //画图
          break;
        }
    }
      display.drawRoundRect(198, 0, 98, 128, 20, FormC);  //画外框
      u8g2Fonts.setFont(chinese_gb2312);
      u8g2Fonts.setForegroundColor(FormC);  // 设置前景色
      u8g2Fonts.setBackgroundColor(BackC);  // 设置背景色
      u8g2Fonts.setCursor(45, 120);
      string text0 = std::to_string(results_0_now_temperature_int1);
      string text1 = "Temperature:" + text0 + "℃";
      const char* p = text1.data();
      u8g2Fonts.print(p);
      Serial.println("xxxxxx");
      Serial.println(p);
      Serial.println("xxxxxx");
      nextPage(10);
    }
  }
  //断开客户端与服务器连接工作
  client.stop();
}




void getweather(int mod = 1) {
  reqUserKey = EEPROM.readString(200);
  reqLocation = EEPROM.readString(300);
  // 建立心知天气API当前天气请求资源地址
  String reqRes = "/v3/weather/now.json?key=" + reqUserKey  +"&location=" + reqLocation + "&language=en&unit=" + reqUnit;

  // 向心知天气服务器服务器请求信息并对信息进行解析
  httpRequest(reqRes, mod);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
