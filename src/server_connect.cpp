#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include "PubSubClient.h"
#include <HTTPClient.h>
//#include "head_file\initwifi.h"
#include "head_file\data.h"
#include <time.h>
#include <ArduinoJson.h>
#include "head_file\fontzh20.h"
 
//uFire_SHT20 sht20;
 
const char* ssid = "99";
const char* password = "HZK12346789h";
const char* mqtt_server = "mqtts.heclouds.com"; //onenet 的 IP地址 mqtts.heclouds.com 183.230.40.96
const int port = 1883;                    //端口号

#define mqtt_pubid "4aS7AhlV8X"    //产品ID
#define mqtt_devid "circlecheck" //设备名称
#define mqtt_password "version=2018-10-31&res=products%2F4aS7AhlV8X%2Fdevices%2Fcirclecheck&et=2058447118&method=md5&sign=jXrmJQJYfBznCCdUJH0SrA%3D%3D"  //key
TFT_eSPI tft = TFT_eSPI();  // 创建对象控制屏幕
#define A3144Port 13 // 霍尔引脚
int hallState = 0 ;
int counter=0,daysum=0,latest=0;
int flag=1;

WiFiClient espClient;           //创建一个WIFI连接客户端
PubSubClient client(espClient); // 创建一个PubSub客户端, 传入创建的WIFI客户端
float dis;

//设备上传数据的post主题
#define ONENET_TOPIC_PROP_POST "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/post" //"$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/post"
//接收下发属性设置主题
#define ONENET_TOPIC_PROP_SET  "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/set" //"$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/set"
//接收下发属性设置成功的回复主题
#define ONENET_TOPIC_PROP_SET_REPLY "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/set_reply" //"$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/set_reply"
 
//接收设备属性获取命令主题
#define ONENET_TOPIC_PROP_GET "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/get" //"$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/get"
//接收设备属性获取命令成功的回复主题
#define ONENET_TOPIC_PROP_GET_REPLY "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/get_reply" //"$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/get_reply"
 
//这是post上传数据使用的模板
#define ONENET_POST_BODY_FORMAT "{\"id\":\"%u\",\"version\":\"1.0\",\"params\":%s}"
//#define ONENET_POST_BODY_FORMAT
int postMsgId = 0; //记录已经post了多少条

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

//重连函数, 如果客户端断线,可以通过此函数重连
void clientReconnect()
{
  while (!client.connected()) //再重连客户端
  {
    Serial.println("reconnect MQTT...");
    if ( client.connect(mqtt_devid, mqtt_pubid, mqtt_password) )
    {
      Serial.println("connected");
    }
    else
    {
      Serial.println("failed");
      Serial.println(client.state());
      Serial.println("try again in 5 sec");
      delay(5000);
    }
  }
}
//连接WIFI相关函数
void setupWifi()
{
  delay(10);
  Serial.println("connect WIFI");
  WiFi.begin(ssid, password);
  while (!WiFi.isConnected())
  {
    Serial.print(".");
    delay(500);
  }
  configTime(8*3600, 0, "pool.ntp.org");
  Serial.println("OK");
  Serial.println("Wifi connected!");
}

void checkNet() {
  // put your main code here, to run repeatedly:

  if (!WiFi.isConnected()) //先看WIFI是否还在连接
  {
    setupWifi();
  }
  if (!client.connected()) //如果客户端没连接ONENET, 重新连接
  {
    clientReconnect();
    delay(100);
  }
  client.loop();
}

void get_frequency(){
    delay(1000);
    tft.init();                // 初始屏幕
    main_show();
    time_t now1,now2;
   
    while(1){
        checkNet();
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


void sendData(int circle)
{
  if (client.connected())
  {
    //先拼接出json字符串
    char params[82];
    char jsonBuf[178];//
     configTime(8*3600, 0, "pool.ntp.org");
    time_t times;
    sprintf(params, "{\"circle\":{\"value\":%d}}", circle); //写在param里
    postMsgId ++;
    sprintf(jsonBuf, ONENET_POST_BODY_FORMAT, postMsgId, params);
    //再从mqtt客户端中发布post消息
    if (client.publish(ONENET_TOPIC_PROP_POST, jsonBuf))
    {
      Serial.print("Post message to cloud: ");
      Serial.println(jsonBuf);
    }
    else
    {
      Serial.println("Publish message to cloud failed!");
    }
  }

}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //初始化串口
  //Wire.begin();
  //sht20.begin();
  delay(1000);
  setupWifi();        
  delay(1000);                                   //调用函数连接WIFI
  client.setServer(mqtt_server, port);                   //设置客户端连接的服务器,连接Onenet服务器, 使用1883端口
  
  Serial.println("setServer Init!"); 
  //client.setCallback(callback);
  client.connect(mqtt_devid, mqtt_pubid, mqtt_password); //客户端连接到指定的产品的指定设备.同时输入鉴权信息
  delay(1000);
  Serial.println("connect Init...."); 
  
  if (client.connected())
  {
    Serial.println("OneNet is connected!"); //判断以下是不是连好了.
  }
  client.subscribe(ONENET_TOPIC_PROP_SET);
  client.subscribe(ONENET_TOPIC_PROP_GET);
  get_frequency();

  
}
int count = 0;
void loop(){

}


