 /*
  * Panel Cruz 
*/

// -------------------------- library
#include <Arduino.h>

#include <MQUnifiedsensor.h>
#include <WiFi.h> //import for wifi functionality
#include <ArduinoJson.h>

#include "time.h"
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>         // Provide the token generation process info.
#include <addons/RTDBHelper.h>          // Provide the RTDB payload printing info and other helper functions.
#include <esp_task_wdt.h>
#include "RTClib.h"
#include "filespiffs.h"
#include "display.h"
#include "clock.h"
#include "pines.h"
#include "system.h"
#include "sensors.h"

//#include <WiFiMulti.h>
//#include <WebSocketsServer.h> //import for websocket
//#include <FirebaseJson.h>
//#include <WiFiUdp.h>
//#include "heltec.h"
//#include <Wire.h>


// --------------  DHT & MQ Definitions & Sound

//#define placa       ("ESP-32")
//#define Voltage_Resolution (3.3)
//#define mq_type     ("MQ-135") //MQ135
//#define ADC_Bit_Resolution (12) // For arduino UNO/MEGA/NANO
//#define RatioMQ135CleanAir (60)//RS / R0 = 3.6 ppm
//float cFactor = 0;


//15 seconds WDT
#define WDT_TIMEOUT 15

// ----------------- Objects



// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;
FirebaseJson json_events;
FirebaseJson conf;
FirebaseJson data_json;
String nodeName;
volatile bool dataChanged = false;
volatile bool nullData = false;
volatile bool saveConfig = false;
FirebaseData stream;

RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
DateTime now;
DateTime last_ac;




// --------- file variables
const char *filedefault = "/default.json";

const char *sen_filename = "/sensors.json";

bool correct = false;
int wifi_trys;
boolean isSaved = false;
char buf[1024];



// ---------------- timing
const int sampleWindow = 50;                              // Sample window width in mS (50 mS = 20Hz)
int sample;
const int tempSample = 60000;                              // Sample interval for sensors

const int soundSample = 1500;                             // Sample window width in mS (50 mS = 20Hz)
unsigned long soundRefresh = 0;
const int airSample = 1000;                              // Sample window width in mS (50 mS = 20Hz)
unsigned long airRefresh = 0;
unsigned long startMillis = 0;
unsigned long printRefresh = 0;
unsigned long printTime = 1000;
int contador;
const uint32_t connectTimeoutMs = 10000;
unsigned long  s_timestamp;
//static int ledOn = 0;  // Current LED status
int i;
unsigned int count;
uint8_t numx;
unsigned int bt_count;

//------------------- LoRa Variables
//unsigned int counter = 0;
String rssi = "RSSI --";
String packSize = "--";
String packet ;
String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages

byte destination = 6;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends
String message;
//char c_msg[30];

// ----------------- MQTT vars
unsigned long lastMsg = 0;

//------------------ Calendar





// ################################################################## CodeFunctions ###########################################




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

      if (obj["wifi"]["sta"]["registered"] == false)
      {
        obj["wifi"]["sta"]["registered"] = true;
        obj["wifi"]["sta"]["count"] = 0;

        Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved_new_wifi\":true}" : "{\"file_saved\":false}");
        Serial.print("{\"wifi\":{\"ssid\":\"");
        Serial.print(obj["wifi"]["sta"]["ssid"].as<const char*>());
        Serial.println("\"}}");
        if (smart_config)
        {
          WiFi.stopSmartConfig();
          smart_config = false;
        }
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

