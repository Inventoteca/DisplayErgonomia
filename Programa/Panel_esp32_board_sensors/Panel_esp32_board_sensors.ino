/*
  Sensor Box
*/
#include "system.h"


//################################################################----------------------- setup--------------------- #############################
void setup() {
  Serial.begin(115200);
  Serial.print("{\"SensorBox_ver\":"); Serial.print(VERSION); Serial.println("}");
  WiFi.mode(WIFI_STA);
  pinMode(FACTORY_BT, INPUT);
  attachInterrupt(FACTORY_BT, factory_reset3, CHANGE);

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
  if ((obj["enable_lora"].as<bool>()) && (success))
  {
    //    //onReceive(LoRa.parsePacket());
    receive_lora();
  }



  if (millis() - mainRefresh > mainTime)
  {
    // ------------------------------------------- ergo
    if (obj["type"].as<String>() == "ergo")
    {



      if (obj["enable_sensors"].as<bool>())  // Sensor Panel normal
      {
        ReadSensors();

        if (obj["enable_rtc"].as<bool>())
          read_clock();

        if ((obj["enable_lora"].as<bool>()) && (success))
          send_lora();
      }

    }
    // ------------------------------------------- cruz
    else if (obj["type"].as<String>() == "cruz")
    {
      if (obj["enable_rtc"].as<bool>())
      {
        read_clock();
        PrintOut();
        SendData();
        if (obj["enable_lora"].as<bool>())
          send_lora();
      }
    }

    PrintOut();
    mainRefresh = millis();
  }

  if (obj["enable_wifi"].as<bool>())
  {
    // ----------------------------------------- check internet
    if ((millis() - s_timestamp) >= connectTimeoutMs) // check to an interval of time
    {
      checkServer();
      SendData();
      s_timestamp = millis();
    }
  }


  // ----------------------------------------- save new data
  if (saveConfig)  // Data change
  {
    Serial.println("{\"upload_config\":true}");
    saveConfigData();
    loadConfig();
    saveConfig = false;
    //ESP.restart();
  }



  // ---------------------------------------- wdt reset
  check_reset();
  esp_task_wdt_reset();
}
