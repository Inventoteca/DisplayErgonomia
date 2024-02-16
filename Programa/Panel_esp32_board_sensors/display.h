#ifndef DISPLAY_H
#define DISPLAY_H

#include "system.h"

#define PIX_STATUS  17    // Decimal poimt as LED status
// -------------- neodigits
#define DIGITS      17    // Number of NeoDigitos connected
#define PIXPERSEG   2    // NeoPixels per segment
//#define NEO_PIN     25    // Pin where the display will be attached

extern bool neo_digits_status;
extern bool blk;
extern short color_status[3];
extern uint32_t color;
//extern unsigned long printRefresh;
//extern unsigned long printTime;

extern SSD1306Wire displayOLED;   // ADDRESS, SDA, SCL 
extern NeoDigito display1; // For more info abaut the last argumets check Adafruit_Neopixel documentation.
extern Adafruit_NeoPixel strip;
extern int pixelCruz[];


void displayInit();
void PrintOut();


#endif  // 
