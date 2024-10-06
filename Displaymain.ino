#include <AHT20.h>
#include <SPIFFS.h>
#include <U8glib.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <_Time.h>
#include <esp_sleep.h>
#include <string>
#include "FS.h"
#include "SD.h"
#include "SPIFFS.h"
extern const int LED; // 设置LED引脚
extern int quotemod;
extern WiFiUDP ntpUDP;
AHT20 aht20;
#include <NTPClient.h>
extern NTPClient timeClient; // 时间（参考ntp获取时间https://blog.csdn.net/veray/article/details/121940328）
#include <string>
using std::string;
String autostart = "";
extern int results_0_now_temperature_int1;
extern int results_0_now_code_int1;
extern const char *dayOfWeekName[];
extern int week;
extern const char *thetime1;
extern const char *theday1;
extern int time00; // 离线时钟计时1
extern int time01; // 离线时钟计时2
extern int hour0; // 时
extern int min0; // 分
extern String reqLocation;
extern String Date1;
extern String xinzhiCity;
extern void gettime111(int, int, int, string);
extern void getweather(int);
extern void gettime0();
extern void connectToWiFi(int, int);
extern void getmsg();
String xiaoxi[40];
File file[5];
int theme = 1;
void kbl(int &i, int &j);
void kbr(int &i, int &j);
void kbd(int &i, int &j);
void keyb(String &);
// void centerx(const char* text, int y, int mod = 1, int x0 = 0, int dx = 296, char* MOD = "否");
#define FormC (theme == 1) ? GxEPD_BLACK : GxEPD_WHITE
#define BackC (theme == 1) ? GxEPD_WHITE : GxEPD_BLACK
bool clockfirsttime = true;

// 相关函数初始化
// 连接WIFI

