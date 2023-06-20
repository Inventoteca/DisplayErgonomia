#include "wifiservice.h"

const uint32_t connectTimeoutMs = 10000;
unsigned long  s_timestamp;
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

    case ARDUINO_EVENT_SC_SCAN_DONE:
      {
        //Serial.println("{\"wifi_event\":\"scan\"}");
        color_status[0] = 255;
        color_status[1] = 255;
        color_status[2] = 255;
      }
      break;

    case ARDUINO_EVENT_SC_FOUND_CHANNEL:
      {
        //Serial.println("{\"wifi_event\":\"found\"}");
        color_status[0] = 255;
        color_status[1] = 0;
        color_status[2] = 255;
      }
      break;

    case ARDUINO_EVENT_SC_GOT_SSID_PSWD:
      {

        //Serial.println("{\"wifi_event\":\"config\"}");
        color_status[0] = 0;
        color_status[1] = 0;
        color_status[2] = 255;

        //if (info.sc_got_ssid_pswd.type == SC_TYPE_ESPTOUCH_V2)
        //{
        //  ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
        //  memcpy(email, rvd_data, sizeof(rvd_data));
        //}

        memcpy(pass, info.sc_got_ssid_pswd.password, sizeof(info.sc_got_ssid_pswd.password) + 1);
        memcpy(ssid, info.sc_got_ssid_pswd.ssid, sizeof(info.sc_got_ssid_pswd.ssid) + 1);

        //Serial.printf("SSID:%s\n", ssid);
        //Serial.printf("PASSWORD:%s\n", pass);

        // Save config
        obj["wifi"]["sta"]["ssid"] = ssid;
        obj["wifi"]["sta"]["pass"] = pass;
        //obj["wifi"]["sta"]["enable"] = true;
        //obj["wifi"]["sta"]["count"] = 0;
        obj["wifi"]["sta"]["registered"] = false;

        WiFi.stopSmartConfig();
        smart_config = false;

      }
      break;

    case ARDUINO_EVENT_SC_SEND_ACK_DONE:
      {
        //Serial.println("{\"wifi_event\":\"ack\"}");
        color_status[0] = 0;
        color_status[1] = 255;
        color_status[2] = 0;
        obj["wifi"]["sta"]["registered"] = true;
        Serial.println(saveJSonToAFile(&obj, filename) ? "{\"registered_wifi_saved\":true}" : "{\"registered_wifi_saved\":false}" );
      }
      break;
  }
}

// -------------------------------------------------------------------- Wifi_disconnected
void Wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  //Serial.println("Disconnected from WIFI access point");
  //Serial.print("WiFi lost connection. Reason: ");
  //Serial.println(info.disconnected.reason);
  String auxssid = obj["wifi"]["sta"]["ssid"].as<String>();
  if (auxssid.length() == 0)
  {
    Serial.println("Reconnecting...");
    WiFi.begin(obj["wifi"]["sta"]["ssid"].as<const char*>(), obj["wifi"]["sta"]["pass"].as<const char*>());
  }

}



