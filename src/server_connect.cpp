#include <Arduino.h>
#include "WiFi.h"
#include "PubSubClient.h"
#include "Ticker.h"
#include "head_file\initwifi.h"

const char *mqtt_server = "	mqtts.heclouds.com"; //onenet 的 IP地址
const int port = 1883;                    //端口号

#define mqtt_pubid "4aS7AhlV8X"    //产品ID
#define mqtt_devid "circlecheck" //设备名称
#define mqtt_password "version=2018-10-31&res=products%2F4aS7AhlV8X%2Fdevices%2Fcirclecheck&et=2058447118&method=md5&sign=jXrmJQJYfBznCCdUJH0SrA%3D%3D"  //key
 
WiFiClient espClient;           //创建一个WIFI连接客户端
PubSubClient client(espClient); // 创建一个PubSub客户端, 传入创建的WIFI客户端
Ticker tim1;                    //定时器,用来循环上传数据
//设备下发命令的set主题
#define ONENET_TOPIC_PROP_SET "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/set"
//设备上传数据的post主题
#define ONENET_TOPIC_PROP_POST "$sys/" mqtt_pubid "/" mqtt_devid "/thing/property/post"
 
//这是post上传数据使用的模板
#define ONENET_POST_BODY_FORMAT "{\"id\":\"%u\",\"params\":%s}"
int postMsgId = 0; //记录已经post了多少条
 
//连接WIFI相关函数
//向主题发送模拟的温湿度数据
void sendData(int circle)
{
  if (client.connected())
  {
    //先拼接出json字符串
    char param[82];
    char jsonBuf[178];
    sprintf(param, "{\"circle\":{\"value\":%d}}",circle); //我们把要上传的数据写在param里
    postMsgId += 1;
    sprintf(jsonBuf, ONENET_POST_BODY_FORMAT, postMsgId, param);
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
 
//重连函数, 如果客户端断线,可以通过此函数重连
void clientReconnect()
{
  while (!client.connected()) //再重连客户端
  {
    Serial.println("reconnect MQTT...");
    if (client.connect(mqtt_devid, mqtt_pubid, mqtt_password))
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
 