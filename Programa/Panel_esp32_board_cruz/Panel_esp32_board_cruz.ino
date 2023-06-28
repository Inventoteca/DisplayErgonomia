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
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiEvent);
  pinMode(FACTORY_BT, INPUT);
  attachInterrupt(FACTORY_BT, factory_reset1, CHANGE);

  // WatchDog Timer
  esp_task_wdt_init(WDT_TIMEOUT, true);  //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);                //add current thread to WDT watch

  // SPIFFS Init
  if (!SPIFFS.begin(true)) {
    Serial.println("{\"spiffs\":false}");
    return;
  } else {
    Serial.println("{\"spiffs\":true}");
    Cfg_get(/*NULL*/);  // Load File from spiffs
    loadConfig();       // Load and update behaivor of system
  }
}



//#################################--------------------------------------------- loop------------------------###################################
void loop()
{
  //  if (obj["lora"]["enable"].as<bool>())
  //  {
  //    //onReceive(LoRa.parsePacket());
  //  }

  if (millis() - mainRefresh > mainTime)
  {
    // ------------------------------------------- ergo
    if (obj["type"].as<String>() == "ergo")
    {
      if (obj["sensors_enable"].as<bool>())  // Sensor Panel normal
      {
        ReadSensors();
        //PrintOut();
        //SendData();
      }
    }
    // ------------------------------------------- cruz
    else if (obj["type"].as<String>() == "cruz")
    {
      read_clock();
      PrintOut();
      SendData();
    }
    mainRefresh = millis();
  }

  if (obj["wifi"]["sta"]["enable"].as<bool>())
  {
    // ----------------------------------------- check internet
    if ((millis() - s_timestamp) >= connectTimeoutMs) // check to an interval of time
    {
      checkServer();
      s_timestamp = millis();
    }
  }


  // ----------------------------------------- save new data
  if (saveConfig)  // Data change
  {
    Serial.println("{\"upload_config\":true}");
    saveConfigData();
    saveConfig = false;
  }



  // ---------------------------------------- wdt reset
  check_reset();
  esp_task_wdt_reset();
}
