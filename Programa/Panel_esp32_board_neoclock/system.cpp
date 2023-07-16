#include "system.h"

#define   PRESS   LOW

bool factory_press = false;
unsigned long factory_time = 0;
unsigned long prev_factory_time = 0;
bool reset_time = false;
bool smart_config = false;
//bool taskCompleted = false;
byte localAddress;
int buttonState = 0;         // variable for reading the pushbutton status

const int sampleWindow = 50;                              // Sample window width in mS (50 mS = 20Hz)
int sample;
const int tempSample = 60000;                              // Sample interval for sensors

const int soundSample = 1500;                             // Sample window width in mS (50 mS = 20Hz)
unsigned long soundRefresh = 0;
const int airSample = 1000;                              // Sample window width in mS (50 mS = 20Hz)
unsigned long airRefresh = 0;
unsigned long startMillis = 0;

unsigned long mainRefresh = obj["mainTime"].as<uint32_t>();
unsigned long mainTime = 1000;

const uint32_t connectTimeoutMs = 10000;
unsigned long  s_timestamp;

unsigned long tiempoInicio;


// ----------------------------------------------------------------------------------------------- factory_reset3 change
void IRAM_ATTR factory_reset3()
{
  if ((factory_press == false) && (digitalRead(FACTORY_BT) == PRESS))
  {
    Serial.println("{\"R_bt\":\"push\"}");
    factory_press = true;
    factory_time = millis();
    return;

  }
  else if((digitalRead(FACTORY_BT) == !PRESS))
  {
    prev_factory_time = millis();
    reset_time = true;
    //Serial.println("{\"reset_button\":\"released\"}");
    return;
  }

}


// --------------------------------------------------------------------------------------------- check_reset
void check_reset()
{
  // Force Factory to input
  pinMode(FACTORY_BT, INPUT);

  if (reset_time)
  {
    Serial.print("{\"reset_time\":"); Serial.print(prev_factory_time - factory_time); Serial.println("}");
    if ((prev_factory_time - factory_time) > 5000)
    {
      reset_config();
    }
    else
      Serial.println("{\"reset\":\"fail\"}");
    factory_press = false;
    reset_time = false;
  }

  // ------------------------------------------------------reboot time es en horas
  int reboot_time = obj["reboot_time"].as<unsigned int>();
  if (reboot_time < 1)
    reboot_time = 24;
  // Si han pasado más de 24 horas del reset anterior o el tiempo en reboot time
  if (millis() - tiempoInicio >=  (reboot_time * 60 * 60 * 1000))
    //if (millis() - tiempoInicio >=  (reboot_time  * 1000))
  { // Comparar el tiempo actual con el tiempo de inicio
    Serial.print("{\"reboot_time\":"); Serial.print(obj["reboot_time"].as<unsigned int>()); Serial.println("}");
    tiempoInicio = millis();  // Actualizar el tiempo de inicio
    ESP.restart();  // Reiniciar el ESP32
  }

  //------------------------------------------------------ restart from command
  if (obj["restart"].as<bool>())
  {
    Serial.println("{\"reboot\":true}");
    SendData();
    obj["restart"] = false;
    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"reboot_save\":true}" : "{\"reboot_save\":false}");
    delay(2000);
    ESP.restart();
  }

}

//----------------------------------------------------------------------------------------------------------- reset_config
void reset_config()
{
  WiFi.disconnect();
  //WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);

  //obj["ssid"] = "";
  //obj["pass"] = "";
  //obj["enable_wifi"] = true;
  //obj["count_wifi"] = 0;
  //obj["registered_wifi"] = false;
  obj = getJSonFromFile(&doc, filedefault);
  Serial.println(saveJSonToAFile(&obj, filename) ? "{\"factory_reset\":true}" : "{\"factory_reset\":false}");
  delay(2000);
  ESP.restart();

}


// ------------------------------------------- strtoBool
bool strToBool(String str)
{
  if (str == "true" || str == "1") {
    return true;
  } else if (str == "false" || str == "0") {
    return false;
  } else {
    // handle invalid input
    return false;
  }
}

