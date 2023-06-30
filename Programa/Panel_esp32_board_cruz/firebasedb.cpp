#include "firebasedb.h"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson json;
//FirebaseJson events_json;
FirebaseJson conf;
FirebaseJson data_json;
String nodeName;
volatile bool updated = true;
volatile bool dataChanged = false;
volatile bool nullData = false;
volatile bool saveConfig = false;
volatile bool fire_stream = false;
String route = "/panels/" + obj["id"].as<String>();// + "/actual";
FirebaseData stream;

// ----------------------------------- OTA The Firebase Storage download callback function
void fcsDownloadCallback(FCS_DownloadStatusInfo info)
{
  esp_task_wdt_reset();
  if (info.status == fb_esp_fcs_download_status_init)
  {
    Serial.printf("Downloading firmware %s (%d)\n", info.remoteFileName.c_str(), info.fileSize);
  }
  else if (info.status == fb_esp_fcs_download_status_download)
  {
    Serial.printf("Downloaded %d%s, Elapsed time %d ms\n", (int)info.progress, "%", info.elapsedTime);
  }
  else if (info.status == fb_esp_fcs_download_status_complete)
  {
    Serial.println("Update firmware completed.");
    Serial.println();
    Serial.println("Restarting...\n\n");

    obj["updated"] = true;
    saveConfigData();
    json.set("updated", true);
    SendData();


    delay(2000);
    ESP.restart();

  }
  else if (info.status == fb_esp_fcs_download_status_error)
  {
    Serial.printf("Download firmware failed, %s\n", info.errorMsg.c_str());
  }
}

//
//// Local time
//void printLocalTime()
//{
//  struct tm timeinfo;
//  if (!getLocalTime(&timeinfo)) {
//    Serial.println("Failed to obtain time");
//    return;
//  }
//  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");   // Comment for ESP8266
//}


// -------------------------------------------------------------------------------------- SendData
void SendData()
{
  if (Firebase.ready() && (WiFi.status() == WL_CONNECTED))
  {
    //json.set("updatedBySelf", true);
    prepareData();

    // ------------------------------------- ergo
    if (obj["type"].as<String>() == "ergo")
    {

    }
    // ------------------------------------- cruz
    else if (obj["type"].as<String>() == "cruz")
    {
      // Log file for reports Cruz
      // Actual readings file for reports Cruz
      //copyJsonObject(events_json, events_obj);
      //json.clear();
      //if (Firebase.RTDB.updateNode(&fbdo, route + "/events", &events_json) == false)
      //   Serial.printf("%s\n", fbdo.errorReason().c_str());

      //json.set("events", events_json);

      if (Firebase.RTDB.updateNode(&fbdo, route + "/actual", &json) == false)
        Serial.printf("%s\n", fbdo.errorReason().c_str());



      json.remove("Ts/.sv");
      json.remove("time");
      if (Firebase.RTDB.updateNode(&fbdo, route + "/data/" + String(now.year()) + "_" + String(now.month()), &json) == false)
      {
        Serial.printf("%s\n", fbdo.errorReason().c_str());
      }
    }

  }
  else if ((obj["enable_wifi"].as<bool>() == true) && (WiFi.status() == WL_CONNECTED))
  {
    connectFirebase();
  }

  if (obj["enable_lora"].as<bool>())
  {
    //sendMessage(message);
  }

}


// ---------------------------------------------------------------------------------------------------- streamCallback
void streamCallback(FirebaseStream data)
{
  // Add config from spiff file

  Serial.println("{\"stream\":true}");

  Serial.printf("stream path: %s\nevent path: %s\ndata type: %s\nevent type: %s\ndata:  %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str(),
                data.payload().c_str());

  if ((strcmp(data.dataPath().c_str(), "/") == 0) && (strcmp(data.eventType().c_str(), "patch") != 0))
  {
    //Serial.println("All obj");
    //DynamicJsonDocument doc(1024);

    // Llamar a deserializeJson() para decodificar la respuesta
    deserializeJson(doc, data.payload().c_str());
    serializeJson(obj, Serial);
    saveConfigData();
    loadConfig();
  }
}