// ------------------------------------------------------------------------------------------------------- checkServer
void checkServer()
{

  if ((millis() - s_timestamp) >= connectTimeoutMs) // check to an interval of time
  {
    String auxssid = obj["wifi"]["sta"]["ssid"].as<String>();


    // ------------------ Wifi Connected
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("{\"wifi\":\"connected\"}");
      update_clock();

      if (smart_config)
      {
        WiFi.stopSmartConfig();
        smart_config = false;
        Serial.println("{\"smart_config\":\"stop\"}");
      }

      if (obj["wifi"]["sta"]["registered"] == false)
      {
        obj["wifi"]["sta"]["registered"] = true;
        obj["wifi"]["sta"]["count"] = 0;

        Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved_new_wifi\":true}" : "{\"file_saved\":false}");
        Serial.print("{\"wifi\":{\"ssid\":\"");
        Serial.print(obj["wifi"]["sta"]["ssid"].as<const char*>());
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

      // Firebase
      //esp_task_wdt_reset();
      if (!Firebase.ready()) // Add more filters
      {
        /* Assign the api key (required) */
        // s_aux = obj["key"].as<String>();
        //len = s_aux.length();
        blk = false;
        Serial.println("{\"firebase_init\":true}");
        config.api_key = obj["key"].as<String>();

        /* Assign the user sign in credentials */
        auth.user.email = obj["email"].as<String>();
        auth.user.password = obj["pass"].as<String>();

        /* Assign the RTDB URL (required) */
        config.database_url = obj["url"].as<String>();
        Firebase.begin(&config, &auth);
        esp_task_wdt_reset();

        Firebase.reconnectWiFi(true);

        String route_config = "/panels/" + obj["id"].as<String>() + "/actual";
        if (!Firebase.RTDB.beginStream(&stream, route_config))
          Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

        Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

        // Timeout, prevent to program halt
        config.timeout.wifiReconnect = 10 * 1000; // 10 Seconds to 2 min (10 * 1000)
        config.timeout.socketConnection = 1 * 1000; // 1 sec to 1 min (30 * 1000)
        config.timeout.sslHandshake = 1 * 1000; // 1 sec to 2 min (2 * 60 * 1000)
        config.timeout.rtdbKeepAlive = 20 * 1000;    // 20 sec to 2 min (45 * 1000)
        config.timeout.rtdbStreamReconnect = 1 * 1000;  //1 sec to 1 min (1 * 1000)
        config.timeout.rtdbStreamError = 3 * 1000;    // 3 sec to 30 sec (3 * 1000)
        config.timeout.serverResponse = 10 * 1000;    //Server response read timeout in ms 1 sec - 1 min ( 10 * 1000).




      }
      else //if (Firebase.ready() /*&& !taskCompleted*/)
      {
        Serial.println("{\"firebase_connected\":true}");
        blk = !blk;

        if (dataChanged)
        {
          dataChanged = false;

          if (Firebase.ready() && (WiFi.status() == WL_CONNECTED))
          {
            if (nullData)
            {
              nullData = false;
              Serial.println("{\"upload_config\":true}");
              prepareData();
              String route_config = "/panels/" + obj["id"].as<String>() + "/actual";

              if (Firebase.RTDB.updateNode(&fbdo, route_config, &json) == false)
              {
                Serial.printf("%s\n", fbdo.errorReason().c_str());
              }
            }

          }

        }

        // Firebase.ready() should be called repeatedly to handle authentication tasks.

        if (!taskCompleted)
        {
          taskCompleted = true;
          String storage_id = obj["storage_id"].as<String>();

          // If you want to get download url to use with your own OTA update process using core update library,
          // see Metadata.ino example

          Serial.println("\nDownload firmware file...\n");

          // In ESP8266, this function will allocate 16k+ memory for internal SSL client.
          if (!Firebase.Storage.downloadOTA(&fbdo, storage_id/* Firebase Storage bucket id */, "firmware.bin" /* path of firmware file stored in the bucket */, fcsDownloadCallback /* callback function */))
            Serial.println(fbdo.errorReason());
        }
      }

    }
    else //wifi not connected
    {
      if (smart_config == false)
      {
        Serial.println("{\"wifi\":\"disconnected\"}");
        String auxssid = obj["wifi"]["sta"]["ssid"].as<String>();
        if ((auxssid.length() > 0) /*&& (obj["wifi"]["sta"]["registered"].as<bool>() == true)*/)
        {
          Serial.println("{\"wifi\":\"reconnecting\"}");
          WiFi.begin(obj["wifi"]["sta"]["ssid"].as<const char*>(), obj["wifi"]["sta"]["pass"].as<const char*>());
          Serial.print("{\"wifi\":{\"ssid\":\"");
          Serial.print(obj["wifi"]["sta"]["ssid"].as<const char*>());
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
    s_timestamp = millis();
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
