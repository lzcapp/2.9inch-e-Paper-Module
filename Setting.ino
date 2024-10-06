#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>                  //用于设备域名 MDNS.begin("esp32")
#include <esp_wifi.h>                 //用于esp_wifi_restore() 删除保存的wifi信息
#include <Update.h>                   // 引用相关库
const char* APset_SSID = "ESP32SET";  //设置AP热点名称
String bafaUID = "";                  //暂时存储巴法云私钥
String xinzhiUID = "";                //暂时存储心知天气私钥
String xinzhiCity = "";               //暂时存储心知天气城市
String bafaTOPIC = "";
String bafaNAME = "";
String Quotemod = "";
int quotemod=0;
bool shouldreboot = false;  // 重启标志，固件升级操作完成后设置该标志准备重启设备
//定义根目录首页网页HTML源代码
const char* ROOTset_HTML PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>WIFI Config by dht</title>
  <meta name="viewport" http-equiv="Content-Type" content="width=device-width, initial-scale=1,test/html;charset=utf-8">
  <style type="text/css">
    }
    body {
      display: flex;
      align-items: center;
      justify-content: center;
      flex-direction: column;
      background-color: #f5f5f5;
    }
    .set {
      text-align: center;
      display: block;
      margin-top: 10px;
      background-color: rgb(130, 175, 241);
    }
    .end {
      text-align: center;
      display: block;
      margin-top: 10px;
      background-color: rgb(164, 201, 255);
    }
    .end span {
      text-align: center;
      width: 150px;
      float: center;
      height: 36px;
      line-height: 36px;
      color: rgb(255, 255, 255);
    }
    .set span {
      text-align: center;
      width: 150px;
      float: center;
      height: 36px;
      line-height: 36px;
      color: rgb(255, 255, 255);
    }
    .input {
      text-align: center;
      display: block;
      margin-top: 10px;
    }
    .input span {
      text-align: center;
      width: 150px;
      float: center;
      height: 36px;
      line-height: 36px;
      color: rgb(130, 175, 241);
    }
    .input input {
      text-align: center;
      height: 30px;
      width: 150px;
    }
    .btn {
      width: 120px;
      height: 35px;
      background-color: rgb(130, 175, 241);
      border: 0px;
      color: white;
      margin-top: 15px;
      margin-left: auto;
      margin-right: auto;
      display: block;
      border-radius: 8px;
      cursor: pointer;
    }
    font {
      color: rgb(130, 175, 241);
      text-align: center;
    }
    input[type="file"] {
      display: block;
      margin: 15px auto;
      padding: .5rem;
      width: 150px;
      height: auto;
      background-color: rgb(130, 175, 241);
      color: rgb(255, 255, 255);
      border-radius: 5px;
      overflow: hidden;
      cursor: pointer;
    }
    input[type="submit"] {
      display: block;
      margin: 15px auto;
      padding: 0.5rem;
      width: 100px;
      height: auto;
      background-color: rgb(130, 175, 241);
      color: #fff;
      border-radius: 5px;
      border: none;
      cursor: pointer;
    }
  </style>
</head>
<body>
  <label class="set"><span>EPA-02 设置</span></label>
  <form method="POST" action="configset">
    <label class="input">
<span>巴法云UID</span>
<input type="text" placeholder="使用图库，备忘录必填项" name="bafaUID" value="">
</label>
    <label class="input">
<span>巴法云TOPIC</span>
<input type="text"placeholder="使用图库，备忘录必填项" name="bafaTOPIC" value="">
</label>
    <label class="input">
<span>巴法云昵称</span>
<input type="text" placeholder="使用图库，备忘录必填项" name="bafaNAME" value="">
</label>
    <label class="input">
<span>心知天气UID</span>
<input type="text" placeholder="使用天气时钟必填项" name="xinzhiUID">
</label>
    <label class="input">
<span>心知天气城市</span>
<input type="text" placeholder="使用天气时钟必填项" name="xinzhiCity" value="">  
<label class="input">
<span>格言来源</span>
<input type="text"placeholder="请填 一言 或 历史" name="quote" value="">  
<input class="btn" type="submit" name="submit" value="上传">
</label><label class="set"><span>EPA-02 OTA升级</span></label>

    <label class="input"><SPAN><a href="http://bin.bemfa.com/b/3BcN2ViMWFjNTk5NDA1NDc0ODlkN2E3NzllMjExMWExNTE=ota.bin">前往下载ota文件 </a></SPAN></label>
  </form>
  <form method='POST' action='/update' enctype='multipart/form-data'>
    <input style="display:block;margin-left:auto;margin-right:auto;margin-top:15px" type='file' name='firmware'>
    <input style="display:block;margin-left:auto;margin-right:auto;margin-top:15px" type='submit' style="color:blue">
  </form>
  <label class="input">