// ------------------------------------------------------------------------------------------------------- prepareData
void prepareData()
{

  if (obj["type"].as<String>() == "ergo")
  {
    json.set("t_max", obj["t_max"].as<int>());
    json.set("t_min", obj["t_min"].as<int>());
    json.set("t_colMax", obj["t_colMax"].as<uint32_t>());
    json.set("t_colMin", obj["t_colMin"].as<uint32_t>());
    json.set("t_colDef", obj["t_colDef"].as<uint32_t>());

    json.set("h_max", obj["h_max"].as<int>());
    json.set("h_min", obj["h_min"].as<int>());
    json.set("h_colMax", obj["h_colMax"].as<uint32_t>());
    json.set("h_colMin", obj["h_colMin"].as<uint32_t>());
    json.set("h_colDef", obj["h_colDef"].as<uint32_t>());

    json.set("uv_max", obj["uv_max"].as<int>());
    json.set("uv_min", obj["uv_min"].as<int>());
    json.set("uv_colMax", obj["uv_colMax"].as<uint32_t>());
    json.set("uv_colMin", obj["uv_colMin"].as<uint32_t>());
    json.set("uv_colDef", obj["uv_colDef"].as<uint32_t>());

    json.set("db_max", obj["db_max"].as<int>());
    json.set("db_min", obj["db_min"].as<int>());
    json.set("db_colMax", obj["db_colMax"].as<uint32_t>());
    json.set("db_colMin", obj["db_colMin"].as<uint32_t>());
    json.set("db_colDef", obj["db_colDef"].as<uint32_t>());

    json.set("lux_max", obj["lux_max"].as<int>());
    json.set("lux_min", obj["lux_min"].as<int>());
    json.set("lux_colMax", obj["lux_colMax"].as<uint32_t>());
    json.set("lux_colMin", obj["lux_colMin"].as<uint32_t>());
    json.set("lux_colDef", obj["lux_colDef"].as<uint32_t>());

    json.set("ppm_max", obj["ppm_max"].as<int>());
    json.set("ppm_min", obj["ppm_min"].as<int>());
    json.set("ppm_colMax", obj["ppm_colMax"].as<uint32_t>());
    json.set("ppm_colMin", obj["ppm_colMin"].as<uint32_t>());
    json.set("ppm_colDef", obj["ppm_colDef"].as<uint32_t>());
  }
  else if (obj["type"].as<String>() == "cruz")
  {
    json.set("last_ac", obj["last_ac"].as<uint32_t>());
    json.set("update", obj["update"].as<bool>());
  }

  //json.set("sensors", obj["sensors"]);
  //json.set("h", h);
  //json.set("uv", uv);
  //json.set("db", db);
  //json.set("lux", lux);
  //json.set("ppm", ppm);

  //json.set("sensors", t);
  // now we will set the timestamp value at Ts
  //json.set("Ts/.sv", "timestamp"); // .sv is the required place holder for sever value which currently supports only string "timestamp" as a value
  //nodeName = String(millis());;
  // Set data with timestamp
  //Serial.printf("%s\n", Firebase.RTDB.updateNode(&fbdo, "/panels/01/actual", &json) ? /*fbdo.to<FirebaseJson>().raw()*/"" : fbdo.errorReason().c_str());
}


//################################################################----------------------- setup--------------------- #############################
void setup()
{
  Serial.begin(115200);
  Wire.begin(); //
  // Interrupts events
  //WiFi.onEvent(Wifi_disconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(WiFiEvent);
  pinMode(FACTORY_BT, INPUT);
  attachInterrupt(FACTORY_BT, factory_reset1, CHANGE);

  // WatchDog Timer
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
  s_timestamp = millis();



  // SPIFFS Init
  if (!SPIFFS.begin(true))
  {
    Serial.println("{\"spiffs\":false}");
    return;
  }
  else
  {
    Serial.println("{\"spiffs\":true}");
    Cfg_get(/*NULL*/);
    loadConfig();
  }



}



//#################################--------------------------------------------- loop------------------------###################################
void loop()
{

  if (obj["wifi"]["sta"]["enable"].as<bool>())
  {
    checkServer();
  }

  if (reset_time)
  {
    if ((prev_factory_time - factory_time) > 5000)
    {
      reset_config();
    }
    factory_press = false;
    reset_time = false;
  }


  //  if (obj["lora"]["enable"].as<bool>())
  //  {
  //    //onReceive(LoRa.parsePacket());
  //  }



  if (obj["type"].as<String>() == "ergo")
  {
    if (obj["sensors_enable"].as<bool>()) // Sensor Panel normal
    {
      ReadSensors();
    }
  }
  else if (obj["type"].as<String>() == "cruz")
  {
    if (millis() - printRefresh > printTime)
    {
      now = rtc.now();
      PrintOut();
      SendData();
      printRefresh = millis();
    }

  }


  if (saveConfig) // Data change
  {
    Serial.println("{\"upload_config\":true}");
    saveConfigData();
    saveConfig = false;
  }

  esp_task_wdt_reset();

}
