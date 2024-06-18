#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "head_file\server_connect.h"
#include <time.h>
#include <vector>
#include <string>
#include <map>
#include <Preferences.h>
#include <iostream>

String tmp="0";
std::map<String,std::map<int,int>> T; 

void timedata_store(time_t &now,int counter,int daycircle){ //时间信息处理
    
    char myStr[25] = { 0 };
    Serial.begin(9600);
	struct tm *t = gmtime(&now);
    t->tm_hour+=8;
    saveLocalday(t->tm_yday);
    sendData(daycircle);
}
void gettimestamp(int counter,int daysum){
    time_t now;
    time(&now);  //获取时间戳 
    time_t tmptime=now;
    String temp;
    while (tmptime){
        temp+=char(tmptime%10+'0');
        tmptime/=10;
    }
    String str;
    for(int i=0;i<temp.length();i++){
        str+=temp[temp.length()-i-1];
    }
    if(tmp!=str){
        timedata_store(now,counter,daysum);
        tmp=str;
    } 
    
}

