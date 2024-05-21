#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
//#include "head_file\initwifi.h"
#include "head_file\server_connect.h"
#include <time.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>

String tmp="0";
std::map<String,std::map<int,int>> T;
void timedata_store(time_t &now,int counter){ //时间信息处理
    
    char myStr[25] = { 0 };
    Serial.begin(9600);
	struct tm *t = gmtime(&now);
    t->tm_hour+=8;
    sendData(counter);
    if(t->tm_year<=2024) return;
    
    //t为当天时间
    char date1[20]={0};
    int time=t->tm_hour;
    std::string myFormat = "%Y-%m-%d";
	strftime(date1, sizeof(date1), myFormat.c_str(), t);
    String date;
    for(int i=0;i<strlen(date1);i++){
        date+=date1[i];
    }
    if(T.find(date)!=T.end()){
        if(T[date].find(time)!=T[date].end()){
            T[date][time]+=counter;
        }else{
            T[date][time]=counter;
        }
    }else{
        T[date][time]=counter;
    }
    Serial.println("---------------------------------------");
    for(auto it=T.begin();it!=T.end();it++){
        for(auto it1=it->second.begin();it1!=it->second.end();it1++){
            Serial.print(it->first);
            Serial.print(" ");
            Serial.printf("%d%c",it1->first,'h');
            Serial.print(" ");
            Serial.printf("%d",it1->second);
            Serial.println("rad");
        }
    }
    Serial.println("---------------------------------------");
    
}
void gettimestamp(int counter){
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
        timedata_store(now,counter);
        tmp=str;
    } 
    
}

