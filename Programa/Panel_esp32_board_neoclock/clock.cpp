#include "clock.h"


RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
DateTime now;
DateTime last_ac;

int dias;
int mes;
int anio;
int dia_hoy;

const char* ntpServer = "pool.ntp.org";
//long  gmtOffset_sec = obj["gmtOff"].as<long>();               // Central Mexico (-5 UTC, -18000): Pacifico (-7 UTC, -25200) :  Noroeste (-8 UTC, -28800)
//int   daylightOffset_sec = obj["dayOff"].as<int>();               // Horario de verano, disabled
long  gmtOffset_sec;
int   daylightOffset_sec;
bool ntpConnected = false;
bool rtcUpdated = false;
bool rtc_ready = false;


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

// ---------------------------------- init_clock
void init_clock()
{
  rtc_ready = rtc.begin();
  if (!rtc_ready )
  {
    Serial.println("{\"rtc_init\":false}");
  }
  else
  {
    Serial.println("{\"rtc_init\":true}");

    now = rtc.now();
    Serial.print("{\"time\":\"");
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

    gmtOffset_sec = obj["gmtOff"].as<long>();
    daylightOffset_sec = obj["dayOff"].as<int>();

    Serial.print("{\"gmtOff\":");
    Serial.print(gmtOffset_sec);
    Serial.println("}");

    Serial.print("{\"dayOff\":");
    Serial.print(daylightOffset_sec);
    Serial.println("}");
    
  }
}


// -------------------------------- update_clock
void update_clock()
{
  if (rtc_ready == true)
  {
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
  else
  {
    init_clock();
  }

}

// -------------------------------- read_clock
void read_clock()
{
  if (rtc_ready)
  {
    now = rtc.now();
    //dias = int(round(round(now.unixtime() - last_ac.unixtime()) / 86400L));
    dias = (now.unixtime() - last_ac.unixtime()) / 86400;
    mes = now.month();
    anio = now.year();
    dia_hoy = now.day(); 

    Serial.print("{\"time\":\"");
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

    Serial.print("{\"since\":\"");
    Serial.print(last_ac.year(), DEC);
    Serial.print('/');
    Serial.print(last_ac.month(), DEC);
    Serial.print('/');
    Serial.print(last_ac.day(), DEC);
    Serial.print(' ');
    Serial.print(last_ac.hour(), DEC);
    Serial.print(':');
    Serial.print(last_ac.minute(), DEC);
    Serial.print(':');
    Serial.print(last_ac.second(), DEC);
    Serial.println("\"}");

    Serial.print("{\"last_ac\":");
    Serial.print(last_ac.unixtime());
    Serial.println("}");
    Serial.print("{\"t_unix\": ");
    Serial.print(now.unixtime());
    Serial.println("}");

    // Si el dia actual es diferente al anterior se reinicia
    // Si el mes o el el anio es diferente se reinicia events
    if(obj["mes_prev"].as<int>() != mes)
    {
      Serial.println("{\"new_month\":true}");
      Serial.print("{\"prev_mes\": "); Serial.print(obj["mes_prev"].as<int>());  Serial.println("}");
      Serial.print("{\"actual_mes\": "); Serial.print(mes);  Serial.println("}");
      obj.remove("events");
      obj["events"]["32"]=0;
      obj["mes_prev"] = mes;
      SendData();
      saveConfig = true;
      update_events = true;
    }
  }
  else
  {
    init_clock();
  }
}


//
//// Local time
//void printLocalTime()
//{
//  struct tm timeinfo;
//  if (!getLocalTime(&timeinfo)) {
//    Serial.println("Failed to obtain time");
//    return;
//  }
//  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");   // Comment for ESP8266
//}
