#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>   //用于设备域名 MDNS.begin("esp32")
#include <esp_wifi.h>  //用于esp_wifi_restore() 删除保存的wifi信息

const int baudRate = 115200;        //设置波特率
const byte DNS_PORT = 53;           //设置DNS端口号
const int webPort = 80;             //设置Web端口号
const int resetPin = 0;             //设置重置按键引脚,用于删除WiFi信息
const int LED = 2;                  //设置LED引脚
const char* AP_SSID = "ESP32";      //设置AP热点名称
const char* AP_PASS = "123456789";  //设置AP热点密码
const char* HOST_NAME = "MY_ESP";   //设置设备名
String scanNetworksID = "";         //用于储存扫描到的WiFi ID
int connectTimeOut_s = 2;           //WiFi连接超时时间，单位秒
IPAddress apIP(192, 168, 4, 1);     //设置AP的IP地址
String wifi_ssid = "";              //暂时存储wifi账号密码
String wifi_pass = "";              //暂时存储wifi账号密码
String wifi_ssid2 = "";              //暂时存储wifi账号密码
String wifi_pass2 = "";              //暂时存储wifi账号密码
//定义根目录首页网页HTML源代码
#define ROOT_HTML "<!DOCTYPE html><html><head><title>WIFI Config by dht</title><meta name=\"viewport\" http-equiv=\"Content-Type\" content=\"width=device-width, initial-scale=1,test/html;charset=utf-8\"></head><style type=\"text/css\">.input{display: block; margin-top: 10px;}.input span{width: 100px; float: left; float: left; height: 36px; line-height: 36px;}.input input{height: 30px;width: 200px;}.btn{width: 120px; height: 35px; background-color: #000000; border:0px; color:#ffffff; margin-top:15px; margin-left:100px;}</style><body><form method=\"POST\" action=\"configwifi\"><label class=\"input\"><span>WiFi名1</span><input type=\"text\" name=\"ssid\" value=\"\"></label><label class=\"input\"><span>WiFi密码1</span><input type=\"text\"  name=\"pass\"></label><label class=\"input\"><span>WiFi名2</span><input type=\"text\" name=\"ssid2\" value=\"\"></label><label class=\"input\"><span>WiFi密码2</span><input type=\"text\"  name=\"pass2\"></label><input class=\"btn\" type=\"submit\" name=\"submit\" value=\"上传\"> <p><span> 附近的 wifi: </P></form>"
//定义成功页面HTML源代码
#define SUCCESS_HTML "<html><body><font size=\"10\">successd,wifi connecting...<br />Please close this page manually.</font></body></html>"

DNSServer dnsServer;        //创建dnsServer实例
WebServer server(webPort);  //开启web服务, 创建TCP SERVER,参数: 端口号,最大连接数

//初始化AP模式
void initSoftAP() {
  WiFi.mode(WIFI_AP);                                          //配置为AP模式
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));  //设置AP热点IP和子网掩码
  if (WiFi.softAP(AP_SSID, AP_PASS)) {                         //开启AP热点,如需要密码则添加第二个参数
    //打印相关信息
    Serial.println("ESP-32 SoftAP is right.");
    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());
    Serial.println(String("MAC address = ") + WiFi.softAPmacAddress().c_str());
  } else {  //开启热点失败
    Serial.println("WiFiAP Failed");
    delay(1000);
    Serial.println("restart now...");
    ESP.restart();  //重启复位esp32
  }
}

//初始化DNS服务器
void initDNS() {
  //判断将所有地址映射到esp32的ip上是否成功
  if (dnsServer.start(DNS_PORT, "*", apIP)) {
    Serial.println("start dnsserver success.");
  } else {
    Serial.println("start dnsserver failed.");
  }
}

//初始化WebServer
void initWebServer() {
  //给设备设定域名esp32,完整的域名是esp32.local
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  //必须添加第二个参数HTTP_GET，以下面这种格式去写，否则无法强制门户
  server.on("/", HTTP_GET, handleRoot);                   //  当浏览器请求服务器根目录(网站首页)时调用自定义函数handleRoot处理，设置主页回调函数，必须添加第二个参数HTTP_GET，否则无法强制门户
  server.on("/configwifi", HTTP_POST, handleConfigWifi);  //  当浏览器请求服务器/configwifi(表单字段)目录时调用自定义函数handleConfigWifi处理
  server.onNotFound(handleNotFound);                      //当浏览器请求的网络资源无法在服务器找到时调用自定义函数handleNotFound处理
  //Tells the server to begin listening for incoming connections.Returns None
  server.begin();  //启动TCP SERVER
                   //server.setNoDelay(true);                                  //关闭延时发送
  Serial.println("WebServer started!");
}

