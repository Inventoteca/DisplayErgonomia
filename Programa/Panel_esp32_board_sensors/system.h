#ifndef SYSTEM_H
#define SYSTEM_H

// -------------------------- library
#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <NeoDigito.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h> //import for wifi functionality
#include <RFM69.h>         //get it here: https://www.github.com/lowpowerlab/rfm69
#include <RFM69_ATC.h>     //get it here: https://www.github.com/lowpowerlab/rfm69
#include "SPIFFS.h"
#include "FS.h"
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
#include "RTClib.h"

#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <MQUnifiedsensor.h>

#include "pines.h"
#include "filespiffs.h"
#include "display.h"
#include "firebasedb.h"
#include "wifiservice.h"
#include "clock.h"
#include "loraservice.h"
#include "version.h"
#include "sensors.h"
#include "time.h"

//15 seconds WDT
#define WDT_TIMEOUT 15


extern bool factory_press;
extern unsigned long factory_time;
extern unsigned long prev_factory_time;
//extern bool reset_time;
extern bool smart_config;
extern bool taskCompleted;
extern byte localAddress;     // address of this device
extern unsigned long mainRefresh;
extern unsigned long mainTime;
extern const uint32_t connectTimeoutMs;
extern unsigned long  s_timestamp;
extern int buttonState;
extern unsigned long tiempoInicio;

void IRAM_ATTR factory_reset3();
void reset_config();
void check_reset();
//void neoConfig();
bool strToBool(String str);
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
void loadConfig();
//void prepareData();

#endif
