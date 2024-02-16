#ifndef CLOCK_H
#define CLOCK_H


#include "system.h"


extern char daysOfTheWeek[7][12];
extern DateTime now;
extern DateTime last_ac;
extern RTC_DS1307 rtc;

extern int dias;
extern int mes;
extern int anio;
extern int dia_hoy;

extern const char* ntpServer;
extern long  gmtOffset_sec;
extern int   daylightOffset_sec;
extern bool ntpConnected;
extern bool rtcUpdated;
extern bool rtc_ready;


extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

void update_clock();
void read_clock();
void init_clock();

#endif
