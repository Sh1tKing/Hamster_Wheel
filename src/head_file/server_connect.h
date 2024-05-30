#ifndef SERVER_CONNECT_H
#define SERVER_CONNECT_H
void sendData(int daycircle);
void sendLatestCir(int circle);
void saveLocalday(int yday);
void saveLocalrh(int rh);
void clientReconnect();
void sendServerHourLap(int PerLap,String time);
#endif