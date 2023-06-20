#include "clock.h"


RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
DateTime now;
DateTime last_ac;

const char* ntpServer = "pool.ntp.org";
//long  gmtOffset_sec = obj["gmtOff"].as<long>();               // Central Mexico (-5 UTC, -18000): Pacifico (-7 UTC, -25200) :  Noroeste (-8 UTC, -28800)
//int   daylightOffset_sec = obj["dayOff"].as<int>();               // Horario de verano, disabled
long  gmtOffset_sec;
int   daylightOffset_sec;
bool ntpConnected = false;
bool rtcUpdated = false;


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);


void update_clock()
{
  // Inicializa el cliente NTP


  // Sincroniza el tiempo del cliente NTP
  if (rtcUpdated == false)
  {
    // Serial.println(gmtOffset_sec);

    if (ntpConnected == false)
    {
      //Serial.println("{\"ntp\":\"connecting...\"}");
      //timeClient.begin();
      timeClient.end();
      gmtOffset_sec = obj["gmtOff"].as<long>();               // Central Mexico (-5 UTC, -18000): Pacifico (-7 UTC, -25200) :  Noroeste (-8 UTC, -28800)
      daylightOffset_sec = obj["dayOff"].as<int>();               // Horario de verano, disabled
      timeClient = NTPClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);
      timeClient.begin();
      Serial.println("{\"ntp\":\"connected\"}");
      ntpConnected = true;
    }

    //timeClient.setTimeOffset(gmtOffset_sec, daylightOffset_sec);
    timeClient.update();

    Serial.print("{\"time_ntp\":\"");
    Serial.print(timeClient.getFormattedTime());
    Serial.println("\"}");

    //Serial.println(timeClient.getEpochTime(), DEC);

    if (timeClient.getSeconds() != 0)
    {
      Serial.println("{\"rtc\":\"updated from NTP\"}");
      // Establece el tiempo del DS1307 utilizando el tiempo del cliente NTP
      rtc.adjust(DateTime(timeClient.getEpochTime()));
      rtcUpdated = true;
    }
    else
    {
      Serial.println("{\"ntp\":\"fail\"}");
      rtcUpdated = false;
    }

    now = rtc.now();
    Serial.print("{\"time_rtc\":\"");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println("\"}");
  }

}
