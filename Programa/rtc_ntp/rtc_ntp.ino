#include <RTClib.h> // Añade esta línea para incluir la biblioteca RTClib
#include <WiFi.h>  
#include "time.h"

RTC_DS3231 rtc; // Crea un objeto RTC_DS3231
DateTime now;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -6 * 60 * 60;         // Adjust for time zone for (UTC -6): -6 * 60 * 60
const int   daylightOffset_sec = 0;   

void setup()
{
  Serial.begin(115200);
  Serial.println("NeoClock");
  Serial.println("WiFi");

  //display1.begin();
  //display1.clear();
  //display1.print("wifi", Red);
  //display1.show();

  WiFi.begin("Inventoteca_2G", "science_7425");

  while ( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected");
  //display1.updateColor(Green);
  //display1.show();
  delay(500);

  // Configura la conexión con el servidor NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Inicializa el RTC
  if (! rtc.begin()) {
    Serial.println("No se pudo encontrar el módulo RTC");
    while (1);
  }

  // Configura el RTC para usar la hora actual del servidor NTP
  rtc.adjust(DateTime(now));

  // Muestra la hora actual en el display
  printRTC();

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void printRTC()
{
  // Lee la hora actual del RTC
  DateTime now = rtc.now();

  // Convierte la hora actual en una cadena de caracteres
  char buffer[20];
  sprintf(buffer, "%02d:%02d", now.hour(), now.minute());

  // Muestra la hora actual en el display
  //display1.clear();
  //display1.print(buffer, Cian);
  //display1.show();
  Serial.println(buffer);
}

void loop()
{
  // Actualiza la hora del RTC cada 5 minutos
  if (millis() % (1 * 60 * 1000) == 0) {
    rtc.adjust(DateTime(now));
  }

  // Muestra la hora actual en el display
  printRTC();

  delay(1000);
}