//扫描WiFi
bool scanWiFi() {
  Serial.println("scan start");
  // 扫描附近WiFi
  int n = WiFi.scanNetworks();  //太多了就要五个

  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
    scanNetworksID = "no networks found";
    return false;
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      scanNetworksID += "<P>" + WiFi.SSID(i) + "</P>";
      delay(10);
    }
    return true;
  }
}

void connectToWiFi(int timeOut_s, int mod = 0) {
  Serial.println("进入connectToWiFi()函数");
  //设置为STA模式并连接WIFI
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);  //设置自动连接
  //用字符串成员函数c_str()生成一个const char*指针，指向以空字符终止的数组,即获取该字符串的指针。
  if (wifi_ssid != "") {
    Serial.println("用web配置信息连接.");

    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
    wifi_ssid = "";
    wifi_pass = "";
  } else {
    Serial.println("用nvs保存的信息连接.");
    WiFi.begin();  //连接上一次连接成功的wifi
  }
  //WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
  int Connect_time = 0;                    //用于连接计时，如果长时间连接不成功，复位设备
  while (WiFi.status() != WL_CONNECTED) {  //等待WIFI连接成功
    Serial.print(".");
    digitalWrite(LED, !digitalRead(LED));
    delay(500);
    Connect_time++;
    if (Connect_time > 2 * timeOut_s && mod == 0) {  //长时间连接不上，重新进入配网页面
      digitalWrite(LED, LOW);
      Serial.println("");
      Serial.println("WIFI autoconnect fail, start AP for webconfig now...");
      wifiConfig();  //转到网页端手动配置wifi
      return;        //跳出 防止无限初始化
      //break;        //跳出 防止无限初始化
    } else if (Connect_time > 2 * timeOut_s && mod == 1) {
      return;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WIFI connect Success");
    Serial.printf("SSID:%s", WiFi.SSID().c_str());
    Serial.printf(", PSW:%s\r\n", WiFi.psk().c_str());
    Serial.print("LocalIP:");
    Serial.print(WiFi.localIP());
    Serial.print(" ,GateIP:");
    Serial.println(WiFi.gatewayIP());
    Serial.print("WIFI status is:");
    Serial.print(WiFi.status());
    digitalWrite(LED, HIGH);
    server.stop();
  }
  digitalWrite(LED, LOW);
}

//用于配置WiFi
void wifiConfig() {
  initSoftAP();
  initDNS();
  initWebServer();
  scanWiFi();
}

//处理网站根目录“/”(首页)的访问请求,将显示配置wifi的HTML页面
void handleRoot() {
  if (server.hasArg("selectSSID")) {
    server.send(200, "text/html", ROOT_HTML + scanNetworksID + "</body></html>");
  } else {
    server.send(200, "text/html", ROOT_HTML + scanNetworksID + "</body></html>");
  }
}
//提交数据后，返回给客户端信息函数
void handleConfigWifi() {
  //返回http状态
  //server.send(200, "text/html", SUCCESS_HTML);
  if (server.hasArg("ssid")) {  //判断是否有账号参数
    Serial.print("got ssid:");
    wifi_ssid = server.arg("ssid");    //获取html表单输入框name名为"ssid"的内容
    wifi_ssid2 = server.arg("ssid2");  //获取html表单输入框name名为"ssid"的内容
EEPROM.writeString(1000, wifi_ssid);
    EEPROM.writeString(1100, wifi_ssid2);
    // strcpy(sta_ssid, server.arg("ssid").c_str());//将账号参数拷贝到sta_ssid中
    Serial.println(wifi_ssid);
  } else {  //没有参数
    Serial.println("error, not found ssid");
    server.send(200, "text/html", "<meta charset='UTF-8'>error, not found ssid");  //返回错误页面
    return;
  }
  //密码与账号同理
  if (server.hasArg("pass")) {
    Serial.print("got password:");
    wifi_pass = server.arg("pass");    //获取html表单输入框name名为"pwd"的内容
    wifi_pass2 = server.arg("pass2");  //获取html表单输入框name名为"pwd"的内容
    EEPROM.writeString(1050, wifi_pass);
EEPROM.writeString(1150, wifi_pass2);
    //strcpy(sta_pass, server.arg("pass").c_str());
    Serial.println(wifi_pass);
  } else {
    Serial.println("error, not found password");
    server.send(200, "text/html", "<meta charset='UTF-8'>error, not found password");
    return;
  }
  EEPROM.commit();
  server.send(200, "text/html", "<meta charset='UTF-8'>SSID：" + wifi_ssid + "<br />password:" + wifi_pass + "<br />已取得WiFi信息,正在尝试连接,请手动关闭此页面。");  //返回保存成功页面
  delay(2000);
  WiFi.softAPdisconnect(true);  //参数设置为true，设备将直接关闭接入点模式，即关闭设备所建立的WiFi网络。
  server.close();               //关闭web服务
  WiFi.softAPdisconnect();      //在不输入参数的情况下调用该函数,将关闭接入点模式,并将当前配置的AP热点网络名和密码设置为空值.
  Serial.println("WiFi Connect SSID:" + wifi_ssid + "  PASS:" + wifi_pass);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("开始调用连接函数connectToWiFi()..");
    connectToWiFi(connectTimeOut_s);  //进入配网阶段
  } else {
    Serial.println("提交的配置信息自动连接成功..");
  }
}

