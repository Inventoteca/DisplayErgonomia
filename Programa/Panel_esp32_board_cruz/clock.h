#ifndef CLOCK_H
#define CLOCK_H

#include "time.h"
//#include <Arduino.h>
#include "RTClib.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "wifiservice.h"

extern char daysOfTheWeek[7][12];
extern DateTime now;
extern DateTime last_ac;
extern RTC_DS1307 rtc;

extern int dias;
extern int mes;
extern int anio;

extern const char* ntpServer;
extern long  gmtOffset_sec;
extern int   daylightOffset_sec;
extern bool ntpConnected;
extern bool rtcUpdated;


extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

void update_clock();
void read_clock();

#endif
