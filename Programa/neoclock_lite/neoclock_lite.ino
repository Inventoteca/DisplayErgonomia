#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <NeoDigito.h>
#include "time.h"
#include <WiFiManager.h>


#define PIN         2   // Pin where the display will be attached, ESP-01 Only GPIO 3(RX)
#define DIGITS      4   // Number of NeoDigitos connected
#define PIXPERSEG   2   // NeoPixels per segment, BIG version

NeoDigito display1 = NeoDigito(DIGITS, PIXPERSEG, PIN);


String city = "Tonantzintla+Puebla";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -6 * 60 * 60;         // Adjust for time zone for (UTC -6): -6 * 60 * 60
const int   daylightOffset_sec = 0;               // Set it to 3600 if your country observes Daylight saving time; otherwise, set it to 0.
String url = "https://wttr.in/" + city + "?format=%C+%f+%h+%T+%l&lang=es";

unsigned long previousMillisTime = 0;
unsigned long previousMillisWeather = 0;
unsigned long startDisplayTime = 0;
const long intervalTime = 500;
const long intervalWeather = 60000;
const long displayDuration = 2000;
const long displayDurationConditions = 500;
int weatherStage = 0;
bool displayingWeather = false;
String condition, temperature, humidity;

// Variables globales para almacenar la última hora y minutos conocidos
int lastKnownHour = -1;
int lastKnownMinute = -1;
bool lastKnownDelimiter = false; // Para almacenar el estado del delimitador
String dayAndDate;

WiFiManager wifiManager;


// -------------------------------- Dias y meses
// Función para convertir el número del día de la semana al nombre en español
String dayInSpanish(int dayOfWeek) {
  const char* days[] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
  return days[dayOfWeek];
}

String monthInSpanish(int month) {
  const char* months[] = {"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};
  return months[month];
}

// ------------------------------- printLocalTime
void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    display1.print("FAIL", Red);
    return;
  }
  // Verificar si los segundos son pares
  bool isSecondEven = (timeinfo.tm_sec % 2 == 0);

  // Actualizar delimitador solo si ha cambiado
  if (isSecondEven != lastKnownDelimiter) {
    lastKnownDelimiter = isSecondEven;
    if (isSecondEven) {
      display1.updateDelimiter(2, White);
    }
    else
      display1.updateDelimiter(2, 0);
  }
  // Actualizar hora solo si ha cambiado
  if (timeinfo.tm_hour != lastKnownHour) {
    lastKnownHour = timeinfo.tm_hour;
    display1.setCursor(0);
    String timeString = "";
    if (lastKnownHour < 10) {
      timeString += " ";
    }
    timeString += String(lastKnownHour);
    display1.print(timeString, Cian);
  }

  // Actualizar minutos solo si han cambiado
  if (timeinfo.tm_min != lastKnownMinute) {
    lastKnownMinute = timeinfo.tm_min;
    String timeString = "";
    if (lastKnownMinute < 10) {
      timeString += "0";
    }
    timeString += String(lastKnownMinute);
    display1.setCursor(2);
    display1.print(timeString, Pink);
  }

  // Crear una cadena para el día y la fecha
  String day = dayInSpanish(timeinfo.tm_wday);
  String month = monthInSpanish(timeinfo.tm_mon);
  dayAndDate = "   " + day + " " + String(timeinfo.tm_mday) + " " + month + " "  + String(timeinfo.tm_year + 1900);

}