// ------------------------------------------------------------------------------------------------------- prepareData
void prepareData()
{
  // ------------------------------------------ all
  json.clear();
  //json.set("updatedBySelf", true);
  route = "/panels/" + obj["id"].as<String>() ; //+ "/data/" + String(now.year()) + "_" + String(now.month());
  json.set("updated", obj["updated"].as<bool>());
  json.set("Ts/.sv", "timestamp"); // .sv is the required place holder for sever value which currently supports only string "timestamp" as a value

  // ------------------------------------------ ergo
  if (obj["type"].as<String>() == "ergo")
  {
    // aqui debe ir para cada nodeNAme
    nodeName = String(now.unixtime());


    DynamicJsonDocument msg(1024);
    String message;
    //message = "HeLoRa World!";   // send a message
    //msg["method"] =
    msg["sensors"]["t"] = t;
    msg["sensors"]["h"] = h;
    msg["sensors"]["uv"] = int(uv * 10); // avoid float
    msg["sensors"]["db"] = db;
    msg["sensors"]["lux"] = lux;
    msg["sensors"]["ppm"] = ppm;
    last_db = 0;

    json.set("t", t);
    json.set("h", h);
    json.set("uv", uv);
    json.set("db", db);
    json.set("lux", lux);
    json.set("ppm", ppm);
    serializeJson(msg, message);
    Serial.println(message);

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

  // ------------------------------------------ cruz
  else if (obj["type"].as<String>() == "cruz")
  {
    //route = "/panels/" + obj["id"].as<String>() + "/data/" + String(now.year()) + "_" + String(now.month());
    json.set("time", now.unixtime());
    json.set("last_ac", last_ac.unixtime());
    json.set("days_ac", dias);
    //json.set("gmtOff", obj["gmtOff"].as<long>());
    //json.set("dayOff", obj["dayOff"].as<int>());
    //json.set("ping",true);
    //JsonVariant eventsData = obj["events"];
    String jsonString;
    serializeJson(obj["events"], jsonString);
    FirebaseJson firebaseJson;
    firebaseJson.setJsonData(jsonString);
    json.set("events", firebaseJson);
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


void connectFirebase()
{
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
    auth.user.password = obj["firepass"].as<String>();

    /* Assign the RTDB URL (required) */
    config.database_url = obj["url"].as<String>();
    //esp_task_wdt_init(WDT_TIMEOUT*3, true);  //enable panic so ESP32 restarts

    Serial.println("{\"connecting_firebase\":true}");
    Firebase.begin(&config, &auth);
    esp_task_wdt_reset();
    Firebase.reconnectWiFi(true);

    while (!Firebase.ready());

    route = "/panels/" + obj["id"].as<String>();
    copyJsonObject(json, obj);


    if (!Firebase.RTDB.get(&fbdo, route + "/config")) {
      Serial.println("{\"config_file\":false}");
      // El nodo no existe, actualizamos.
      if (!Firebase.RTDB.updateNode(&fbdo, route + "/config", &json)) {
        Serial.printf("%s\n", fbdo.errorReason().c_str());
      } else {
        Serial.println("{\"upload_config\":true}");
      }
    }
    else
    {
      Serial.println("{\"get_config_file\":true}");
    }


    // Timeout, prevent to program halt
    config.timeout.wifiReconnect = 5 * 1000; // 10 Seconds to 2 min (10 * 1000)
    config.timeout.socketConnection = 30 * 1000; // 1 sec to 1 min (30 * 1000)
    config.timeout.sslHandshake = 2 * 60 * 1000; // 1 sec to 2 min (2 * 60 * 1000)
    config.timeout.rtdbKeepAlive = 20 * 1000;    // 20 sec to 2 min (45 * 1000)
    config.timeout.rtdbStreamReconnect = 1 * 1000;  //1 sec to 1 min (1 * 1000)
    config.timeout.rtdbStreamError = 3 * 1000;    // 3 sec to 30 sec (3 * 1000)
    config.timeout.serverResponse = 1 * 1000;    //Server response read timeout in ms 1 sec - 1 min ( 10 * 1000).

  }



  if ((!fire_stream) /*|| (!fire_events)*/)
  {
    if (!Firebase.RTDB.beginStream(&stream, route + "/config"))
      Serial.printf("config sream begin error, %s\n\n", stream.errorReason().c_str());
    else
    {
      Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
      fire_stream = true;
    }

  }


  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (!updated)
  {
    updated = true;
    String storage_id = obj["storage_id"].as<String>();

    // If you want to get download url to use with your own OTA update process using core update library,
    // see Metadata.ino example

    Serial.println("\nDownload firmware file...\n");

    // In ESP8266, this function will allocate 16k+ memory for internal SSL client.
    if (!Firebase.Storage.downloadOTA(&fbdo, storage_id/* Firebase Storage bucket id */, "Panel_esp32_board_cruz.ino.esp32da.bin" /* path of firmware file stored in the bucket */, fcsDownloadCallback /* callback function */))
      Serial.println(fbdo.errorReason());
  }
}


// ---------------------------------------------------------------------------------------------------- streamTimeoutCallback
void streamTimeoutCallback(bool timeout)
{
  // if (timeout)
  //   Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("{\"stream_error\":\"%s\"}\n\n", stream.errorReason().c_str());
}


// ----------------------------------------------------------------------------------------------------- copyJsonObject
void copyJsonObject(FirebaseJson & firebaseJson, JsonObject & jsonObject)
{
  for (JsonObject::iterator it = jsonObject.begin(); it != jsonObject.end(); ++it) {
    if (it->value().is<JsonObject>()) {
      FirebaseJson nestedFirebaseJson;
      JsonObject nestedJsonObject = it->value().as<JsonObject>();
      copyJsonObject(nestedFirebaseJson, nestedJsonObject);
      firebaseJson.set(it->key().c_str(), nestedFirebaseJson);
    }
    else if (it->value().is<String>()) {
      firebaseJson.set(it->key().c_str(), it->value().as<String>());
    }
    else if (it->value().is<int>()) {
      firebaseJson.set(it->key().c_str(), it->value().as<int>());
    }
    else if (it->value().is<float>()) {
      firebaseJson.set(it->key().c_str(), it->value().as<float>());
    }
    else if (it->value().is<bool>()) {
      firebaseJson.set(it->key().c_str(), it->value().as<bool>());
    }
    else if (it->value().is<long>()) {
      firebaseJson.set(it->key().c_str(), it->value().as<long>());
    }
    else if (it->value().is<double>()) {
      firebaseJson.set(it->key().c_str(), it->value().as<double>());
    }
    else {
      Serial.printf("Unsupported data type for key: %s\n", it->key().c_str());
    }
  }
}