// 设置处理404情况的函数'handleNotFound'
void handleNotFound() {  // 当浏览器请求的网络资源无法在服务器找到时通过此自定义函数处理
  handleRoot();          //访问不存在目录则返回配置页面
  //   server.send(404, "text/plain", "404: Not found");
}


//删除保存的wifi信息,并使LED闪烁5次
void restoreWiFi() {
  esp_wifi_restore();  //删除保存的wifi信息
  Serial.println("连接信息已清空,准备重启设备..");
}

void checkConnect(bool reConnect) {
  if (WiFi.status() != WL_CONNECTED) {
    //  Serial.println("WIFI未连接.");
    //  Serial.println(WiFi.status());
    if (reConnect == true && WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA) {
      Serial.println("WIFI未连接.");
      Serial.println("WiFi Mode:");
      Serial.println(WiFi.getMode());
      Serial.println("正在连接WiFi...");
      connectToWiFi(connectTimeOut_s);
    }
  }
}


void PeiWang() {
  if (digitalRead(button3) == LOW) {
    delay(200);
    if (digitalRead(button3) == LOW) {
      wifi_ssid = EEPROM.readString(1100);
      wifi_pass = EEPROM.readString(1150);
      centerx("已启用备用wifi信息",125);
      nextPage();
    }
  }else{
wifi_ssid = EEPROM.readString(1000);
      wifi_pass = EEPROM.readString(1050);    
  }
  Serial.begin(baudRate);
  WiFi.hostname(HOST_NAME);  //设置设备名
  connectToWiFi(connectTimeOut_s);
  do {
    //长按5毫秒(P0)清除网络配置信息
    delay(500);
    if (digitalRead(button1) == LOW) {
      //delay(10);
      //if (digitalRead(resetPin)==LOW) {
      //Serial.println("\n按键已长按2秒,正在清空网络连接保存信息.");
      display.firstPage();
      display.fillScreen(BackC);
      centerx("按键已按下,正在清空网络连接保存信息...", 50);
      nextPage();
      restoreWiFi();  //删除保存的wifi信息
      display.firstPage();
      display.fillScreen(BackC);
      centerx("即将重启复位...", 50);
      nextPage();
      ESP.restart();  //重启复位esp32
      Serial.println("已重启设备.");
      //}
      digitalWrite(LED, LOW);
    }
    dnsServer.processNextRequest();  //检查客户端DNS请求
    server.handleClient();           //检查客户端(浏览器)http请求
    checkConnect(true);              //检测网络连接状态，参数true表示如果断开重新连接
    digitalWrite(LED, LOW);
  } while (WiFi.status() != WL_CONNECTED);digitalWrite(LED, LOW);
}

void PeiWangStart() {
  display.firstPage();
  display.fillScreen(BackC);
  display.drawInvertedBitmap(116, 10, peiwangpic, 64, 64, FormC);
  centerx("配网中..若连接失败将在10秒后将自动创建热点", 90);
  centerx("如需添加网络,请连接热点：ESP32", 110);
  nextPage();
  PeiWang();  //见PeiWang.h
  centerx("配网成功", 90);
  nextPage();
  digitalWrite(LED, LOW);
  mod0setup();digitalWrite(LED, LOW);
}