#include "system.h"
#include "display.h"

bool factory_press = false;
unsigned long factory_time = 0;
unsigned long prev_factory_time = 0;
bool reset_time = false;
bool smart_config = false;
bool taskCompleted = false;
byte localAddress;  

// ----------------------------------------------------------------------------------------------- factory_reset
void IRAM_ATTR factory_reset1() 
{
  if (factory_press == false) {
    factory_press = true;
    factory_time = millis();
  }
  else {
    prev_factory_time = millis();
    reset_time = true;
  }

}

//----------------------------------------------------------------------------------------------------------- reset_config
void reset_config()
{
  WiFi.disconnect();
  //WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  obj["wifi"]["sta"]["ssid"] = "";
  obj["wifi"]["sta"]["pass"] = "";
  obj["wifi"]["sta"]["enable"] = true;
  obj["wifi"]["sta"]["count"] = 0;
  obj["wifi"]["sta"]["registered"] = false;
  Serial.println(saveJSonToAFile(&obj, filename) ? "{\"factory_reset\":true}" : "{\"factory_reset\":false}");
  neoConfig();
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

  taskCompleted = obj["update"].as<bool>();

  // ----------------------- WiFi STA
  if (obj["wifi"]["sta"]["enable"].as<bool>() == true /*&& (WiFi.status() != WL_CONNECTED)*/)
  {
    WiFi.mode(WIFI_STA);
    String auxssid = obj["wifi"]["sta"]["ssid"].as<String>();


    // ---- WiFi already Registered or try to connect a new wifi
    if ((obj["wifi"]["sta"]["registered"].as<bool>() == true) /*|| (obj["wifi"]["sta"]["count"] < 2)*/)
    {

      Serial.println("{\"load_registered_wifi\":true}");
      WiFi.begin(obj["wifi"]["sta"]["ssid"].as<const char*>(), obj["wifi"]["sta"]["pass"].as<const char*>());
      //Serial.print("WiFi connecting: \t");
      Serial.print("{\"wifi\":{\"ssid\":\"");
      Serial.print(obj["wifi"]["sta"]["ssid"].as<const char*>());
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
  else
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
  if (obj["neodisplay"]["enable"].as<bool>())
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

    strip.setPin(obj["neodisplay"]["pin"].as<int>());
    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.clear();
    strip.show();            // Turn OFF all pixels ASAP
    strip.setBrightness(255);
    last_ac = DateTime(obj["last_ac"].as<uint32_t>());
  }


  // ------------- ID and LoRa
  // check for id or mac is the config.json file
  String s_aux;
  s_aux = obj["id"].as<String>();
  int len = s_aux.length();
  int i_aux  = obj["lora"]["local"].as<int>();
  char aux_buf[50];


  if ((len == 0) && (i_aux == 0))
  {
    Serial.println("{\"update_id\":true}");
    Serial.print("{\"ID\":\"");
    Serial.print(WiFi.macAddress());
    Serial.println("\"}");


    obj["id"].set( WiFi.macAddress());
    // obj["id"] = WiFi.macAddress().c_str());

    Serial.println("{\"update_lora\":true}");
    strcpy(aux_buf, s_aux.c_str());
    localAddress = 0;

    for ( int i = 0; i < len; i++)
      localAddress += aux_buf[i];

    if ((localAddress == 0) || (localAddress == 255))localAddress = random(1, 254);
    obj["lora"]["local"].set(localAddress);
    //Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}" );

    //Serial.println( localAddress, HEX);
    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"lora_id_file_saved\":true}" : "{\"lora_id_file_saved\":false}" );
    serializeJson(obj, Serial);
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
  if (! rtc.begin()) {
    Serial.println("{\"rtc\":false}");
    Serial.flush();
    //while (1) delay(10);
  }
  else
    Serial.println("{\"rtc\":true}");

  Serial.println("{\"config\":true}");

}
