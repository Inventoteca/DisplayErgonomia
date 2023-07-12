#include "wifiservice.h"

//const uint32_t connectTimeoutMs = 10000;
//unsigned long  s_timestamp;
bool correct = false;
int wifi_trys;
boolean isSaved = false;


//------------------------------------------------------------------------------------------------ WiFiEvent
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{

  uint8_t ssid[65] = { 0 };
  uint8_t pass[65] = { 0 };
  uint8_t rvd_data[33] = { 0 };
  char email[33] = {0};

  //Serial.printf("[WiFi-event] event: %d\n", event);
  //Serial.printf("{\"wifi_event\":\"%d\"}", event);
  //Serial.println();

  switch (event)
  {

    //case ARDUINO_EVENT_SC_SCAN_DONE:
    //{
    //Serial.println("{\"wifi_event\":\"scan\"}");
    //color_status[0] = 255;
    //color_status[1] = 255;
    //color_status[2] = 255;
    //}
    //break;

    case ARDUINO_EVENT_SC_FOUND_CHANNEL:
      {
        //Serial.println("{\"wifi_event\":\"found\"}");
        //color_status[0] = 255;
        //color_status[1] = 0;
        //color_status[2] = 255;
      }
      break;

    case ARDUINO_EVENT_SC_GOT_SSID_PSWD:
      {

        //Serial.println("{\"wifi_event\":\"config\"}");
        //color_status[0] = 0;
        //color_status[1] = 0;
        //color_status[2] = 255;

        //if (info.sc_got_ssid_pswd.type == SC_TYPE_ESPTOUCH_V2)
        //{
        //  ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
        //  memcpy(email, rvd_data, sizeof(rvd_data));
        //}

        memcpy(pass, info.sc_got_ssid_pswd.password, sizeof(info.sc_got_ssid_pswd.password) + 1);
        memcpy(ssid, info.sc_got_ssid_pswd.ssid, sizeof(info.sc_got_ssid_pswd.ssid) + 1);

        if (obj["test"].as<bool>()) {
          Serial.printf("SSID:%s\n", ssid);
          Serial.printf("PASSWORD:%s\n", pass);
        }


        // Save config
        obj["ssid"] = ssid;
        obj["pass"] = pass;
        //obj["enable_wifi"] = true;
        //obj["count_wifi"] = 0;
        obj["registered_wifi"] = false;

        WiFi.stopSmartConfig();
        smart_config = false;

      }
      break;

    case ARDUINO_EVENT_SC_SEND_ACK_DONE:
      {
        //Serial.println("{\"wifi_event\":\"ack\"}");
        //color_status[0] = 0;
        //color_status[1] = 255;
        //color_status[2] = 0;
        obj["registered_wifi"] = true;
        Serial.println(saveJSonToAFile(&obj, filename) ? "{\"registered_wifi_saved\":true}" : "{\"registered_wifi_saved\":false}" );
      }
      break;
  }
  //return;
}

// -------------------------------------------------------------------- Wifi_disconnected
void Wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  //Serial.println("Disconnected from WIFI access point");
  //Serial.print("WiFi lost connection. Reason: ");
  //Serial.println(info.disconnected.reason);
  String auxssid = obj["ssid"].as<String>();
  if (auxssid.length() == 0)
  {
    Serial.println("Reconnecting...");
    WiFi.begin(obj["ssid"].as<const char*>(), obj["pass"].as<const char*>());
  }
  //return;
}



