#ifndef SENSORS_H
#define SENSORS_H

#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include "filespiffs.h"
#include "pines.h"
#include "display.h"

#define DHTTYPE     DHT22   // DHT 22  (AM2302), AM2321

extern bool sensors_init;
extern DHT dht;

extern Adafruit_TSL2561_Unified tsl;

extern bool tsl_status;

extern unsigned long tempRefresh;

// ---------------- sensors variables
extern int t;
extern int h;
extern int db;
//double ppm;
extern unsigned int ppm;
extern float uv;
extern unsigned long lux;
extern sensors_event_t event;

// ------ auxiliar variables for sensors
extern float outputVoltage;
extern int adc;
extern int adc_33;

extern char ch_db[10];
extern int last_db;
extern int last_t;
extern int last_h;



//mq-135
extern float m; //Slope
extern float b; //Y-Intercept
//float R0 = 11.820; //Sensor Resistance in fresh air f
extern float R0; //Sensor Resistance in fresh air f

void sensorInit();
void ReadSensors();
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);

#endif
