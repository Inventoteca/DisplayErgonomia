/*
  Sensor Box
*/
#include "system.h"


//################################################################----------------------- setup--------------------- #############################
void setup() 
{

   system_init();
   
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
      //checkServer();
      if (wifi_check())
      {
        SendData();
      }
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