// ------------------------------------ setup
void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("NeoClock");
  Serial.println("WiFi");

  display1.begin();             // This fuction calls Adafruit_NeoPixel.begin() to configure.
  //display1.clear();             // It erase the value.
  display1.print("wifi", Red);      // It prints the value.
  //display1.show();              // Lights up the pixels.

  // Configura el modo Wi-Fi en AP + STA
  WiFi.mode(WIFI_AP_STA);

  // Obtén la dirección MAC del ESP8266
  String macAddress = WiFi.macAddress();
  macAddress.replace(":", ""); // Elimina los caracteres de dos puntos
  Serial.println("MAC Address: " + macAddress);

  // Extrae los últimos 4 caracteres de la dirección MAC
  String macSuffix = macAddress.substring(macAddress.length() - 4);

  // Concatena los caracteres extraídos con el prefijo "NeoClock_" para formar el SSID
  String apSSID = "NeoClock_" + macSuffix;
  Serial.println("AP SSID: " + apSSID);

  // Configura el punto de acceso con el SSID generado
  const char* apPassword = "12345678"; // Debe tener al menos 8 caracteres


  // Si no puedes conectarte a la red WiFi previamente configurada, entra en modo de configuración
  if (WiFi.status() != WL_CONNECTED) {
    // Configurar el portal cautivo con WiFiManager
    wifiManager.autoConnect(apSSID.c_str());
  }

  // Si aún no estás conectado, puedes utilizar SmartConfig
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.beginSmartConfig();

    // Espera a que termine SmartConfig
    while (!WiFi.smartConfigDone()) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("SmartConfig recibido.");

    // Espera a que se conecte a WiFi
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Conectado a ");
    Serial.println(WiFi.SSID());
  }


  // Intenta conectarse a una red previamente configurada
  //if (!wifiManager.autoConnect(apSSID.c_str())) {
  //  Serial.println("Falló la conexión a Wi-Fi y se alcanzó el tiempo de espera");

  // Abre el portal de configuración si no pudo conectarse
  //  wifiManager.startConfigPortal(apSSID.c_str(), apPassword);
  // }

  //Serial.println("Conectado");
  display1.updateColor(Green);    // Color specified by name RED, WHITE, YELLOW, etc or 32bit, or 8bit numbers (R, G, B)
  //display1.show();
  delay(500);

  //wifiManager.startConfigPortal();

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

}


// ----------------------------------- loop
void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisTime >= intervalTime && !displayingWeather) {
    previousMillisTime = currentMillis;
    printLocalTime();
  }

  if (currentMillis - previousMillisWeather >= intervalWeather && !displayingWeather) {
    previousMillisWeather = currentMillis;
    displayingWeather = true;
    weatherStage = 0;

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      HTTPClient http;

      // Ignore SSL certificate errors
      client.setInsecure();

      http.begin(client, url);
      int httpCode = http.GET();

      if (httpCode > 0) {
        String payload = http.getString();
        Serial.println(payload);

        int plusSign = payload.indexOf('+');
        int percentSign = payload.indexOf('%');
        int colonSign = payload.indexOf(':');
        int minusSign = payload.lastIndexOf('-');
        int lastSpace = payload.lastIndexOf(' ');

        String time = payload.substring(percentSign + 2, minusSign - 1);  // Time is between the percentage sign and the minus sign
        String timezone = payload.substring(minusSign, lastSpace);  // Timezone is between the minus sign and the last space
        String location = payload.substring(lastSpace + 1);  // Location is everything after the last space
        condition = "   " + location + " " +  payload.substring(0, plusSign - 1);
        temperature = payload.substring(plusSign + 1, percentSign - 3);
        humidity = payload.substring(percentSign - 2, percentSign + 1);

        startDisplayTime = currentMillis;
      }

      http.end();
    }
  }

  if (displayingWeather) {
    if (weatherStage <= 2 && currentMillis - startDisplayTime >= displayDuration) {
      startDisplayTime = currentMillis;
      display1.clear();
      weatherStage++;
      if (weatherStage == 1) {
        display1.print(temperature, Purple);
      } else if (weatherStage == 2) {
        display1.print(humidity, Cian);
      }
    } else if (weatherStage > 2 && currentMillis - startDisplayTime >= displayDurationConditions) {
      startDisplayTime = currentMillis;
      display1.clear();
      weatherStage++;
      int startChar = weatherStage - 3;
      if (startChar < condition.length()) {
        String conditionSnippet = condition.substring(startChar, startChar + 4);
        display1.print(conditionSnippet, Yellow);
      }
      // Luego mostrar el día y la fecha
      else if (startChar - condition.length() < dayAndDate.length()) {
        int dateChar = startChar - condition.length();
        String dateSnippet = dayAndDate.substring(dateChar, dateChar + 4);
        display1.print(dateSnippet, White);
        //neodisplay.updateColor(Rainbow);
      }
      else {
        displayingWeather = false;
        lastKnownHour = -1;
        lastKnownMinute = -1;
        lastKnownDelimiter = false;
      }
    }
  }
}
