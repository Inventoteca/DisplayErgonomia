/*
   Panel Cruz
*/

// -------------------------- library
#include "filespiffs.h"
#include "display.h"
#include "pines.h"
#include "system.h"
#include "clock.h"
#include "sensors.h"
#include "firebasedb.h"
#include "wifiservice.h"



//################################################################----------------------- setup--------------------- #############################
void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiEvent);
  pinMode(FACTORY_BT, INPUT);
  attachInterrupt(FACTORY_BT, factory_reset1, CHANGE);

  // WatchDog Timer
  esp_task_wdt_init(WDT_TIMEOUT, true);   //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);                 //add current thread to WDT watch

  // SPIFFS Init
  if (!SPIFFS.begin(true))
  {
    Serial.println("{\"spiffs\":false}");
    return;
  }
  else
  {
    Serial.println("{\"spiffs\":true}");
    Cfg_get(/*NULL*/);                    // Load File from spiffs
    loadConfig();                         // Load and update behaivor of system
  }

}



//#################################--------------------------------------------- loop------------------------###################################
void loop()
{
  //  if (obj["lora"]["enable"].as<bool>())
  //  {
  //    //onReceive(LoRa.parsePacket());
  //  }

  // ------------------------------------------- ergo
  if (obj["type"].as<String>() == "ergo")
  {
    if (obj["sensors_enable"].as<bool>()) // Sensor Panel normal
    {
      ReadSensors();
    }
  }
  // ------------------------------------------- cruz
  else if (obj["type"].as<String>() == "cruz")
  {
    {
      now = rtc.now();
      PrintOut();
    }

  }

  // ----------------------------------------- save new data
  if (saveConfig) // Data change
  {
    Serial.println("{\"upload_config\":true}");
    saveConfigData();
    saveConfig = false;
  }

  // ----------------------------------------- check internet
  if (obj["wifi"]["sta"]["enable"].as<bool>())
  {
    checkServer();
  }

  // ----------------------------------------- reset wifi data
  if (reset_time)                         // Press and hold reset button
  {
    if ((prev_factory_time - factory_time) > 5000)
    {
      reset_config();
    }
    factory_press = false;
    reset_time = false;
  }

  // ---------------------------------------- wdt reset
  esp_task_wdt_reset();

}