// ---------------------------------------------------------------------------------------------------- loadConfig
// Update a new config in the file an change behivor
void loadConfig()
{

  updated = obj["updated"].as<bool>();
  color = (obj["defColor"].as<uint32_t>() > 0 ? obj["defColor"].as<uint32_t>() : 0x00FF00);

  // ------------- ID
  String s_aux = obj["id"].as<String>();
  int len = s_aux.length();
  // check for id or mac is the config.json file
  if ((len == 0))
  {
    //(i_aux == 0)
    Serial.println("{\"update_id\":true}");
    Serial.print("{\"ID\":\"");
    Serial.print(WiFi.macAddress());
    Serial.println("\"}");

    obj["id"].set( WiFi.macAddress());
    // obj["id"] = WiFi.macAddress().c_str());

    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"id_file_saved\":true}" : "{\"id_file_saved\":false}" );
    serializeJson(obj, Serial);
  }

  // ----------------------- WiFi STA
  if (obj["enable_wifi"].as<bool>() == true && (WiFi.status() != WL_CONNECTED))
  {
    WiFi.mode(WIFI_STA);
    String auxssid = obj["ssid"].as<String>();
    String auxpass = obj["pass"].as<String>();


    // ---- WiFi already Registered and ssid pass is not empty or (try to connect a new wifi)
    if ((obj["registered_wifi"].as<bool>() == true) && (auxssid.length() > 0) && (auxpass.length() > 0) /*|| (obj["wifi"]["sta"]["count"] < 2)*/)
    {

      Serial.println("{\"load_registered_wifi\":true}");
      WiFi.begin(obj["ssid"].as<const char*>(), obj["pass"].as<const char*>());
      //Serial.print("WiFi connecting: \t");
      Serial.print("{\"wifi\":{\"ssid\":\"");
      Serial.print(obj["ssid"].as<const char*>());
      Serial.println("\"}}");
    }
    // ---- New WiFi or wrong configured
    else //if (((obj["wifi"]["sta"]["registered"].as<bool>() == false) && (obj["wifi"]["sta"]["count"] >= 2)) || (auxssid.length() == 0))
    {
      //
      //      if (auxssid.length() == 0){
      //        Serial.println("{\"load_new_wifi\":true}");}
      //      else{
      //        Serial.println("{\"wrong_new_wifi\":true}");}
      neoConfig();
    }
  }
  else if (obj["enable_wifi"].as<bool>() == false)
  {
    //
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("{\"wifi\":{\"enable\":false}}");
  }

  // --------------------------- OLED
  //if (obj["oled"].as<bool>())
  //{
  //    Heltec.display->clear();
  //    Heltec.display->drawString(0, 0, obj["name"]);
  //      Heltec.display->display();
  //Heltec.display->flipScreenVertically();
  //}

  // ------------------------- NeoDisplay
  if (obj["enable_neo"].as<bool>())
  {
    //Display Init
    if (neo_digits_status == false)
      displayInit();
  }

  //---------------- Type Init
  // ---------------- ergo
  if (obj["type"].as<String>() == "ergo")
  {
    if (obj["sensors_enable"].as<bool>())
    {
      // sensorInit();
    }
  }

  //---------------- cruz
  if (obj["type"].as<String>() == "cruz")
  {
    //last_ac = obj["last_ac"].as<DateTime>();
    //uint32_t last_ac_aux;
    last_ac = (obj["last_ac"].as<uint32_t>()) + (obj["gmtOff"].as<long>());
    //uint32_t last_ac_sec = static_cast<uint32_t>(last_ac_aux /* / 1000 */);
    //last_ac(last_ac_aux);
    //last_ac = obj["last_ac"];
    Serial.print("{\"last_ac\":\"");
    //Serial.print(last_ac);
    Serial.print(last_ac.unixtime());
    Serial.println("\"}");
  }

  //----------------- RTC
  if (obj["enable_rtc"].as<bool>())
  {
    rtcUpdated = false;
    ntpConnected = false;
    init_clock();
  }

  // ------------- LoRa
  if (obj["enable_lora"].as<bool>())
  {
    init_lora();
  }




  //
  //  if (obj["mqtt"]["enable"].as<bool>())
  //  {
  //    client.setBufferSize(1024);
  //    client.setServer(obj["mqtt"]["broker"].as<const char*>(), obj["mqtt"]["port"].as<unsigned int>());
  //    //client.setServer(obj["mqtt"]["broker"].as<const char*>(),1883);
  //    //client.setServer("inventoteca.com", 1883);
  //    client.setCallback(callback);
  //    // Serial.println(obj["mqtt"]["port"].as<unsigned int>());
  //
  //  }
  //Serial.println("Config Loaded");


  // -------------------------------- mainTime
  // refres time
  JsonVariant objTime = obj["mainTime"];
  if (objTime.isNull())
  {
    Serial.println("{\"mainTime\":NULL}");
    mainTime = 1000;
  }
  else
  {
    mainTime = obj["mainTime"].as<uint32_t>();
    Serial.print("{\"mainTime\":");
    Serial.print(mainTime);
    Serial.println("}");
  }
  mainRefresh = mainTime + 1;


  Serial.println("{\"config\":true}");

}
