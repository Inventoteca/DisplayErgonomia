#ifndef WIFISERVICE_H
#define WIFISERVICE_H

#include "system.h"

//extern const uint32_t connectTimeoutMs;
//extern unsigned long  s_timestamp;
extern bool correct;
extern int wifi_trys;
extern boolean isSaved;

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
void Wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void checkServer();
void websocketInit();
void neoConfig();

#endif 
