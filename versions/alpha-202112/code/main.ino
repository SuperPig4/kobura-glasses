
//   Diagnostic test for the displayed colour order
//
// Written by Bodmer 17/2/19 for the TFT_eSPI library:
// https://github.com/Bodmer/TFT_eSPI

/* 
 Different hardware manufacturers use different colour order
 configurations at the hardware level.  This may result in
 incorrect colours being displayed.

 Incorrectly displayed colours could also be the result of
 using the wrong display driver in the library setup file.

 Typically displays have a control register (MADCTL) that can
 be used to set the Red Green Blue (RGB) colour order to RGB
 or BRG so that red and blue are swapped on the display.

 This control register is also used to manage the display
 rotation and coordinate mirroring. The control register
 typically has 8 bits, for the ILI9341 these are:

 Bit Function
 7   Mirror Y coordinate (row address order)
 6   Mirror X coordinate (column address order)
 5   Row/column exchange (for rotation)
 4   Refresh direction (top to bottom or bottom to top in portrait orientation)
 3   RGB order (swaps red and blue)
 2   Refresh direction (top to bottom or bottom to top in landscape orientation)
 1   Not used
 0   Not used

 The control register bits can be written with this example command sequence:
 
    tft.writecommand(TFT_MADCTL);
    tft.writedata(0x48);          // Bits 6 and 3 set
    
 0x48 is the default value for ILI9341 (0xA8 for ESP32 M5STACK)
 in rotation 0 orientation.
 
 Another control register can be used to "invert" colours,
 this swaps black and white as well as other colours (e.g.
 green to magenta, red to cyan, blue to yellow).
 
 To invert colours insert this line after tft.init() or tft.begin():

    tft.invertDisplay( invert ); // Where invert is true or false

*/

#include <SPI.h>

#include <ESP8266WebServer.h>
#include <TFT_eSPI.h>       // Hardware-specific library
#include <FS.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <WiFiClient.h>


TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

//wifi配置
const char* ssid = "wadashinowifi";
const char* password = "pwd*1364";

// html
const char mainPageString[] PROGMEM = "<!DOCTYPE html>\r\n\
<html>\r\n\
<head>\r\n\
  <meta http-equiv='Content-type' content='text/html; charset=utf-8'>\r\n\
  <title>ESP Monitor</title>\r\n\
</head>\r\n\
<body>\r\n\
  <form method='POST' action='/' enctype='multipart/form-data'>\r\n\
  <input type='input' name='x'>\r\n\
  <input type='input' name='y'>\r\n\
  <input type='input' name='update'>\r\n\
  <input type='submit' value='提交'>\r\n\
  </form>\r\n\
  文件格式必须为bmp，24位深，本示例使用的屏幕为1.44寸TFT,分辨率128*128，ic:ST7735\r\n\
</body>\r\n\
</html>";

//监听80端口
ESP8266WebServer server(80);

//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

// 跟页面处理
void handleRoot()
{
  server.send(200, "text/html", mainPageString);
  server.client().stop();
}

// 提交处理
void handlePostSubmit() {
    if (server.hasArg("text")) {
        Serial.print("请求中token参数的值:"); 
        Serial.println(server.arg("text"));
        
        tft.fillScreen(TFT_WHITE);
        tft.setTextColor(TFT_RED, TFT_WHITE);
        tft.setCursor(server.arg("x").toInt(), server.arg("y").toInt(), 4);
        tft.println(server.arg("text"));
    }
    else {
        Serial.println("当前请求中无法找到指定请求体内容");
    }

    server.send(200, "text/html", "ok");
    server.client().stop();
}

void setup(void) {
    // 初始化串口通讯波特率为115200
    Serial.begin(115200);
    Serial.print("\n");
    Serial.setDebugOutput(true);

    // 输出目录
    SPIFFS.begin();
    {
        Dir dir = SPIFFS.openDir("/");
        while (dir.next()) {    
            String fileName = dir.fileName();
            size_t fileSize = dir.fileSize();
            Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
        }
        Serial.printf("\n");
    }
  
    // tft
    tft.init();
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_GREEN, TFT_WHITE);
    tft.println("Green text");
    delay(2000);

    // wifi初始化
    Serial.printf("Connecting to %s\n", ssid);
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());

    //SERVER INIT
    server.on("/", HTTP_GET, handleRoot);
    server.on("/", HTTP_POST, handlePostSubmit);
    server.onNotFound([](){
        server.send(404, "text/plain", "FileNotFound-55555");
    });
    server.begin();
}

void loop() {
  server.handleClient();
}