</label>
</body>
</label><label class="end"><span>Designed by dht</span></label>
</html>
)=====";


// const char* ROOTset_HTML =
// "";


//定义成功页面HTML源代码
#define SUCCESSset_HTML "<html><body><font size=\"10\">successd<br />Please close this page manually.</font></body></html>"
String _updaterError = "";

//初始化AP模式
void initSoftAPset() {
  WiFi.mode(WIFI_AP);                                          //配置为AP模式
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));  //设置AP热点IP和子网掩码
  if (WiFi.softAP(APset_SSID)) {                               //开启AP热点,如需要密码则添加第二个参数
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
void initDNSset() {
  //判断将所有地址映射到esp32的ip上是否成功
  if (dnsServer.start(DNS_PORT, "*", apIP)) {
    Serial.println("start dnsserver success.");
  } else {
    Serial.println("start dnsserver failed.");
  }
}
void handleResponse()  //回调函数
{
  server.sendHeader("Connection", "close");
  //if (Update.hasError()) {
  //  server.send(200, F("text/html"), String(F("Update error:")) + _updaterError);
  //}
  server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
}

void handleFileupload()  //回调函数
{
  HTTPUpload& upload = server.upload();    // 文件上传对象
  if (upload.status == UPLOAD_FILE_START)  // 文件上传开始
  {
    Serial.printf("开始上传文件: %s\n", upload.filename.c_str());
    if (!Update.begin())  // 开始固件升级，检查空可用间大小，如果正在升级或空间不够则返回false
    {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE)  // 文件读写中
  {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)  // 将文件数据写入到OTA区域
    {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END)  // 文件上传完成
  {
    Serial.println("写入" + String(upload.totalSize) + "字节到OTA区域");
    if (!Update.end(true))  // 完成数据写入，设置在系统重启后自动将OTA区域固件移动到Sketch区域 // Update.begin不指定大小时这里设置true
    {
      Update.printError(Serial);
    }
    Serial.println("升级操作完成，模块将在5秒后重启以完成固件升级");
    shouldreboot = true;
  } else {
    Serial.printf("固件上传失败: status=%d\n", upload.status);
  }
}

//初始化WebServer
void initWebServerset() {
  //给设备设定域名esp32,完整的域名是esp32.local
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  //必须添加第二个参数HTTP_GET，以下面这种格式去写，否则无法强制门户
  server.on("/", HTTP_GET, handleRootset);            //  当浏览器请求服务器根目录(网站首页)时调用自定义函数handleRootset处理，设置主页回调函数，必须添加第二个参数HTTP_GET，否则无法强制门户
  server.on("/configset", HTTP_POST, handleSetting);  //  当浏览器请求服务器/configset(表单字段)目录时调用自定义函数handleSetting处理
  server.on("/update", HTTP_POST, handleResponse, handleFileupload);
  server.onNotFound(handleNotFoundset);  //当浏览器请求的网络资源无法在服务器找到时调用自定义函数handleNotFound处理
  //Tells the server to begin listening for incoming connections.Returns None
  server.begin();  //启动TCP SERVER
                   //server.setNoDelay(true);                                  //关闭延时发送
  Serial.println("WebServer started!");
}

//处理网站根目录“/”(首页)的访问请求,将显示配置wifi的HTML页面
void handleRootset() {
  if (server.hasArg("selectSSID")) {
    server.send(200, "text/html", ROOTset_HTML + scanNetworksID + "</body></html>");
  } else {
    server.send(200, "text/html", ROOTset_HTML + scanNetworksID + "</body></html>");
  }
}
//提交数据后，返回给客户端信息函数
void handleSetting() {
  bool commit = false;
  bool hiku = false;
  bool history = false;
  //返回http状态
  //server.send(200, "text/html", SUCCESS_HTML);
  if (server.hasArg("bafaUID") && server.arg("bafaUID").length() > 0) {  //判断是否有账号参数
    Serial.print("got bafaUID:");
    bafaUID = server.arg("bafaUID");  //获取html表单输入框name名为"bafaUID"的内容
    EEPROM.writeString(100, bafaUID);
    Serial.println(bafaUID);
    commit = true;
  }
  /*else {  //没有参数
    Serial.println("error, not found bafaUID");
    server.send(200, "text/html", "<meta charset='UTF-8'>error, not found bafaUID");  //返回错误页面
    return;
  }*/
  //密码与账号同理
  if (server.hasArg("xinzhiUID") && server.arg("xinzhiUID").length() > 0) {
    Serial.print("got xinzhiUID:");
    xinzhiUID = server.arg("xinzhiUID");  //获取html表单输入框name名为"xinzhiUID"的内容
    EEPROM.writeString(200, xinzhiUID);
    Serial.println(xinzhiUID);
    commit = true;
  }
  if (server.hasArg("xinzhiCity") && server.arg("xinzhiCity").length() > 0) {  //判断是否有账号参数
    Serial.print("got xinzhiCity:");
    xinzhiCity = server.arg("xinzhiCity");  //获取html表单输入框name名为"xinzhiCity"的内容
    EEPROM.writeString(300, xinzhiCity);
    Serial.println(xinzhiCity);
    commit = true;
  }
  if (server.hasArg("bafaTOPIC") && server.arg("bafaTOPIC").length() > 0) {  //判断是否有账号参数
    Serial.print("got bafaTOPIC:");
    bafaTOPIC = server.arg("bafaTOPIC");  //获取html表单输入框name名为"bafaTOPIC"的内容
    EEPROM.writeString(400, bafaTOPIC);
    Serial.println(bafaTOPIC);
    commit = true;
  }
  if (server.hasArg("bafaNAME") && server.arg("bafaNAME").length() > 0) {  //判断是否有账号参数
    Serial.print("got bafaNAME:");
    bafaNAME = server.arg("bafaNAME");  //获取html表单输入框name名为"bafaNAME"的内容
    EEPROM.writeString(500, bafaNAME);
    Serial.println(bafaNAME);
    commit = true;
  }
  if (server.hasArg("quote") && server.arg("quote").length() > 0) {  //判断是否有账号参数
    Quotemod=server.arg("quote");
      if (Quotemod=="一言") {
    EEPROM.writeString(980, "0");
    delay(200);EEPROM.commit();
    
  }else if (Quotemod=="历史"){
    EEPROM.writeString(980, "1");
    delay(200);EEPROM.commit();
  }
    commit = true;
  }

  if (commit == true) {
    EEPROM.commit();
    server.send(200, "text/html", "<meta charset='UTF-8'>巴法云UID：" + bafaUID + "<br />巴法云TOPIC:" + bafaTOPIC + "<br />巴法云昵称:" + bafaNAME + "<br />心知UID:" + xinzhiUID + "<br />心知城市:" + xinzhiCity + "<br />已取得信息,请手动关闭此页面。");  //返回保存成功页面
    delay(200);
    WiFi.softAPdisconnect(true);  //参数设置为true，设备将直接关闭接入点模式，即关闭设备所建立的WiFi网络。
    server.close();               //关闭web服务
    WiFi.softAPdisconnect();      //在不输入参数的情况下调用该函数,将关闭接入点模式,并将当前配置的AP热点网络名和密码设置为空值.
    //PeiWangStart();
  }
}

// 设置处理404情况的函数'handleNotFoundset'
void handleNotFoundset() {  // 当浏览器请求的网络资源无法在服务器找到时通过此自定义函数处理
  handleRootset();          //访问不存在目录则返回配置页面
  //   server.send(404, "text/plain", "404: Not found");
}


void setConfig() {
  initSoftAPset();
  initDNSset();
  initWebServerset();
}

void Setting() {
  Serial.begin(baudRate);
  WiFi.hostname(HOST_NAME);  //设置设备名
  setConfig();
  do {
    dnsServer.processNextRequest();  //检查客户端DNS请求
    server.handleClient();           //检查客户端(浏览器)http请求
    if (shouldreboot) {
      delay(5000);
      ESP.restart();  // 重启设备
    }
    if (digitalRead(button2) == HIGH) {
      WiFi.softAPdisconnect(true);  //参数设置为true，设备将直接关闭接入点模式，即关闭设备所建立的WiFi网络。
      server.close();               //关闭web服务
      WiFi.softAPdisconnect();
      break;
    }
  } while (1);
}

void SettingStart() {
  display.firstPage();
  display.fillScreen(BackC);
  display.drawInvertedBitmap(116, 10, peiwangpic, 64, 64, FormC);
  centerx("设置中...", 90);
  centerx("请连接热点：ESP32SET，进行配置", 110);
  nextPage();
  Setting();  
  centerx("设置完成", 90);
  nextPage();
  mod0setup();
}