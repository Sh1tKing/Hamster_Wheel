#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include "PubSubClient.h"
#include <HTTPClient.h>
#include "head_file\initwifi.h"
#include "head_file\data.h"
#include <time.h>
#include <ArduinoJson.h>
#include "head_file\fontzh20.h"


const char *mqtt_server = "	mqtts.heclouds.com"; //onenet 的 IP地址
const int port = 1883;                    //端口号

WiFiClient espClient;           //创建一个WIFI连接客户端
PubSubClient client(espClient); // 创建一个PubSub客户端, 传入创建的WIFI客户端
#define mqtt_pubid "4aS7AhlV8X"    //产品ID
#define mqtt_devid "circlecheck" //设备名称
#define mqtt_password "version=2018-10-31&res=products%2F4aS7AhlV8X%2Fdevices%2Fcirclecheck&et=2058447118&method=md5&sign=jXrmJQJYfBznCCdUJH0SrA%3D%3D"  //key
TFT_eSPI tft = TFT_eSPI();  // 创建对象控制屏幕
#define A3144Port 13 // 霍尔引脚
int hallState = 0 ;
int counter=0,daysum=0,latest=0;
int flag=1;

void main_show(){
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);  
    tft.loadFont(fontzh20);      

    tft.drawCentreString("当前: ",30,20,20); 
    tft.setTextSize(2);          
    tft.print(counter);  
    tft.drawCentreString("圈",100,20,20);  

    tft.drawCentreString("最近: ",30,50,20); 
    tft.setTextSize(2);          
    tft.print(latest);  
    tft.drawCentreString("圈",100,50,20);   

    tft.drawCentreString("天圈数: ",40,80,20);  
    tft.setTextSize(2);          
    tft.print(daysum);  
    tft.drawCentreString("圈",110,80,20);   

    tft.unloadFont();
    
}
void get_frequency(){
    delay(1000);
    tft.init();                // 初始屏幕
    main_show();
    time_t now1,now2;
   
    while(1){
        hallState = digitalRead ( A3144Port ) ; 
        time(&now1);
        if(hallState==LOW){
            time(&now2);
            flag=0;
            counter++;
            while (hallState==LOW)
            {
               hallState = digitalRead ( A3144Port ) ;
            }
            main_show();
        } 
        if(now1-now2==5 && flag==0) {
            gettimestamp(counter);
            latest=counter;
            daysum+=counter;
            counter=0;
            flag=1;
            main_show();
        }
    }
}
void setup()
{
    Serial.begin(9600); //初始化串口
    delay(500);      
    initWiFi();
    client.setServer(mqtt_server, port);                   //连接Onenet服务器
    client.connect(mqtt_devid, mqtt_pubid, mqtt_password); //客户端连接到指定的产品的指定设备.同时输入鉴权信息
    if (client.connected())
    {
        Serial.println("OneNet is connected!"); 
    }
    get_frequency();
 
}
void loop(){
    if (!WiFi.isConnected()) {
        initWiFi();
    }
}