void espsleep() {
    gpio_wakeup_enable(GPIO_NUM_12, GPIO_INTR_HIGH_LEVEL);
    gpio_wakeup_enable(GPIO_NUM_16, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable(GPIO_NUM_0, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    esp_light_sleep_start();
}
//////////////////////////////////////////////
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

// 巴法云服务器地址默认即可
#define TCP_SERVER_ADDR "bemfa.com"
// 服务器端口，tcp创客云端口8344
#define TCP_SERVER_PORT "8344"

//********************需要修改的部分*******************//
#define DEFAULT_STASSID "TP-LINK_8BD0" // WIFI名称，区分大小写，不要写错
#define DEFAULT_STAPSW "dht3com3218" // WIFI密码
String UID = ""; // 用户私钥，可在控制台获取,修改为自己的UID
// String TOPICmsg = "message";                      //主题名字，可在控制台新建
//**************************************************//

// 最大字节数
#define MAX_PACKETSIZE 10000
// 设置心跳值30s
#define KEEPALIVEATIME 60 * 1000

// tcp客户端相关初始化，默认即可
WiFiClient TCPclient;
String TcpClient_Buff = "";
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0; // 心跳
unsigned long preTCPStartTick = 0; // 连接
bool preTCPConnected = false;

class bafa {
public:
    String getinfo;
    string F;
    //////////////////////////
    String msgTOPIC;
    bool msgfirsttime;
    bool picfirsttime;
    String reminder[5];
    int rmdlength[5];
    int position[6];
    String rmd;
    /////////////////////
    String picTOPIC;
    String pic[5];
    ////////////////////
    String chatTOPIC;
    String chat;
    String upUrl;
    bafa() {
        getinfo = "";
        msgTOPIC = "";
        msgfirsttime = true;
        rmd = "";
        picfirsttime = true;
        picTOPIC = "";
        for (int i = 0; i < 5; i++)
            pic[i] = "";

        chatTOPIC = "test";
        chat = "";
        // upUrl = "http://bin.bemfa.com/b/3BcN2ViMWFjNTk5NDA1NDc0ODlkN2E3NzllMjExMWExNTE=ota.bin";
    }
    void info(String &, bool &, int, int, int);
    void showrmd(String);
    void showpic(String);
    void showchat();
    void putchat(String, String, String);
    // 连接WIFI
    void doWiFiTick();
    void startSTA();
    // TCP初始化连接
    void sendtoTCPServer(String);
    void startTCPClient();
};
bafa MY;
void showtime(int);
void nextPage(int n = 10, int mod = 0) /*屏幕刷新（每10次局刷进行一次全局刷新）*/ {
    n = (theme == 0) ? n / 2 : n;

    if (tick < n) {
        if (mod == 0) {
            if (tick == 0 || tick == 5) {
                if (WiFi.status() == WL_CONNECTED) {
                    display.drawInvertedBitmap(286, 0, WIFIY, 8, 8, FormC);

                } else {
                    display.drawInvertedBitmap(286, 0, WIFIN, 8, 8, FormC);
                }
                display.setPartialWindow(0, 0, 296, 128); // 设置局部刷新窗口
                showtime(1);
                display.drawInvertedBitmap(2, 0, BATTERY, 16, 8, FormC);
                double bfb = getBatVolNew();
                bfb = (bfb + 5) / 20;
                for (int i = 0; i < (bfb - 1); i++) {
                    display.drawFastVLine(5 + i * 2, 2, 4, FormC);
                }
            }
            display.nextPage();
        } else {
            display.setPartialWindow(0, 0, 296, 128); // 设置局部刷新窗口
            display.nextPage();
        }
        tick += 1;
    } else {
        display.setFullWindow();
        display.nextPage();
        tick = 0;
    }
    digitalWrite(LED, LOW);
}

void centerx_B(char *text, int y, int mod = 1, int x0 = 0, int dx = 296, char *MOD = "否") {
    int text_width = u8g2Fonts.getUTF8Width(text); // 计算文本长度
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    int d = (text_width / dx) + 1;
    text_width /= d;
    for (int i = 0; i <= d - 1; i++) {
        const char *dtext = "";
        String s = text;
        s = s.substring(i * (s.length() / d), (1 + i) * (s.length() / d));
        dtext = s.c_str();
        Serial.println(dtext);

        u8g2Fonts.setCursor(x0 + ((dx - text_width) / 2),
                            ((MOD == "完全居中") ? y - (d * 16 / 2) : y) + 16 * i); // 文本居中
        u8g2Fonts.print(dtext);
    }
}
void centerx(char *text, int y, int mod = 1, int x0 = 0, int dx = 296, char *MOD = "否") {
    int text_width = u8g2Fonts.getUTF8Width(text); // 计算文本长度
    switch (mod) {
        case 1:
            u8g2Fonts.setForegroundColor(FormC);
            u8g2Fonts.setBackgroundColor(BackC);
            break;
        default:
            u8g2Fonts.setForegroundColor(BackC);
            u8g2Fonts.setBackgroundColor(FormC);
            break;
    }
    int d = (text_width / dx) + 1;
    text_width /= d;
    for (int i = 0; i <= d - 1; i++) {
        const char *dtext = "";
        String s = text;
        s = s.substring(i * (s.length() / d), (1 + i) * (s.length() / d));
        dtext = s.c_str();
        Serial.println(dtext);

        u8g2Fonts.setCursor(x0 + ((dx - text_width) / 2),
                            ((MOD == "完全居中") ? y - (d * 16 / 2) : y) + 16 * i); // 文本居中
        u8g2Fonts.print(dtext);
    }
}
void centerx_B(const char *text, int y, int mod = 1, int x0 = 0, int dx = 296, char *MOD = "否") {
    int text_width = u8g2Fonts.getUTF8Width(text); // 计算文本长度
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    int d = (text_width / dx) + 1;
    text_width /= d;
    for (int i = 0; i <= d - 1; i++) {
        const char *dtext = "";
        String s = text;
        s = s.substring(i * (s.length() / d), (1 + i) * (s.length() / d));
        dtext = s.c_str();
        Serial.println(dtext);
        u8g2Fonts.setCursor(x0 + ((dx - text_width) / 2),
                            ((MOD == "完全居中") ? y - (d * 16 / 2) : y) + 16 * i); // 文本居中
        u8g2Fonts.print(dtext);
    }
}
void centerx(const char *text, int y, int mod = 1, int x0 = 0, int dx = 296, char *MOD = "否") {
    int text_width = u8g2Fonts.getUTF8Width(text); // 计算文本长度
    switch (mod) {
        case 1:
            u8g2Fonts.setForegroundColor(FormC);
            u8g2Fonts.setBackgroundColor(BackC);
            break;
        default:
            u8g2Fonts.setForegroundColor(BackC);
            u8g2Fonts.setBackgroundColor(FormC);
            break;
    }
    int d = (text_width / dx) + 1;
    text_width /= d;
    for (int i = 0; i <= d - 1; i++) {
        const char *dtext = "";
        String s = text;
        s = s.substring(i * (s.length() / d), (1 + i) * (s.length() / d));
        dtext = s.c_str();
        Serial.println(dtext);
        u8g2Fonts.setCursor(x0 + ((dx - text_width) / 2),
                            ((MOD == "完全居中") ? y - (d * 16 / 2) : y) + 16 * i); // 文本居中
        u8g2Fonts.print(dtext);
    }
}
void centerx_B(String &text1, int y, int mod = 1, int x0 = 0, int dx = 296, char *MOD = "否") {

    const char *text = text1.c_str();
    int text_width = u8g2Fonts.getUTF8Width(text); // 计算文本长度

    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);

    int d = (text_width / dx) + 1;
    text_width /= d;
    for (int i = 0; i <= d - 1; i++) {
        const char *dtext = "";
        String s = text;
        s = s.substring(i * (s.length() / d), (1 + i) * (s.length() / d));
        dtext = s.c_str();
        Serial.println(dtext);
        u8g2Fonts.setCursor(x0 + ((dx - text_width) / 2),
                            ((MOD == "完全居中") ? y - (d * 16 / 2) : y) + 16 * i); // 文本居中
        u8g2Fonts.print(dtext);
    }
}
void centerx(String &text1, int y, int mod = 1, int x0 = 0, int dx = 296, char *MOD = "否") {

    const char *text = text1.c_str();
    int text_width = u8g2Fonts.getUTF8Width(text); // 计算文本长度
    switch (mod) {
        case 1:
            u8g2Fonts.setForegroundColor(FormC);
            u8g2Fonts.setBackgroundColor(BackC);
            break;
        default:
            u8g2Fonts.setForegroundColor(BackC);
            u8g2Fonts.setBackgroundColor(FormC);
            break;
    }
    int d = (text_width / dx) + 1;
    text_width /= d;
    for (int i = 0; i <= d - 1; i++) {
        const char *dtext = "";
        String s = text;
        s = s.substring(i * (s.length() / d), (1 + i) * (s.length() / d));
        dtext = s.c_str();
        Serial.println(dtext);
        u8g2Fonts.setCursor(x0 + ((dx - text_width) / 2),
                            ((MOD == "完全居中") ? y - (d * 16 / 2) : y) + 16 * i); // 文本居中
        u8g2Fonts.print(dtext);
    }
}
void showtime(int mod) {
    gettime0();
    string h = (hour0 < 10) ? ("0" + std::__cxx11::to_string(hour0)) : (std::__cxx11::to_string(hour0));
    string m = (min0 < 10) ? ("0" + std::__cxx11::to_string(min0)) : (std::__cxx11::to_string(min0));
    string x = h + ":" + m; // 拼凑出离线时间
    const char *text = x.c_str(); // string转char才能被输出在屏幕
    u8g2Fonts.setFont(u8g2_font_pcsenior_8n);
    centerx(text, 6, mod, 0, 296); // thetime1见gettime里的变量//打印出时间
    u8g2Fonts.setFont(chinese_gb2312);
}
void writeFile(fs::FS &fs, const char *path, const char *message) {
    Serial.printf("Writing file: %s\r\n", path);
    File file = fs.open(path, FILE_WRITE, FILE_WRITE);
    if (!file) {
        Serial.println("- failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        Serial.println("- file written");
        display.firstPage();

    } else {
        Serial.println("- write failed");
        display.firstPage();
        centerx("文件保存失败", 64);
        nextPage();
    }
    file.close();
}
/*
 *发送数据到TCP服务器
 */
void bafa::sendtoTCPServer(String p) {

    if (!TCPclient.connected()) {
        Serial.println("Client is not readly");

        return;
    }
    TCPclient.print(p);
    Serial.println(p);
    preHeartTick = millis(); // 心跳计时开始，需要每隔60秒发送一次数据
}


/*
 *初始化和服务器建立连接
 */
void bafa::startTCPClient() {
    MY.msgTOPIC = EEPROM.readString(400);
    MY.picTOPIC = EEPROM.readString(400);
    MY.chatTOPIC = EEPROM.readString(500);
    UID = EEPROM.readString(100);
    if (TCPclient.connect(TCP_SERVER_ADDR, atoi(TCP_SERVER_PORT))) {
        Serial.print("\nConnected to server:");
        Serial.printf("%s:%d\r\n", TCP_SERVER_ADDR, atoi(TCP_SERVER_PORT));
        String tcpTemp = ""; // 初始化字符串
        tcpTemp = "cmd=1&uid=" + UID + "&topic=" + msgTOPIC + "," + picTOPIC + "," + chatTOPIC + "\r\n"; // 构建订阅指令
        sendtoTCPServer(tcpTemp); // 发送订阅指令
        tcpTemp = ""; // 清空
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
int putpicstate[5] = {0, 0, 0, 0, 0};

void bafa::info(String &TOPIC, bool &firsttime, int mod, int no = 1, int putpicstate = 0) {
    MY.msgTOPIC = EEPROM.readString(400);
    MY.picTOPIC = EEPROM.readString(400);
    MY.chatTOPIC = EEPROM.readString(500);
    UID = EEPROM.readString(100);
    // 检查是否断开，断开后重连
    if (WiFi.status() != WL_CONNECTED)
        return;

    if (!TCPclient.connected()) { // 断开重连

        if (preTCPConnected == true) {

            preTCPConnected = false;
            preTCPStartTick = millis();
            Serial.println();
            Serial.println("TCP Client disconnected.");
            TCPclient.stop();
        } else if (millis() - preTCPStartTick > 1 * 1000) // 重新连接
            startTCPClient();
    } else {

        if (TCPclient.available()) { // 收数据
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
        if (millis() - preHeartTick >= KEEPALIVEATIME) { // 保持心跳

            preHeartTick = millis();
            Serial.println("--Keep alive:");
            sendtoTCPServer("cmd=0&msg=keep\r\n");
        }
    }
    if ((TcpClient_Buff.length() >= 1) && (millis() - TcpClient_preTick >= 200)) { // data ready

        TCPclient.flush();
        Serial.print("Rev string: ");
        TcpClient_Buff.trim(); // 去掉首位空格
        Serial.println(TcpClient_Buff); // 打印接收到的消息
        String getTopic = "";
        String upload = "cmd=9&uid=" + UID + "&topic=" + TOPIC + "\r\n";
        getinfo = "";
        if (firsttime == true) {
            sendtoTCPServer(upload); // 返回上次最后一条消息
            firsttime = false;
        }
        if (TcpClient_Buff.length() >
            15) { // 注意TcpClient_Buff只是个字符串，在上面开头做了初始化 String TcpClient_Buff = "";

            // 此时会收到推送的指令，指令大概为 cmd=2&uid=xxx&topic=light002&msg=off
            int topicIndex = TcpClient_Buff.indexOf("&topic=") +
                             7; // c语言字符串查找，查找&topic=位置，并移动7位，不懂的可百度c语言字符串查找
            int msgIndex = TcpClient_Buff.indexOf("&msg="); // c语言字符串查找，查找&msg=位置
            // getTopic = TcpClient_Buff.substring(topicIndex, msgIndex);
            // //c语言字符串截取，截取到topic,不懂的可百度c语言字符串截取
            getinfo = TcpClient_Buff.substring(msgIndex + 5); // c语言字符串截取，截取到消息
            // Serial.print("topic:------");
            // Serial.println(getTopic);  //打印截取到的主题值
            /// Serial.print("msg:--------");
            Serial.println(getinfo); // 打印截取到的消息值
            if (mod == 1) {
                rmd = getinfo;
                EEPROM.writeString(600, getinfo);
                EEPROM.commit();
                showrmd(rmd);
            } else if (mod == 2 && putpicstate == 0) {
                pic[no] = getinfo;
                // showpic(pic[no]);
                F = "/" + std::__cxx11::to_string(no) + ".txt";
                const char *f = F.c_str();
                const char *Info = pic[no].c_str();
                writeFile(SPIFFS, f, Info);
                showpic(pic[no]);
                centerx("接收中...", 64);
                nextPage();
            } else if (mod == 2 && putpicstate == 1) {
                pic[no] += getinfo;
                // showpic(pic[no]);
                F = "/" + std::__cxx11::to_string(no) + ".txt";
                const char *f = F.c_str();
                const char *Info = pic[no].c_str();
                writeFile(SPIFFS, f, Info);
                showpic(pic[no]);
                nextPage();
            } else if (mod == 3) {
                chat = getinfo;
                showchat();
            }
        }
        TcpClient_Buff = "";
        TcpClient_BuffIndex = 0;
    }
}


void bafa::startSTA() {
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
void bafa::doWiFiTick() {
    static bool startSTAFlag = false;
    static bool taskStarted = false;
    static uint32_t lastWiFiCheckTick = 0;

    /*if (!startSTAFlag) {
      startSTAFlag = true;
      startSTA();
      Serial.printf("Heap size:%d\r\n", ESP.getFreeHeap());
    }*/

    // 未连接1s重连
    if (WiFi.status() != WL_CONNECTED) {
        if (millis() - lastWiFiCheckTick > 1000) {
            lastWiFiCheckTick = millis();
        }
    }
    // 连接成功建立
    else {
        if (taskStarted == false) {
            taskStarted = true;
            Serial.print("\r\nGet IP Address: ");
            Serial.println(WiFi.localIP());
            startTCPClient();
        }
    }
}


///////////////////////////////////
void bafa::showrmd(String remd) {
    MY.msgTOPIC = EEPROM.readString(400);
    MY.picTOPIC = EEPROM.readString(400);
    MY.chatTOPIC = EEPROM.readString(500);
    display.firstPage();
    display.fillScreen(BackC);
    for (int i = 0; i < 5; i++) {
        display.drawRoundRect(0, 5 + i * 25, 296, 20, 4, FormC); //
    }
    position[0] = remd.indexOf("*1*");
    position[1] = remd.indexOf("*2*");
    position[2] = remd.indexOf("*3*");
    position[3] = remd.indexOf("*4*");
    position[4] = remd.indexOf("*5*");
    position[5] = remd.indexOf("*6*");

    for (int i = 0; i < 5; i++) {
        rmdlength[i] = position[i + 1] - position[i] - 2;
    }
    for (int i = 0; i < 5; i++) {

        reminder[i] = remd.substring(position[i] + 3, position[i + 1]);
        if (reminder[i].length() != 0)
            centerx(reminder[i], 30 + 25 * i, 1, 0, 296, "完全居中");
    }
    nextPage();
    getinfo = "";
}

void bafa::putchat(String from, String to, String s) {
    doWiFiTick();
    info(chatTOPIC, msgfirsttime, 4);
    getweather(2); // 借用心知天气api里的日期
    Date1 = Date1.substring(0, 10); // 取date的前10个字符（就是完整日期了）date在getweather()中有赋值
    timeClient.begin();
    timeClient.update();
    String thetime = timeClient.getFormattedTime(); // 时刻
    thetime = thetime.substring(0, 5); // 截取了00：00（时和分）
    String TIME = Date1 + " " + thetime;
    chat = "*1*" + from + ":" + s + "*2*" + "发送时间:" + TIME + "*3*";
    String upload = "cmd=2&uid=" + UID + "&topic=" + to + "&msg=" + chat + "\r\n";
    sendtoTCPServer(upload);
    // chat = "";
    getinfo = "";
}
void bafa::showchat() {
    display.firstPage();
    display.fillScreen(BackC);
    for (int i = 0; i < 2; i++) {
        display.drawRoundRect(0, i * 68, 296, 60, 4, FormC); //
    }
    nextPage();
    position[0] = getinfo.indexOf("*1*");
    position[1] = getinfo.indexOf("*2*");
    position[2] = getinfo.indexOf("*3*");

    for (int i = 0; i < 2; i++) {
        rmdlength[i] = position[i + 1] - position[i] - 2;
    }
    for (int i = 0; i < 2; i++) {
        reminder[i] = getinfo.substring(position[i] + 3, position[i + 1]);
        if (reminder[i].length() != 0)
            centerx(reminder[i], 30 + 68 * i, 1, 0, 296, "完全居中");
    }
    nextPage();
    getinfo = "";
}

void bafa::showpic(String pict) {
    int l = pict.length();
    if (l >= 9472) {
        display.firstPage();
        display.fillScreen(BackC);
        // centerx("图片获取成功！", 64);
        // nextPage();
        display.fillScreen(GxEPD_WHITE);
        String picture = "";
        for (int i = 0; i < 9472; i++) {
            picture += HexToBin(pict.charAt(i));
        }
        for (int y = 0; y < 128; y++) {
            for (int x = 0; x < 296; x++) {
                if (picture.charAt(x + y * 296) == '1') {
                    display.drawPixel(x, y, GxEPD_BLACK);
                } else if (picture.charAt(x + y * 296) == '0') {
                    display.drawPixel(x, y, GxEPD_WHITE);
                }
            }
        }
    }
    // nextPage();
    getinfo = "";
}


void getPic() {
    for (int i = 0; i < 5; i++) {
        string f1 = "/" + std::__cxx11::to_string(i) + ".txt";
        const char *f = f1.c_str();
        if (SPIFFS.exists(f) && MY.pic[i] == "") {
            Serial.println("从spiffs中获得图片");
            file[i] = SPIFFS.open(f, FILE_READ);
            MY.pic[i] = file[i].readString();
        }
    }
}


using std::string;
SPIClass SPIsd;
;
void start() {
    Wire.begin(21, 22); // GPIO 21（SDA）GPIO 22（SCL）  Wire.begin(SDA, SCL);
    if (aht20.begin() == false) {
        Serial.println("AHT20 not detected. Please check wiring. Freezing.");
        // while (1);
    }
    SPIsd.begin(18, 19, 23, 5);
    // SPI.end();                  // release standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
    // SPI.begin(13, 12, 14, 15);  // map and init SPI pins SCK(13), MISO(12), MOSI(14), SS(15)
    EEPROM.begin(4096); // 只读存储器初始化
    u8g2Fonts.setFont(chinese_gb2312);
    display.init(115200);
    u8g2Fonts.begin(display); // 将u8g2连接到display
    display.display(1);
    display.setRotation(1);
    display.firstPage();
    display.drawInvertedBitmap(0, 0, Bitmap_kaiji, 296, 128, GxEPD_BLACK); // 开机图标
    MY.showpic(MY.pic[0]);
    centerx("启动中...", 100, 2);
    nextPage();
    if (!SD.begin(SS, SPIsd)) {
        Serial.println("Card Mount Failed");
        centerx("SD卡接入失败", 64);
        nextPage();
        // return;
    }
    pinMode(button1, INPUT_PULLUP); // INPUT_PULLUP
    pinMode(button3, INPUT_PULLUP); // INPUT_PULLUP
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        Serial.println("SPIFFS Mount Failed");
        display.firstPage();
        centerx("文件系统挂载失败", 64, 1, 0, 296, "否");
        display.nextPage();
        return;
    }
    Serial.println("SPIFFS OK!");
    getPic();

    // display.firstPage();
    // centerx("文件系统挂载成功", 64, 1, 0, 296, "否");
    // display.nextPage();
    delay(100);
    for (int i; i < 40; i++)
        xiaoxi[i] = " ";
}

void checkbat() {
    centerx("检测电池adc...", 64);
    nextPage();
    if (getBatVolNew() == 100) {
        Serial.println(batpin);
        (batpin == 39) ? batpin = 34 : batpin = 39;
    }
    Serial.println(batpin);
    EEPROM.write(1110, batpin);
    EEPROM.commit();
}
void buttoncheck(int T) {
    gpio_wakeup_enable(GPIO_NUM_12, GPIO_INTR_HIGH_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    esp_sleep_enable_timer_wakeup(T * 60 * 1000000);
    esp_light_sleep_start();
    pinMode(button2, INPUT_PULLDOWN);
    if (digitalRead(button2) == HIGH) {
        pinMode(button2, OUTPUT);
        mod0setup();
    }


    pinMode(button2, OUTPUT);
}


void mod0setup() /*home初始化*/ {
    digitalWrite(LED, LOW);
    u8g2Fonts.setFont(chinese_gb2312);
    displaymod = 0;
    displaypage = 1;
    display.firstPage();
    display.fillScreen(BackC);
    /////////////////////////////////////////
    display.drawInvertedBitmap(66, 16, clockpic, 16, 16, FormC);
    display.drawInvertedBitmap(214, 16, Bitmap_img, 16, 16, FormC);
    display.drawInvertedBitmap(66, 80, Bitmap_read, 16, 16, FormC);
    display.drawInvertedBitmap(214, 80, Bitmap_next, 16, 16, FormC);
    // 打印出四个模式的图标////////////////////

    /////////////////////////////////////////
    u8g2Fonts.setForegroundColor(FormC); // 设置前景色
    u8g2Fonts.setBackgroundColor(BackC); // 设置背景色
    centerx("时钟", 50, 1, 0, 148);
    display.drawRoundRect(8, 8, 132, 48, 4, FormC);
    centerx("相册", 50, 1, 148, 148);
    centerx("阅读", 114, 1, 0, 148);
    centerx("下一页", 114, 1, 148, 148);
    // 打印出四个模式的文字//////////////////////

    ///////////////////////////////////////////////////////////////电量检测程序，还没做好
    /* String batVcc_s = String(uint8_t(getBatVolBfb(bat_vcc))) + "%";
    u8g2Fonts.setCursor(4, 113);
    u8g2Fonts.print(batVcc_s);*/
    ///////////////////////////////////////////////////////////////
    nextPage();
    displaymod0(); // 进入home
}
void mod0setup1() /*home初始化*/ {
    u8g2Fonts.setFont(chinese_gb2312);
    displaymod = 0;
    displaypage = 11;
    display.firstPage();
    display.fillScreen(BackC);
    /////////////////////////////////////////////////////////////////////
    display.drawInvertedBitmap(66, 16, Bitmap_msg, 16, 16, FormC);
    display.drawInvertedBitmap(214, 16, Bitmap_game, 16, 16, FormC);
    display.drawInvertedBitmap(66, 80, Bitmap_list, 16, 16, FormC);
    display.drawInvertedBitmap(214, 80, Bitmap_next, 16, 16, FormC);
    // 打印出四个模式的图标//////////////////////////////////////////////

    /////////////////////////////////////////
    u8g2Fonts.setForegroundColor(FormC); // 设置前景色
    u8g2Fonts.setBackgroundColor(BackC); // 设置背景色
    centerx("云信", 50, 1, 0, 148);
    display.drawRoundRect(8, 8, 132, 48, 4, FormC);
    centerx("娱乐", 50, 1, 148, 148);
    centerx("备忘录", 114, 1, 0, 148);
    centerx("下一页", 114, 1, 148, 148);
    // 打印出四个模式的文字//////////////////////

    ///////////////////////////////////////////////////////////////电量检测程序，还没做好
    /* String batVcc_s = String(uint8_t(getBatVolBfb(bat_vcc))) + "%";
    u8g2Fonts.setCursor(4, 113);
    u8g2Fonts.print(batVcc_s);*/
    ///////////////////////////////////////////////////////////////
    nextPage();
    displaymod0(); // 进入home
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void weatherclock() /*时钟模式*/ {
    centerx("校准时间中...", 50, 1);
    nextPage(10, 1);

    displaypage = 1;
    display.firstPage();
    display.fillScreen(BackC);
    int no = 0;
    do {
        // 天气
        int temperature = aht20.getTemperature();
        int humidity = aht20.getHumidity();
        if (WiFi.status() != WL_CONNECTED && no == 0) {
            centerx("未联网，正在尝试联网", 70, 1);
            nextPage(10, 1);
            connectToWiFi(5, 1);
            no += 1;
        }
        display.fillScreen(BackC);
        // display.drawInvertedBitmap(50, 0, Bitmap_GZ, 98, 60, FormC);
        display.drawRoundRect(0, 0, 195, 128, 20, FormC);
        u8g2Fonts.setFont(u8g2_font_courB24_tf);
        reqLocation = EEPROM.readString(300);
        centerx(reqLocation, 35, 1, 0, 195);
        displaypage = 1;
        display.fillRect(50, 90, 98, 14, FormC);
        if (WiFi.status() == WL_CONNECTED) /*如果有网络就联网时钟*/ {
            no = (no == 60) ? 0 : no + 1;

            getweather(1); // 见Webserver.h
            gettime111(80, 0, 198, "big"); // 时间
            time00 = millis(); // 在此时记录程序运行时间，与下一次检测运行时间作差来进行离线时间计算
            Serial.println("ok");
            WiFi.mode(WIFI_OFF);
        } else /*离线模式*/ {
            WiFi.mode(WIFI_OFF);
            no = (no == 60) ? 0 : no + 1;
            gettime0(); // 见Webserver.ino
            u8g2Fonts.setFont(u8g2_font_fub30_tf);
            u8g2Fonts.setForegroundColor(FormC); // 设置前景色
            u8g2Fonts.setBackgroundColor(BackC); // 设置背景色
            string h = (hour0 < 10) ? ("0" + std::__cxx11::to_string(hour0)) : (std::__cxx11::to_string(hour0));
            string m = (min0 < 10) ? ("0" + std::__cxx11::to_string(min0)) : (std::__cxx11::to_string(min0));
            string x = h + ":" + m; // 拼凑出离线时间
            const char *text = x.c_str(); // string转char才能被输出在屏幕
            centerx(text, 80, 1, 0, 198); // thetime1见gettime里的变量//打印出时间
            u8g2Fonts.setFont(chinese_gb2312);
            time00 = millis(); // 在此时记录程序运行时间，与下一次检测运行时间作差来进行离线时间计算

            display.drawRoundRect(198, 0, 98, 128, 20, FormC); // 画外框
            string theday /*星期x*/ = "2023-" + std::__cxx11::to_string(tm1.Month) + "-" +
                                      std::__cxx11::to_string(tm1.Day) + " " +
                                      dayOfWeekName[tm1.Wday - 1]; // 拼接"星期x"
            theday1 = theday.c_str();

            centerx(theday1, 100, 0, 0, 198);
            if (results_0_now_code_int1 == 4 || results_0_now_code_int1 == 5 || results_0_now_code_int1 == 6 ||
                results_0_now_code_int1 == 7 || results_0_now_code_int1 == 8) {
                display.drawInvertedBitmap(198, 0, duoyun, 98, 128, FormC); // 画图
            } else if (results_0_now_code_int1 == 0 || results_0_now_code_int1 == 1 || results_0_now_code_int1 == 2 ||
                       results_0_now_code_int1 == 3) {
                display.drawInvertedBitmap(198, 0, qing, 98, 128, FormC); // 画图
            } else if (results_0_now_code_int1 == 9) {
                display.drawInvertedBitmap(198, 0, yin, 98, 128, FormC); // 画图
            } else if (results_0_now_code_int1 == 10) {
                display.drawInvertedBitmap(198, 0, zhenyu, 98, 128, FormC); // 画图
            } else if (results_0_now_code_int1 == 11 || results_0_now_code_int1 == 12) {
                display.drawInvertedBitmap(198, 0, leizhenyu, 98, 128, FormC); // 画图
            } else if (results_0_now_code_int1 == 13) {
                display.drawInvertedBitmap(198, 0, xiaoyu, 98, 128, FormC); // 画图
            } else if (results_0_now_code_int1 == 14) {
                display.drawInvertedBitmap(198, 0, zhongyu, 98, 128, FormC); // 画图
            } else if (results_0_now_code_int1 == 15 || results_0_now_code_int1 == 16 ||
                       results_0_now_code_int1 == 17 || results_0_now_code_int1 == 18) {
                display.drawInvertedBitmap(198, 0, dayu, 98, 128, FormC); // 画图
            } else if (results_0_now_code_int1 == 30 || results_0_now_code_int1 == 31) {
                display.drawInvertedBitmap(198, 0, dawu, 98, 128, FormC); // 画图
            } else {
                display.drawInvertedBitmap(198, 0, nosignal, 98, 128, FormC);
            }
            display.drawRoundRect(198, 0, 98, 128, 20, FormC); // 画外框
            u8g2Fonts.setFont(chinese_gb2312);
            u8g2Fonts.setForegroundColor(FormC); // 设置前景色
            u8g2Fonts.setBackgroundColor(BackC); // 设置背景色
            u8g2Fonts.setCursor(45, 120);
            string text0 = std::to_string(results_0_now_temperature_int1);
            string text1 = "Temperature:" + text0 + "℃";
            const char *p = text1.data();
            u8g2Fonts.print(p);
            Serial.println("xxxxxx");
            Serial.println(p);
            Serial.println("xxxxxx");
        }
        if (temperature != -50 && humidity != 0) {
            display.drawInvertedBitmap(5, 68, tem, 16, 16, FormC);
            display.drawInvertedBitmap(173, 68, wet, 16, 16, FormC);
            centerx((std::__cxx11::to_string(temperature) + "℃").c_str(), 100, 1, 0, 40);
            centerx((std::__cxx11::to_string(humidity) + "%").c_str(), 100, 1, 160, 40);
        }
        nextPage(10, 1);
        buttoncheck(1);

    } while (displaymod == 1 && displaypage == 1);
}

// 一言
#include <ArduinoJson.h>
#include <HTTPClient.h>
String quote = "";
void get_Hitokoto(void) {

    if (WiFi.status() == WL_CONNECTED) /*如果有网络就联网*/ {
        HTTPClient http;
        http.begin("https://v1.hitokoto.cn/?encode=json&min_length=1&max_length=15"); // Specify the URL
        int httpCode = http.GET(); // Make the request
        if (httpCode > 0) { // Check for the returning code
            quote = http.getString();
            Serial.println(httpCode);
            Serial.println(quote);
        } else {
            Serial.println("Error on HTTP request");
        }

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, quote);
        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return;
        }
        const char *quote1 = doc["hitokoto"]; // "奇怪的是，当他死去，所有人才开始爱他。"
        Serial.println("1");
        quote = quote1;
        Serial.println("2");
        http.end(); // Free the resources
        if (quote != "") {
            EEPROM.writeString(900, quote);
            EEPROM.commit();
        }
    }

    Serial.println(quote);
    quote = EEPROM.readString(900);
}
void get_History(void) {
    if (WiFi.status() == WL_CONNECTED) /*如果有网络就联网*/ {
        HTTPClient http;
        http.begin("https://zj.v.api.aa1.cn/api/bk/?num=5&type=json"); // Specify the URL
        int httpCode = http.GET(); // Make the request
        if (httpCode > 0) { // Check for the returning code
            quote = http.getString();
            Serial.println(httpCode);
            Serial.println(quote);
        } else {
            Serial.println("Error on HTTP request");
        }

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, quote);
        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return;
        }
        int numm = -5 + rand() % 11;
        const char *quote1 = doc["content"][numm]; // "奇怪的是，当他死去，所有人才开始爱他。"
        Serial.println("1");
        Serial.println(quote1);
        quote = quote1;
        Serial.println("2");
        http.end(); // Free the resources
        if (quote != "") {
            EEPROM.writeString(900, quote);
            EEPROM.commit();
        }
    }
    Serial.println(quote);
    quote = EEPROM.readString(900);
}
void quoteclock() {
    int no = 0;
    int Case = 1;
    Case = atoi(EEPROM.readString(990).c_str());
    displaypage = 3;
    while (displaymod == 1 && displaypage == 3) {

        if (WiFi.status() != WL_CONNECTED && no == 0) {
            centerx("未联网，正在尝试联网", 70, 1);
            nextPage(10, 1);
            connectToWiFi(5, 1);
            no += 1;
        }

        display.firstPage();
        if (Case == 0) {
            display.drawInvertedBitmap(0, 0, quoteclock1, 296, 128, GxEPD_BLACK);
            display.fillRoundRect(8, 105, 280, 28, 5, GxEPD_WHITE);
            display.drawRoundRect(8, 105, 280, 28, 5, GxEPD_BLACK);
            display.drawInvertedBitmap(15, 112, Bitmap_read, 16, 16, GxEPD_BLACK);
        }
        quotemod = atoi(EEPROM.readString(980).c_str());
        if (quotemod == 0)
            get_Hitokoto();
        if (quotemod == 1)
            get_History();

        no = (no == 60) ? 0 : no + 1;
        if (WiFi.status() == WL_CONNECTED) {
            getweather(2);
            gettime111(78, 37, 216, "huge"); // 时间
        } else {
            gettime0(); // 见Webserver.ino
            u8g2Fonts.setFont(u8g2_font_logisoso62_tn);
            u8g2Fonts.setForegroundColor(GxEPD_BLACK); // 设置前景色
            // u8g2Fonts.setBackgroundColor(BackC);  // 设置背景色
            string h = (hour0 < 10) ? ("0" + std::__cxx11::to_string(hour0)) : (std::__cxx11::to_string(hour0));
            string m = (min0 < 10) ? ("0" + std::__cxx11::to_string(min0)) : (std::__cxx11::to_string(min0));
            string x = h + ":" + m; // 拼凑出离线时间
            const char *text = x.c_str(); // string转char才能被输出在屏幕
            centerx_B(text, 78, 0, 37, 216); // thetime1见gettime里的变量//打印出时间
            u8g2Fonts.setFont(chinese_gb2312);
            time00 = millis();
            string today = ""; // 在此时记录程序运行时间，与下一次检测运行时间作差来进行离线时间计算 //画外框
            today /*星期x*/ = "2023-" + std::__cxx11::to_string(tm1.Month) + "-" + std::__cxx11::to_string(tm1.Day) +
                              " " + dayOfWeekName[tm1.Wday - 1]; // 拼接"星期x"
            const char *today1 = today.c_str();
            centerx_B(today1, 78 + 18, 1, 41, 216);
        }
        u8g2Fonts.setForegroundColor(GxEPD_BLACK); // 设置前景色
        u8g2Fonts.setCursor(40, 125); // 文本居中
        const char *quotechar = quote.c_str();
        u8g2Fonts.print(quotechar);
        int temperature = aht20.getTemperature();
        int humidity = aht20.getHumidity();
        if (temperature != -50 && humidity != 0) {
            display.drawInvertedBitmap(20, 48, tem, 16, 16, FormC);
            display.drawInvertedBitmap(256, 48, wet, 16, 16, FormC);
            centerx_B((std::__cxx11::to_string(temperature) + "℃").c_str(), 80, 1, 12, 40);
            centerx_B((std::__cxx11::to_string(humidity) + "%").c_str(), 80, 1, 245, 40);
        }
        nextPage(10, 1);
        WiFi.mode(WIFI_OFF);
        gpio_wakeup_enable(GPIO_NUM_0, GPIO_INTR_LOW_LEVEL);
        buttoncheck(1);
        if (digitalRead(button3) == LOW && Case == 0) {
            EEPROM.writeString(990, "1");
            Case = 1;
            EEPROM.commit();
        } else if (digitalRead(button3) == LOW && Case == 1) {
            EEPROM.writeString(990, "0");
            Case = 0;
            EEPROM.commit();
        }
    }
}
void time1(int y = 100, int x0 = 0, int dx = 100) {
    RTC.read(tm1);
    Serial.println("rtcread");
    if (WiFi.status() == WL_CONNECTED) /*如果有网络就联网时钟*/ {
        getweather(2);
        gettime111(y, x0, dx, "small"); // 时间
        time00 = millis(); // 在此时记录程序运行时间，与下一次检测运行时间作差来进行离线时间计算
        Serial.println("ok");
    } else /*离线模式*/ {
        gettime0(); // 见Webserver.ino
        Serial.println("rtcread1");
        u8g2Fonts.setFont(u8g2_font_helvB24_tf);
        u8g2Fonts.setForegroundColor(FormC); // 设置前景色
        u8g2Fonts.setBackgroundColor(BackC); // 设置背景色
        string h = (hour0 < 10) ? ("0" + std::__cxx11::to_string(hour0)) : (std::__cxx11::to_string(hour0));
        string m = (min0 < 10) ? ("0" + std::__cxx11::to_string(min0)) : (std::__cxx11::to_string(min0));
        string x = h + ":" + m; // 拼凑出离线时间
        const char *text = x.c_str(); // string转char才能被输出在屏幕
        centerx(text, y, 1, x0, 100); // thetime1见gettime里的变量//打印出时间
        Serial.println("printedthetime1");
        u8g2Fonts.setFont(chinese_gb2312);
        time00 = millis(); // 在此时记录程序运行时间，与下一次检测运行时间作差来进行离线时间计算
        string theday /*星期x*/ = "2023-" + std::__cxx11::to_string(tm1.Month) + "-" +
                                  std::__cxx11::to_string(tm1.Day) + " " + dayOfWeekName[tm1.Wday - 1]; // 拼接"星期x"
        theday1 = theday.c_str();
        Serial.println("printedtheday1");
        centerx(theday1, y + 20, 1, x0, 100);
    }
    // nextPage(10, 1);
}

void showpicloop(int);
void displaymod1() {
    displaymod = 1;
    displaypage = 1;
    display.fillScreen(BackC);
    centerx("天气时钟", 60, 1, 40, 48);
    centerx("图册时钟", 60, 1, 40 + 56, 48);
    centerx("一言时钟", 60, 1, 40 + 56 * 2, 48);
    centerx("返回", 70, 1, 40 + 56 * 3, 48);
    display.drawRoundRect(40 + 56 * (displaypage - 1), 40, 48, 48, 4, FormC);
    nextPage(10, 1);
    while (1) {
        pinMode(button2, INPUT_PULLDOWN);
        if (digitalRead(button2) == HIGH) {
            pinMode(button2, OUTPUT);
            if (displaypage == 1) {
                weatherclock();
                break;
            }
            if (displaypage == 2) {
                showpicloop(1);
                break;
            }
            if (displaypage == 3) {
                quoteclock();
                break;
            }
            if (displaypage == 4) {
                mod0setup();
                break;
            }
        }
        autostart = EEPROM.readString(800);

        if (digitalRead(button3) == LOW && displaypage == 1 && autostart != "weatherclockon") {
            centerx("已将天气时钟设置为开机启动", 22);
            nextPage(10);
            autostart = "weatherclockon";
            EEPROM.writeString(800, autostart);
            EEPROM.commit();
            delay(200);
            autostart = EEPROM.readString(800);
            display.fillRoundRect(0, 8, 296, 25, 2, BackC);
            Serial.println(autostart);
            nextPage(10);
        } else if (digitalRead(button3) == LOW && displaypage == 1 && autostart == "weatherclockon") {
            centerx("已将天气时钟开机启动已关闭", 22);
            autostart = "weatherclockoff";
            nextPage(10);
            EEPROM.writeString(800, autostart);
            EEPROM.commit();
            autostart = EEPROM.readString(800);
            delay(200);
            display.fillRoundRect(0, 8, 296, 25, 2, BackC);
            nextPage(10);
            Serial.println(autostart);
        }

        if (digitalRead(button3) == LOW && displaypage == 2 && autostart != "picclockon") {
            centerx("已将图册时钟设置为开机启动", 22);
            nextPage(10);
            autostart = "picclockon";
            EEPROM.writeString(800, autostart);
            EEPROM.commit();
            delay(200);
            autostart = EEPROM.readString(800);
            display.fillRoundRect(0, 8, 296, 25, 2, BackC);
            Serial.println(autostart);
            nextPage(10);
        } else if (digitalRead(button3) == LOW && displaypage == 2 && autostart == "picclockon") {
            centerx("已将图册时钟开机启动已关闭", 22);
            autostart = "picclockoff";
            nextPage(10);
            EEPROM.writeString(800, autostart);
            EEPROM.commit();
            autostart = EEPROM.readString(800);
            delay(200);
            display.fillRoundRect(0, 8, 296, 25, 2, BackC);
            nextPage(10);
            Serial.println(autostart);
        }

        if (digitalRead(button3) == LOW && displaypage == 3 && autostart != "quoteclockon") {
            centerx("已将一言时钟设置为开机启动", 22);
            nextPage(10);
            autostart = "quoteclockon";
            EEPROM.writeString(800, autostart);
            EEPROM.commit();
            delay(200);
            autostart = EEPROM.readString(800);
            display.fillRoundRect(0, 8, 296, 25, 2, BackC);
            Serial.println(autostart);
            nextPage(10);
        } else if (digitalRead(button3) == LOW && displaypage == 3 && autostart == "quoteclockon") {
            centerx("已将一言时钟开机启动已关闭", 22);
            autostart = "quoteclockoff";
            nextPage(10);
            EEPROM.writeString(800, autostart);
            EEPROM.commit();
            autostart = EEPROM.readString(800);
            delay(200);
            display.fillRoundRect(0, 8, 296, 25, 2, BackC);
            nextPage(10);
            Serial.println(autostart);
        }
        if (digitalRead(button1) == LOW) {
            pinMode(button2, OUTPUT);
            display.drawRoundRect(40 + 56 * (displaypage - 1), 40, 48, 48, 4, BackC);
            displaypage = (displaypage == 4) ? 1 : displaypage + 1;
            display.drawRoundRect(40 + 56 * (displaypage - 1), 40, 48, 48, 4, FormC);
            nextPage(10, 1);
        }
    }
    // mod0setup();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/*void displaymod2() {

  display.firstPage();
  display.fillScreen(BackC);
  centerx("读取消息中...", 50, 1);
  nextPage();
  getweather(1);  //借用心知天气api里的日期
  getmsg();       //见Webserver.h

  Date1 = Date1.substring(0, 10);  //取date的前10个字符（就是完整日期了）date在getweather()中有赋值
  display.fillScreen(BackC);
  u8g2Fonts.setFont(chinese_gb2312);
  u8g2Fonts.setForegroundColor(FormC);  // 设置前景色
  u8g2Fonts.setBackgroundColor(BackC);  // 设置背景色
  int n = 0;
  int b = 0;  //用于计次分行截取文本
  while (1) {
    display.firstPage();
    display.fillScreen(BackC);
    display.drawRoundRect(0, 0, 180, 128, 20, FormC);
    if (n <= 27) {
      for (int i = 0; i < 7; i++)  {//每7行截取一次
        u8g2Fonts.setCursor(60, 16 * i);
        u8g2Fonts.print(xiaoxi[b]);
        b += 1;
      }
    } else break;
    display.fillRect(180, 0, 116, 128, BackC);
    display.drawInvertedBitmap(185, 0, xiaoxi1, 116, 128, FormC);
    u8g2Fonts.setCursor(205, 100);
    u8g2Fonts.print(Date1);
    display.drawRoundRect(185, 0, 109, 128, 20, FormC);
    nextPage();
    buttoncheck(1, 1);
    n += 7;
  }
  mod0setup();
}*/
//////////////////////////////////////////////////////////////////////////////////////////////////////
void rmdmod() {
    MY.msgTOPIC = EEPROM.readString(400);
    MY.picTOPIC = EEPROM.readString(400);
    displaymod = 13;
    displaypage = 1;
    MY.msgfirsttime = true;
    display.firstPage();
    display.fillScreen(BackC);
    centerx("备忘录载入中...", 64);
    nextPage();
    if (WiFi.status() == WL_CONNECTED) {
        while (1) {
            MY.doWiFiTick();
            MY.info(MY.msgTOPIC, MY.msgfirsttime, 1);
            pinMode(button2, INPUT_PULLDOWN); // 将该按钮引脚设为输入，才能进行状态检测
            if (digitalRead(button2) == HIGH)
                break;
            pinMode(button2, OUTPUT);
        }
        pinMode(button2, OUTPUT);
    } else {
        display.firstPage();
        display.fillScreen(BackC);
        centerx("联网失败，离线模式将显示上次保存的备忘录...", 64);
        nextPage();
        String remd = EEPROM.readString(600);
        MY.showrmd(remd);
        while (1) {
            if (digitalRead(button2) == HIGH)
                break;
            pinMode(button2, OUTPUT);
        }
    }
    mod0setup1();
}
////////////////////////////////////////////////////////////////////////
void showpicloop(int mod = 0) {
    getPic();
    int T;
    bool firsttime[5] = {true, true, true, true, true};
    displaypage = 1;
    display.firstPage();
    display.fillScreen(BackC);
    if (mod == 0 || mod == 1) {
        centerx("请输入翻页间隔：", 64, 1, 0, 148);
        display.fillRoundRect(168, 0, 128, 128, 10, BackC);
        display.drawInvertedBitmap(168, 0, nkb, 128, 128, FormC);
        nextPage(50, 1);
        String t = "";
        T = nkeyb();
    } else if (mod == 2) {
        T = 1;
    }

    while (1) {

        if (MY.pic[displaypage - 1] == "") {
            if (firsttime[displaypage - 1] == true) {
                display.firstPage();
                display.fillScreen(BackC);
                // nextPage(10, 1);
                firsttime[displaypage - 1] = false;
                if (mod != 0) {
                    time1();
                }
                nextPage(10, 1);
            }
        } else {
            if (firsttime[displaypage - 1] == true) {
                display.firstPage();
                display.fillScreen(BackC);
                MY.showpic(MY.pic[displaypage - 1]);
                firsttime[displaypage - 1] = false;
                if (mod != 0) {
                    display.fillRoundRect(-10, 72, 105, 32, 5, BackC);
                    display.fillRoundRect(-10, 108, 105, 14, 5, BackC);
                    // nextPage(10, 1);
                    time1();
                }
                nextPage(10, 1);
                pinMode(button3, INPUT_PULLUP);
            }
        }
        if (displaypage == 5) {
            for (int i = 0; i < 5; i++)
                firsttime[i] = true;
        }
        displaypage = (displaypage == 5) ? 1 : displaypage + 1;
        gpio_wakeup_enable(GPIO_NUM_12, GPIO_INTR_HIGH_LEVEL);
        esp_sleep_enable_gpio_wakeup();
        esp_sleep_enable_timer_wakeup(T * 60 * 1000000);
        esp_light_sleep_start();
        pinMode(button2, INPUT_PULLDOWN);
        if (digitalRead(button2) == HIGH) {
            pinMode(button2, OUTPUT);
            mod0setup();
            break;
        }

        pinMode(button2, OUTPUT);
    }
}

///////////////////////////////////////////////////////////////////////
void displaymod11() {
    displaymod = 2;
    displaypage = 1;
    MY.picfirsttime = false;
    for (int i = 0; i < 5; i++) {
        string f1 = "/" + std::__cxx11::to_string(i) + ".txt";
        const char *f = f1.c_str();
        if (SPIFFS.exists(f) && MY.pic[i] == "") {
            Serial.println("从spiffs中获得图片");
            file[i] = SPIFFS.open(f, FILE_READ);
            MY.pic[i] = file[i].readString();
        }
    }
    bool firsttime[6] = {true, true, true, true, true, true};
    display.firstPage();
    display.fillScreen(BackC);
    nextPage();
    while (1) {
        if (displaypage == 6) {
            if (firsttime[5] == true) {
                display.firstPage();
                display.fillScreen(BackC);
                centerx("自动播放请按左一按钮进入", 64);
                nextPage();
                firsttime[5] = false;
            }

            if (digitalRead(button2) == HIGH) {
                showpicloop();
                break;
            }

        } else {
            if (MY.pic[displaypage - 1].length() == 0) {
                if (firsttime[displaypage - 1] == true) {
                    display.firstPage();
                    display.fillScreen(BackC);
                    string p = "第" + std::__cxx11::to_string(displaypage) + "页";
                    const char *page = p.c_str();
                    centerx(page, 54);
                    centerx("请上传图片(1)", 70);
                    nextPage();
                    firsttime[displaypage - 1] = false;
                }
                MY.doWiFiTick();
                MY.info(MY.picTOPIC, MY.picfirsttime, 2, displaypage - 1, 0);
                // continue;
            } else if (MY.pic[displaypage - 1].length() == 4736) {
                if (firsttime[displaypage - 1] == true) {
                    display.firstPage();
                    display.fillScreen(BackC);
                    string p = "第" + std::__cxx11::to_string(displaypage) + "页";
                    const char *page = p.c_str();
                    centerx(page, 54);
                    centerx("请上传图片(2)", 90);
                    nextPage();
                    firsttime[displaypage - 1] = false;
                }
                MY.doWiFiTick();
                MY.info(MY.picTOPIC, MY.picfirsttime, 2, displaypage - 1, 1);
            } else {
                if (firsttime[displaypage - 1] == true) {
                    display.firstPage();
                    display.fillScreen(BackC);
                    MY.showpic(MY.pic[displaypage - 1]);
                    nextPage(10, 1);
                    firsttime[displaypage - 1] = false;
                    pinMode(button3, INPUT_PULLUP);
                }
                if (digitalRead(button3) == LOW) {
                    delay(10);
                    if (digitalRead(button3) == LOW) {
                        display.firstPage();
                        string ff = "/" + std::__cxx11::to_string(displaypage - 1) + ".txt";
                        const char *f = ff.c_str();
                        if (SPIFFS.remove(f)) {
                            Serial.println("- file deleted");
                            centerx("已删除", 64);
                            nextPage();
                            firsttime[displaypage - 1] = true;
                        } else {
                            Serial.println("- delete failed");
                            centerx("删除失败", 64);
                            nextPage();
                        }
                        MY.pic[displaypage - 1] = "";
                        continue;
                    }
                }
            }
            pinMode(button3, INPUT_PULLUP);
            pinMode(button2, INPUT_PULLDOWN); // 将该按钮引脚设为输入，才能进行状态检测
            if (digitalRead(button1) == LOW) {
                displaypage = (displaypage == 6) ? 1 : displaypage + 1;
                if (displaypage == 6) {
                    for (int i = 0; i < 5; i++)
                        firsttime[i] = true;
                }
            }
            if (digitalRead(button2) == HIGH)
                break;
            pinMode(button2, OUTPUT);
        }
    }
    pinMode(button2, OUTPUT);
    mod0setup();
}
//////////////////////////////////////////////////////////////////////////
void kbl(int &i, int &j) {
    i -= 1;
    if (i == -1) {
        i = 9;
        j -= 1;
    }
    if (j == -1) {
        j = 3;
    }
}
void kbr(int &i, int &j) {
    i += 1;
    if (i == 10) {
        i = 0;
        j += 1;
    }
    if (j == 4) {
        j = 0;
    }
}
void kbd(int &i, int &j) {
    j += 1;
    if (j == 4)
        j = 0;
}
void keyb(String &textb) { // 键盘
    textb = "";
    display.firstPage();
    display.fillScreen(BackC);
    display.drawInvertedBitmap(0, 0, keyboard, 296, 128, FormC);
    String say = "";
    String keyvalue = "abcdefghijklmnopqrstuvwxyz1234567890. ";
    int i = 0;
    int j = 0;
    int num = 0;
    int len = 0;
    display.drawRoundRect(63 + 17 * i, 64 + 16 * j, 17, 16, 2, FormC);
    nextPage(50);
    while (1) {
        display.fillRect(16, 7, 264, 40, BackC);
        centerx(textb, 20, 1, 16, 264);
        espsleep();
        pinMode(button2, INPUT_PULLDOWN);
        pinMode(button3, INPUT_PULLUP);
        if (digitalRead(button2) == HIGH && digitalRead(button1) == HIGH) {
            pinMode(button2, OUTPUT);
            // pinMode(button3, OUTPUT);
            display.drawRoundRect(63 + 17 * i, 64 + 16 * j, 17, 16, 2, BackC);
            kbr(i, j);
            display.drawRoundRect(63 + 17 * i, 64 + 16 * j, 17, 16, 2, FormC);
            nextPage(50);
        }
        if (digitalRead(button1) == LOW && digitalRead(button2) == LOW) {
            pinMode(button2, OUTPUT);
            //(button3, OUTPUT);
            display.drawRoundRect(63 + 17 * i, 64 + 16 * j, 17, 16, 2, BackC);
            kbl(i, j);
            display.drawRoundRect(63 + 17 * i, 64 + 16 * j, 17, 16, 2, FormC);
            nextPage(50);
        }
        if (digitalRead(button2) == HIGH && digitalRead(button1) == LOW) {
            pinMode(button2, OUTPUT);
            // pinMode(button3, OUTPUT);
            display.drawRoundRect(63 + 17 * i, 64 + 16 * j, 17, 16, 2, BackC);
            kbd(i, j);
            display.drawRoundRect(63 + 17 * i, 64 + 16 * j, 17, 16, 2, FormC);
            nextPage(50);
        }
        num = i + 10 * j;
        // if (digitalRead(button2) == LOW && digitalRead(button1) == HIGH && digitalRead(button3) == HIGH) {
        // delay(20);
        if (/*digitalRead(button2) == LOW && digitalRead(button1) == HIGH && */ digitalRead(button3) == LOW &&
            num < 38) {
            textb += keyvalue[num];
            display.fillRect(16, 7, 264, 40, BackC);
            centerx(textb, 20, 1, 16, 264);
            nextPage(50);
        } else if (/*digitalRead(button2) == LOW && digitalRead(button1) == HIGH && */ digitalRead(button3) == LOW &&
                   num == 39) {
            display.fillScreen(BackC);
            centerx("键入完成", 64);
            nextPage();
            delay(50);
            break;
        } else if (/*digitalRead(button2) == LOW && digitalRead(button1) == HIGH && */ digitalRead(button3) == LOW &&
                   num == 38) {
            textb = textb.substring(0, textb.length() - 1);
            display.fillRect(16, 7, 264, 40, BackC);
            centerx(textb, 20, 1, 16, 264);
            nextPage(50);
        }
        //}
    }
}
///////////////////////////////////////////////////////////////////////////
void putchat() {
    MY.msgTOPIC = EEPROM.readString(400);
    MY.picTOPIC = EEPROM.readString(400);
    MY.chatTOPIC = EEPROM.readString(500);
    MY.chat = "";
    displaymod = 11;
    displaypage = 1;
    String towhom = "";
    display.firstPage();
    display.fillScreen(BackC);
    centerx("请键入发送对象", 64);
    nextPage();
    keyb(towhom);
    centerx("请键入发送内容", 64);
    nextPage();
    keyb(MY.chat);
    if (WiFi.status() == WL_CONNECTED) {
        MY.msgfirsttime = true;
        display.fillScreen(BackC);
        String from = EEPROM.readString(500);
        MY.putchat(from, towhom, MY.chat);
        centerx("已发送", 64);
        nextPage();
        delay(50);
    } else {
        display.fillScreen(BackC);
        centerx("网络已断开", 64);
        nextPage();
        delay(50);
    }
    mod0setup1();
}
///////////////////////////////////////////////////////////////////////////
void shouchats() {
    MY.msgTOPIC = EEPROM.readString(400);
    MY.picTOPIC = EEPROM.readString(400);
    MY.chatTOPIC = EEPROM.readString(500);
    MY.msgfirsttime = true;
    display.firstPage();
    display.fillScreen(BackC);
    centerx("接收消息中...", 64);
    nextPage();
    while (1) {
        MY.doWiFiTick();
        MY.info(MY.chatTOPIC, MY.msgfirsttime, 3);
        pinMode(button2, INPUT_PULLDOWN); // 将该按钮引脚设为输入，才能进行状态检测
        if (digitalRead(button2) == HIGH)
            break;
        pinMode(button2, OUTPUT);
    }
    pinMode(button2, OUTPUT);
    mod0setup1();
}
void displaymod111() {
    displaymod = 11;
    displaypage = 1;
    display.fillScreen(BackC);
    centerx("信箱", 70, 1, 78, 48);
    centerx("发送", 70, 1, 78 + 56, 48);
    centerx("返回", 70, 1, 78 + 56 * 2, 48);
    display.drawRoundRect(78 + 56 * (displaypage - 1), 40, 48, 48, 4, FormC);
    nextPage();
    while (1) {
        pinMode(button2, INPUT_PULLDOWN);
        if (digitalRead(button2) == HIGH) {
            pinMode(button2, OUTPUT);
            if (displaypage == 1) {
                shouchats();
                break;
            }
            if (displaypage == 2) {
                putchat();
                break;
            }
            if (displaypage == 3) {
                mod0setup1();
                break;
            }
        }
        if (digitalRead(button1) == LOW) {
            pinMode(button2, OUTPUT);
            display.drawRoundRect(78 + 56 * (displaypage - 1), 40, 48, 48, 4, BackC);
            displaypage = (displaypage == 3) ? 1 : displaypage + 1;
            display.drawRoundRect(78 + 56 * (displaypage - 1), 40, 48, 48, 4, FormC);
            nextPage();
        }
    }
    mod0setup1();
}
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
int gamedesk() {
    display.fillRoundRect(48, 2, 200, 64, 10, BackC);
    display.drawRoundRect(48, 2, 200, 64, 10, FormC);
    centerx("继续游戏", 35, 1, 53, 30);
    centerx("重置记录", 35, 1, 53 + 40, 30);
    centerx("退出游戏", 35, 1, 53 + 40 * 2, 30);
    display.fillRoundRect(53, 52, 34, 5, 2, FormC);
    time1(33, 155);
    nextPage(10, 1);

    while (1) {
        Serial.println("kkkkkk");
        espsleep();
        if (digitalRead(button1) == LOW) {
            Gamemode = (Gamemode == 2) ? 0 : Gamemode + 1;
            display.fillRoundRect(53 + 40 * ((Gamemode == 0) ? 2 : (Gamemode - 1)), 52, 34, 5, 2, BackC); // 画外框
            display.fillRoundRect(53 + 40 * Gamemode, 52, 34, 5, 2, FormC);
            nextPage(10, 1);
        }
        if (digitalRead(button2) == HIGH) {
            if (Gamemode == 0)
                return -1;
            if (Gamemode == 1) {
                formatffsshow();
                longpressbtn();
                break;
            }
            if (Gamemode == 2)
                return -2;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void displaymod13(int desk, int batin) {
    displaymod = 13;
    displaypage = 1;
    display.firstPage();
    display.fillScreen(BackC);

    centerx("设置", 70, 1, 40, 48);
    centerx("配网", 70, 1, 40 + 56, 48);
    centerx("深/浅", 70, 1, 40 + 56 * 2, 48);
    centerx("返回", 70, 1, 40 + 56 * 3, 48);
    display.drawRoundRect(40 + 56 * (displaypage - 1), 40, 48, 48, 4, FormC);
    nextPage();
    while (1) {
        pinMode(button3, INPUT_PULLUP);
        espsleep();
        // if (digitalRead(button1) == LOW&&digitalRead(button3) == LOW&&digitalRead(button2) == HIGH){
        //   batpin=(batpin==34)?39:34;
        //   EEPROM.write(1110, batpin);EEPROM.commit();centerx("batpin changed",60,1,0,296);nextPage();
        // }
        if (digitalRead(button3) == LOW) {
            pinMode(button2, OUTPUT);
            display.firstPage();
            centerx("进入休眠", 64);
            MY.showpic(MY.pic[0]);
            // display.drawInvertedBitmap(0, 0, Bitmap_kaiji, 296, 128, GxEPD_BLACK);
            nextPage(10, 1);
            esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
        }
        // while (1) {
        pinMode(button2, INPUT_PULLDOWN);

        if (digitalRead(button2) == HIGH) {
            pinMode(button2, OUTPUT);
            if (displaypage == 1) {
                SettingStart();
                break;
            }
            if (displaypage == 2) {
                PeiWangStart();
                break;
            }
            if (displaypage == 3) {
                theme = (theme == 1) ? 0 : 1;
                displaypage = 3;
                display.firstPage();
                display.fillScreen(BackC);
                centerx("设置", 70, 1, 40, 48);
                centerx("配网", 70, 1, 40 + 56, 48);
                centerx("深/浅", 70, 1, 40 + 56 * 2, 48);
                centerx("返回", 70, 1, 40 + 56 * 3, 48);
                display.drawRoundRect(40 + 56 * 2, 40, 48, 48, 4, FormC);
                nextPage();
            }
            if (displaypage == 4) {
                if (desk == 0) {
                    mod0setup();
                    break;
                } else if (desk == 1) {
                    mod0setup1();
                    break;
                }

            } // OTAStart();
        }
        if (digitalRead(button1) == LOW) {
            pinMode(button2, OUTPUT);
            display.drawRoundRect(40 + 56 * (displaypage - 1), 40, 48, 48, 4, BackC);
            displaypage = (displaypage == 4) ? 1 : displaypage + 1;
            display.drawRoundRect(40 + 56 * (displaypage - 1), 40, 48, 48, 4, FormC);
            nextPage();
        }
    }
    // mod0setup1();
}
void displaymod0() /*选择模式*/ {
    // espsleep();
    pinMode(button3, INPUT_PULLUP);
    pinMode(button2, INPUT_PULLDOWN); // 将该按钮引脚设为输入，才能进行状态检测
    // 中间按钮(button2)
    int desk = (displaypage <= 10) ? 0 : 1;
    if (digitalRead(button3) == LOW) {
        pinMode(button2, OUTPUT);
        displaymod13(desk, batpin);
    }
    if (digitalRead(button2) == HIGH) {
        switch (displaypage) {
            case 1:
                pinMode(button3, INPUT_PULLUP);
                pinMode(button2, OUTPUT); // 检测完后将其变为输出，才能使屏幕正常运行，因为它和屏幕公用了一个引脚
                displaymod = 1;
                displaypage = 1;
                displaymod1();
                break;
            case 2:
                pinMode(button3, INPUT_PULLUP);
                pinMode(button2, OUTPUT); // 检测完后将其变为输出，才能使屏幕正常运行，因为它和屏幕公用了一个引脚
                displaymod = 2;
                displaypage = 1;
                displaymod11();
                break;
            case 3:
                pinMode(button3, INPUT_PULLUP);
                if (!SD.begin(SS, SPIsd)) {
                    Serial.println("Card Mount Failed");
                    display.firstPage();
                    centerx("SD卡接入失败", 64);
                    nextPage();
                    mod0setup1();
                    return;
                } else {
                    txtmod("/", 0);
                }


                break;
            case 4:
                pinMode(button3, INPUT_PULLUP);
                pinMode(button2, OUTPUT); // 检测完后将其变为输出，才能使屏幕正常运行，因为它和屏幕公用了一个引脚
                mod0setup1();
                break;
            case 11:
                pinMode(button3, INPUT_PULLUP);
                pinMode(button2, OUTPUT); // 检测完后将其变为输出，才能使屏幕正常运行，因为它和屏幕公用了一个引脚
                displaymod111(); // 云信
                break;
            case 12:
                // 初始化看门狗
                pinMode(button3, INPUT_PULLUP);
                pinMode(button2, OUTPUT); // 检测完后将其变为输出，才能使屏幕正常运行，因为它和屏幕公用了一个引脚
                GAMEsetup();
                gameloop();
                mod0setup1();
                break;
            case 13:
                pinMode(button3, INPUT_PULLUP);
                pinMode(button2, OUTPUT); // 检测完后将其变为输出，才能使屏幕正常运行，因为它和屏幕公用了一个引脚
                displaymod = 13;
                displaypage = 1;
                Serial.begin(115200);

                Serial.println("Beginning...");

                rmdmod();

                break;
            case 14:
                pinMode(button3, INPUT_PULLUP);
                pinMode(button2, OUTPUT); // 检测完后将其变为输出，才能使屏幕正常运行，因为它和屏幕公用了一个引脚
                mod0setup();
                break;
        }
    }
    // 右侧按钮（button1)，没有公用引脚问题
    // 功能是按一次按钮，将“选定框”移动一格，并声明“页面模式”
    if (digitalRead(button1) == LOW && displaymod == 0 && (displaypage == 1 || displaypage == 11)) {
        pinMode(button2, OUTPUT);
        pinMode(button3, INPUT_PULLUP);
        display.drawRoundRect(8, 8, 132, 48, 4, BackC); // 用白色在上次位置覆盖掉上一次画出黑色选择框
        display.drawRoundRect(156, 8, 132, 48, 4, FormC); // 画出黑色选择框
        displaypage += 1;
        nextPage();
    } else if (digitalRead(button1) == LOW && displaymod == 0 && (displaypage == 2 || displaypage == 12)) {
        pinMode(button2, OUTPUT);
        display.drawRoundRect(156, 8, 132, 48, 4, BackC);
        display.drawRoundRect(8, 72, 132, 48, 4, FormC);
        displaypage += 1;
        nextPage();
    } else if (digitalRead(button1) == LOW && displaymod == 0 && (displaypage == 3 || displaypage == 13)) {
        pinMode(button2, OUTPUT);
        display.drawRoundRect(8, 72, 132, 48, 4, BackC);
        display.drawRoundRect(156, 72, 132, 48, 4, FormC);
        displaypage += 1;
        nextPage();
    } else if (digitalRead(button1) == LOW && displaymod == 0 && (displaypage == 4 || displaypage == 14)) {
        pinMode(button2, OUTPUT);
        display.drawRoundRect(156, 72, 132, 48, 4, BackC);
        display.drawRoundRect(8, 8, 132, 48, 4, FormC);
        displaypage -= 3;
        nextPage();
    }
}
