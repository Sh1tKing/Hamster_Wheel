#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include "PubSubClient.h"
#include <HTTPClient.h>
#include "head_file\data.h"
#include <time.h>
#include <ArduinoJson.h>
#include <Preferences.h>
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
int counter=0,daysum=0,latest=0,phround=0;
int flag=1;

String serverName = "https://iot-api.heclouds.com/thingmodel/query-device-property?product_id=4aS7AhlV8X&device_name=circlecheck";
const char *headerKeys[] = {"Authorization", "version=2022-05-01&res=userid%2F390250&et=2016217735&method=sha1&sign=pq%2Bbjtzv8VTEJcqS2v5uIME7uMk%3D"};
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
int postMsgId1 = 0,postMsgId2 = 0,postMsgId3 = 0 ; //记录已经post了多少条

void sendData(int daycircle);
void sendLatestCir(int circle);
void clientReconnect();
void sendPerRData(int PerR);

int getdaycircle(){

  HTTPClient http; // 声明HTTPClient对象

  http.begin(serverName); // 准备启用连接

  http.collectHeaders(headerKeys, 2); // 准备需要接收的响应头内容

  http.addHeader("Authorization","version=2022-05-01&res=userid%2F390250&et=2016217735&method=sha1&sign=pq%2Bbjtzv8VTEJcqS2v5uIME7uMk%3D",false,true);
  int httpCode = http.GET(); // 发起GET请求

  int daycircle=0;
  if (httpCode > 0) // 如果状态码大于0说明请求过程无异常
  {
    if (httpCode == HTTP_CODE_OK) // 请求被服务器正常响应，等同于httpCode == 200
    {
      char buff[512] = {0};

      int len = http.getSize(); // 读取响应正文数据字节数，如果返回-1是因为响应头中没有Content-Length属性

      WiFiClient *stream = http.getStreamPtr(); // 获取响应正文数据流指针

      while (http.connected() && (len > 0 || len == -1)) // 当前已连接并且有数据可读
      {
        size_t size = stream->available(); // 获取数据流中可用字节数
        if (size)
        {
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size)); // 读取数据到buff
          
          if (len > 0)
          {
            len -= c;
          }
        }
        
      }
      for(int i=0;i<strlen(buff);i++){
        if(buff[i]=='d'&&buff[i+1]=='a'&&buff[i+2]=='y'){
            int len=41;
            while(1){
              
              if(buff[i+len]=='"') break;
              daycircle=daycircle*10+buff[i+len]-'0';
              len++;
            }
            break;
        }
        
      }
    }
  }
  return daycircle;
}
time_t checkday(){
HTTPClient http; // 声明HTTPClient对象

  http.begin(serverName); // 准备启用连接

  http.collectHeaders(headerKeys, 2); // 准备需要接收的响应头内容

  http.addHeader("Authorization","version=2022-05-01&res=userid%2F390250&et=2016217735&method=sha1&sign=pq%2Bbjtzv8VTEJcqS2v5uIME7uMk%3D",false,true);
  int httpCode = http.GET(); // 发起GET请求

  time_t day=0;
  if (httpCode > 0) // 如果状态码大于0说明请求过程无异常
  {
    if (httpCode == HTTP_CODE_OK) // 请求被服务器正常响应，等同于httpCode == 200
    {
      char buff[512] = {0};

      int len = http.getSize(); // 读取响应正文数据字节数，如果返回-1是因为响应头中没有Content-Length属性

      WiFiClient *stream = http.getStreamPtr(); // 获取响应正文数据流指针

      while (http.connected() && (len > 0 || len == -1)) // 当前已连接并且有数据可读
      {
        size_t size = stream->available(); // 获取数据流中可用字节数
        if (size)
        {
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size)); // 读取数据到buff
          
          if (len > 0)
          {
            len -= c;
          }
        }
        
      }
      for(int i=0;i<strlen(buff);i++){
        if(buff[i]=='d'&&buff[i+1]=='a'&&buff[i+2]=='y'){
            int len=18;
            while(1){
              if(buff[i+len+3]==',') break;
              day=day*10+buff[i+len]-'0';
              len++;
            }
            break;
        }
        
      }
    }
  }
  return day;
}
void main_show(){
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);  
    tft.loadFont(fontzh20);      

    tft.drawCentreString("当前: ",30,20,20); 
    tft.setTextSize(2);          
    tft.print(counter);  
    tft.drawCentreString("圈",110,20,20);  

    tft.drawCentreString("最近: ",30,50,20); 
    tft.setTextSize(2);          
    tft.print(latest);  
    tft.drawCentreString("圈",110,50,20);   

    tft.drawCentreString("今日",20,80,20);  
    tft.drawCentreString("圈数: ",30,110,20);  
    tft.setTextSize(2);          
    tft.print(daysum);  
    tft.drawCentreString("圈",110,110,20);   

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
  configTime(8*3600, 0, "asia.pool.ntp.org");
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
Preferences prefs;
uint32_t daystamp = prefs.getUInt("daystamp", 0);
uint32_t rh = prefs.getUInt("rh", 0);
void saveLocalday(int yday){
    // 声明Preferences对象
    Serial.begin(9600);
    prefs.begin("mynamespace"); // 打开命名空间mynamespace
    prefs.putUInt("daystamp", yday); // 将数据保存到当前命名空间的"count"键中
    prefs.end(); // 关闭当前命名空间
}
void saveLocalrh(int rh){
    // 声明Preferences对象
    Serial.begin(9600);
    prefs.begin("mynamespace"); // 打开命名空间mynamespace
    prefs.putUInt("rh", rh); // 将数据保存到当前命名空间的"count"键中
    prefs.end(); // 关闭当前命名空间
}
void get_frequency(){
    delay(1000);
    tft.init();       
    main_show();
    time_t now1=time(NULL),now2=time(NULL);
    while(1){
        checkNet();
        time(&now1);
        struct tm *t = gmtime(&now1); 
        int nowyear=t->tm_year, nowmin=t->tm_min,nowhour=t->tm_hour,nowsec=t->tm_sec,nowday=t->tm_yday;
        while(nowyear==70){
          Serial.print("loading time");
          Serial.print(".");
          delay(1000);
        }
        prefs.begin("mynamespace");
        //Serial.println(nowyear);
        if(prefs.getUInt("daystamp",0)!=nowday){
             sendData(0);
             daysum=getdaycircle();
             main_show();
        }
        hallState = digitalRead ( A3144Port ) ; 
        time(&now1); 
        //Serial.println(nowmin);
        if(hallState==LOW){
            time(&now2);
            flag=0;
            counter++;
            phround++;
            saveLocalrh(phround);
            while (hallState==LOW)
            {
               hallState = digitalRead ( A3144Port ) ;
            }
            sendLatestCir(counter);
            main_show();
        } 

        if(nowmin==0 && nowsec==0){
            //Serial.println("asdfasdfa");
            sendPerRData(phround);
            phround=0;
            saveLocalrh(phround);
            delay(1000);
        }
        if(now1-now2==3 && flag==0) {
            latest=counter;
            daysum+=counter;
            sendLatestCir(0);
            gettimestamp(counter,daysum);
            counter=0;
            flag=1;
            main_show();
        }
    }
}
void sendLatestCir(int circle){
  //发送当前圈数
    {
       char params[82];
       char jsonBuf[178];//
       configTime(8*3600, 0, "asia.pool.ntp.org");
       time_t times;
       sprintf(params, "{\"circle\":{\"value\":%d}}", circle); //写在param里
       postMsgId1 ++;
       sprintf(jsonBuf, ONENET_POST_BODY_FORMAT, postMsgId1, params);
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
void sendData(int daycircle)
{
  if (client.connected())
  {
    
    {
      //发送每日圈数
      char params[82];
      char jsonBuf[178];//
      configTime(8*3600, 0, "pool.ntp.org");
      time_t times;
      sprintf(params, "{\"daycircle\":{\"value\":%d}}", daycircle); //写在param里
      postMsgId2 ++;
      sprintf(jsonBuf, ONENET_POST_BODY_FORMAT, postMsgId2, params);
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

}
void sendPerRData(int PerR)
{
  if (client.connected())
  {
    {
      //发送每日圈数
      char params[82];
      char jsonBuf[178];//
      configTime(8*3600, 0, "asia.pool.ntp.org");
      time_t times;
      sprintf(params, "{\"PerR\":{\"value\":%d}}", PerR); //写在param里
      postMsgId3 ++;
      sprintf(jsonBuf, ONENET_POST_BODY_FORMAT, postMsgId3, params);
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

}

void setup() {
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
  prefs.begin("mynamespace");
  phround=prefs.getUInt("rh",0);
  //Serial.println(phround);
  daysum=getdaycircle();
  get_frequency();
  
  
}
int count = 0;
void loop(){

}


