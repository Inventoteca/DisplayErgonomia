#ifndef CLOCK_H
#define CLOCK_H

//#include "time.h"
//#include <Arduino.h>
#include "RTClib.h"

extern DateTime now;
extern DateTime last_ac;
extern RTC_DS1307 rtc;

#endif