// ------------------------------------------------------------------------------------------------------- checkServer
void checkServer()
{

  //if ((millis() - s_timestamp) >= connectTimeoutMs) // check to an interval of time
  {
    String auxssid = obj["ssid"].as<String>();


    // ------------------ Wifi Connected
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("{\"wifi\":\"connected\"}");
      if (obj["enable_rtc"].as<bool>())
        update_clock();

      if (smart_config)
      {
        WiFi.stopSmartConfig();
        smart_config = false;
        Serial.println("{\"smart_config\":\"stop\"}");
      }

      if (obj["registered_wifi"] == false)
      {
        obj["registered_wifi"] = true;
        obj["count_wifi"] = 0;

        Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved_new_wifi\":true}" : "{\"file_saved\":false}");
        Serial.print("{\"wifi\":{\"ssid\":\"");
        Serial.print(obj["ssid"].as<const char*>());
        Serial.println("\"}}");
      }

      // MQTT Enable
      //      if (obj["mqtt"]["enable"].as<bool>())
      //      {
      //        if (!client.connected())
      //        {
      //          Serial.println("{\"mqtt_server\":\"reconnect\"}");
      //          blk = false;
      //          reconnect();
      //        }
      //        // Blink led status on printOut
      //        else
      //        {
      //          //Serial.println("{\"mqtt_server\":\"connected\"}");
      //          //if (obj["neodisplay"]["enable"].as<bool>())
      //          //{
      //          //  display1.updatePoint(obj["neodisplay"]["status"].as<int>(), 0, 255, 0);
      //          //  display1.show();
      //          // }
      //          blk = !blk;
      //        }
      //
      //      }

      connectFirebase();
    }

    else //wifi not connected
    {
      if (smart_config == false)
      {
        Serial.println("{\"wifi\":\"disconnected\"}");
        String auxssid = obj["ssid"].as<String>();
        if ((auxssid.length() > 0) /*&& (obj["wifi"]["sta"]["registered"].as<bool>() == true)*/)
        {
          Serial.println("{\"wifi\":\"reconnecting\"}");
          WiFi.begin(obj["ssid"].as<const char*>(), obj["pass"].as<const char*>());
          Serial.print("{\"wifi\":{\"ssid\":\"");
          Serial.print(obj["ssid"].as<const char*>());
          Serial.println("\"}}");
        }
      }
      else
      {
        Serial.println("{\"SmartConfig\":\"running\"}");
      }
    }

    //Serial.println();
    //PrintOut();
    //Serial.println("{\"server_check\":true}");
    Serial.printf("{\"status\":%d,%d,%d}", color_status[0], color_status[1], color_status[2]);
    Serial.println();
    //s_timestamp = millis();
  }
}



// ----------------------------------------------------------------------------------------- neoConfig
void neoConfig()
{

  //if (wifi_config == false) // Si aun no se inicia la config
  if (!WiFi.smartConfigDone() && (smart_config == false))
  {
    //if (obj["wifi"]["sta"]["count"] > 0)
    //{
    //  obj["wifi"]["sta"]["count"] = 0;
    //  Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved_wifi_count\":true}" : "{\"file_saved\":false}");
    //}

    WiFi.disconnect(true);

    //WiFi.mode(WIFI_STA);

    //if (obj["neodisplay"]["enable"].as<bool>())
    //{
    //display1.updatePoint(obj["neodisplay"]["status"], white); //no funciona
    //display1.updatePoint(obj["neodisplay"]["status"], 255, 255, 255); //no funciona, la libreria es u_int

    //display1.updatePoint(obj["neodisplay"]["status"].as<int>(), 255, 255, 255);
    //display1.show();

    //color_status[0] = 255;
    //color_status[1] = 255;
    //color_status[2] = 255;
    //}

    smart_config = WiFi.beginSmartConfig(SC_TYPE_ESPTOUCH_V2);

    Serial.print("{\"SmartConfig\":");
    Serial.print(smart_config);
    Serial.println("}");

    //while (!WiFi.smartConfigDone());
    //if (!wifi_config) ESP.restart();
  }
  //else // Configuracion iniciada
  //{
  //Serial.print("Wait conection response");
  //if (WiFi.smartConfigDone()) // Configuracion correcta
  //{
  //WiFi.stopSmartConfig();
  // wifi_config = false;
  // wifi_trys = 0;
  //Serial.print("SmartConfig Started Done");
  //}

}

//
//// -------------------------------------------------------------------------------------------------------------- ap_init
//void ap_Init(const char *ap_ssidx, const char *ap_passx)
//{
//  Serial.println("Starting AP");
//  IPAddress apIP(192, 168, 0, 1);   //Static IP for wifi gateway
//  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); //set Static IP gateway on NodeMCU
//  WiFi.softAP(ap_ssidx, ap_passx); //turn on WIFI
//  Serial.println("AP Ready");
//  Serial.println(ap_ssidx);
//  //websocketInit();
//}


// ----------------------------------------------------------------------------------------------------- websockerInit
void websocketInit()
{
  //webSocket.begin(); //websocket Begin
  //webSocket.onEvent(webSocketEvent); //set Event for websocket
  //Serial.println("Websocket is started");
}
