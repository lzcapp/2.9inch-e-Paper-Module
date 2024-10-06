#include <WiFi.h>
#include <WebServer.h>
#include <Update.h> // 引用相关库

const char *ssid = "ChinaNect-kmtr";
const char *password = "ztfnmgk5";

WebServer server(80);

// 以下为网页文件
String indexhtml = String("") +
                   "<!DOCTYPE html>\n" +
                   "<head>\n" +
                   "    <meta charset=\"UTF-8\">\n" +
                   "    <title>Update Test</title>\n" +
                   "</head>\n" +
                   "<body>\n" +
                   "    <form method=\'POST\' action=\'/update\' enctype=\'multipart/form-data\'>\n" +
                   "        <input type=\'file\' name=\'firmware\'>\n" +
                   "        <input type=\'submit\'>\n" +
                   "    </form>\n" +
                   "</body>\n";

bool shouldreboot = false; // 重启标志，固件升级操作完成后设置该标志准备重启设备

void handleResponse() //回调函数
{
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
}

void handleFileupload() //回调函数
{
    HTTPUpload &upload = server.upload();   // 文件上传对象
    if (upload.status == UPLOAD_FILE_START) // 文件上传开始
    {
        Serial.printf("开始上传文件: %s\n", upload.filename.c_str());
        if (!Update.begin()) // 开始固件升级，检查空可用间大小，如果正在升级或空间不够则返回false
        {
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE) // 文件读写中
    {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) // 将文件数据写入到OTA区域
        {
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_END) // 文件上传完成
    {
        Serial.println("写入" + String(upload.totalSize) + "字节到OTA区域");
        if (!Update.end(true)) // 完成数据写入，设置在系统重启后自动将OTA区域固件移动到Sketch区域 // Update.begin不指定大小时这里设置true
        {
            Update.printError(Serial);
        }
        Serial.println("升级操作完成，模块将在5秒后重启以完成固件升级");
        shouldreboot = true;
    }
    else
    {
        Serial.printf("固件上传失败: status=%d\n", upload.status);
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println();

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected");
    Serial.print("IP Address:");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", indexhtml); // 发送网页
    });

    server.on("/update", HTTP_POST, handleResponse, handleFileupload); // 绑定回调函数

    server.begin(); //启动服务器

    Serial.println("Web server started");
}

void loop()
{
    server.handleClient(); //处理来自客户端的请求

    if (shouldreboot)
    {
        delay(5000);
        ESP.restart(); // 重启设备
    }
}
