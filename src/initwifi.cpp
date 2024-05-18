#include <WiFi.h>
#include <HTTPClient.h>
#include <iostream>
#include <time.h>
#include <TFT_eSPI.h>
#include <string>

const char* ssid = "99";
const char* password = "HZK12346789h";
const char *mqtt_server = "mqtts.heclouds.com"; //onenet的地址
const int port = 1883;
TFT_eSPI tft_WIFI = TFT_eSPI();
const long utcOffsetInSeconds = 28800;
const char* ntpServer = "pool.ntp.org";


void show(String msg){
  tft_WIFI.fillScreen(TFT_BLACK);
  tft_WIFI.setCursor(1, 1, 1); 
  tft_WIFI.setTextColor(TFT_WHITE);     
  tft_WIFI.setTextSize(1);              
  tft_WIFI.println(msg);
}
void initWiFi() {
  //连接wifi
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  configTime(utcOffsetInSeconds, 0, ntpServer);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to WiFi...");
    delay(1000);
  }
  Serial.print("Connected to WiFi");
}

