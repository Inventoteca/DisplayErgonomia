#ifndef SYSTEM_H
#define SYSTEM_H

#include <Arduino.h>
#include <WiFi.h> //import for wifi functionality
#include "pines.h"
#include "filespiffs.h"

extern bool factory_press;
extern unsigned long factory_time;
extern unsigned long prev_factory_time;
extern bool reset_time;
extern bool smart_config;
extern bool taskCompleted;
extern byte localAddress;     // address of this device

void IRAM_ATTR factory_reset1();
void reset_config();
void neoConfig();
bool strToBool(String str);
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
void loadConfig();

#endif
