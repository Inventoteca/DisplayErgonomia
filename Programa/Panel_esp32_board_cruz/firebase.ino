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

    obj["update"] = true;
    saveConfigData();
    json.set("update", true);
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
  DynamicJsonDocument msg(1024);
  String message;
  if (obj["type"].as<String>() == "ergo")
  {

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

  }

  else if (obj["type"].as<String>() == "cruz")
  {
    json.set("days", int(round(round(now.unixtime() - last_ac.unixtime()) / 86400L)));
    json.set("last_ac", last_ac.unixtime());

    //DynamicJsonDocument event(512);
    //deserializeJson(event, value);
    //obj[key] = event;

    //json.set("events",obj["events"]);
    // json.set("update", true);
  }


  if (Firebase.ready() && (WiFi.status() == WL_CONNECTED))
  {

    // now we will set the timestamp value at Ts
    json.set("Ts/.sv", "timestamp"); // .sv is the required place holder for sever value which currently supports only string "timestamp" as a value
    //nodeName = String(millis());
    json.set("time", now.unixtime());

    DateTime now = rtc.now();
    nodeName = String(now.unixtime());

    // Set data with timestamp
    //Serial.printf("%s\n", Firebase.RTDB.updateNode(&fbdo, "/panels/01/actual", &json) ? /*fbdo.to<FirebaseJson>().raw()*/"" : fbdo.errorReason().c_str());

    String route = "/panels/" + obj["id"].as<String>() + "/actual";
    //Serial.println(route);

    //(String route = "/panels/40:91:51:93:45:B8/actual";

    if (Firebase.RTDB.updateNode(&fbdo, route, &json) == false)
    {
      Serial.printf("%s\n", fbdo.errorReason().c_str());
    }

    route = "/panels/" + obj["id"].as<String>() + "/data/" + String(now.year()) + "_" + String(now.month());

    if (Firebase.RTDB.updateNode(&fbdo, route, &json) == false)
    {
      Serial.printf("%s\n", fbdo.errorReason().c_str());
    }
  }
  else
  {
    Serial.println("{\"firebase\":false}");
  }

  if (obj["lora"]["enable"].as<bool>())
  {
    sendMessage(message);
  }

}


// ---------------------------------------------------------------------------------------------------- streamCallback
void streamCallback(FirebaseStream data)
{
  // Add config from spiff file

  saveConfig = false;
  if (strcmp(data.eventType().c_str(), "patch") != 0)  // Not an update
  {

    Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                  data.streamPath().c_str(),
                  data.dataPath().c_str(),
                  data.dataType().c_str(),
                  data.eventType().c_str());

    data_json.setJsonData(data.payload());
    String jsonStr;
    data_json.toString(jsonStr, false);

    if ((strcmp(data.eventType().c_str(), "put") == 0) &&  (strcmp(data.dataType().c_str(), "json") != 0))
    {
      saveConfig = true;
      if (strcmp(data.dataPath().c_str(), "/t_max") == 0)
        obj["t_max"] = jsonStr.toInt();
      if (strcmp(data.dataPath().c_str(), "/t_min") == 0)
        obj["t_min"] = jsonStr.toInt();
      if (strcmp(data.dataPath().c_str(), "/t_colMax") == 0)
        obj["t_colMax"] = strtoul(data.payload().c_str(), NULL, 10);
      if (strcmp(data.dataPath().c_str(), "/t_colMin") == 0)
        obj["t_colMin"] = strtoul(data.payload().c_str(), NULL, 10);
      if (strcmp(data.dataPath().c_str(), "/t_colDef") == 0)
        obj["t_colDef"] = strtoul(data.payload().c_str(), NULL, 10);


      if (strcmp(data.dataPath().c_str(), "/h_max") == 0)
        obj["h_max"] = jsonStr.toInt();
      if (strcmp(data.dataPath().c_str(), "/h_min") == 0)
        obj["h_min"] = jsonStr.toInt();
      if (strcmp(data.dataPath().c_str(), "/h_colMax") == 0)
        obj["h_colMax"] = strtoul(data.payload().c_str(), NULL, 10);
      if (strcmp(data.dataPath().c_str(), "/h_colMin") == 0)
        obj["h_colMin"] = strtoul(data.payload().c_str(), NULL, 10);
      if (strcmp(data.dataPath().c_str(), "/h_colDef") == 0)
        obj["h_colDef"] = strtoul(data.payload().c_str(), NULL, 10);


      if (strcmp(data.dataPath().c_str(), "/uv_max") == 0)
        obj["uv_max"] = jsonStr.toInt();
      if (strcmp(data.dataPath().c_str(), "/uv_min") == 0)
        obj["uv_min"] = jsonStr.toInt();
      if (strcmp(data.dataPath().c_str(), "/uv_colMax") == 0)
        obj["uv_colMax"] = strtoul(data.payload().c_str(), NULL, 10);
      if (strcmp(data.dataPath().c_str(), "/uv_colMin") == 0)
        obj["uv_colMin"] = strtoul(data.payload().c_str(), NULL, 10);
      if (strcmp(data.dataPath().c_str(), "/uv_colDef") == 0)
        obj["uv_colDef"] = strtoul(data.payload().c_str(), NULL, 10);


      if (strcmp(data.dataPath().c_str(), "/db_max") == 0)
        obj["db_max"] = jsonStr.toInt();
      if (strcmp(data.dataPath().c_str(), "/db_min") == 0)
        obj["db_min"] = jsonStr.toInt();
      if (strcmp(data.dataPath().c_str(), "/db_colMax") == 0)
        obj["db_colMax"] = strtoul(data.payload().c_str(), NULL, 10);
      if (strcmp(data.dataPath().c_str(), "/db_colMin") == 0)
        obj["db_colMin"] = strtoul(data.payload().c_str(), NULL, 10);
      if (strcmp(data.dataPath().c_str(), "/db_colDef") == 0)
        obj["db_colDef"] = strtoul(data.payload().c_str(), NULL, 10);


      if (strcmp(data.dataPath().c_str(), "/lux_max") == 0)
        obj["lux_max"] = jsonStr.toInt();
      if (strcmp(data.dataPath().c_str(), "/lux_min") == 0)
        obj["lux_min"] = jsonStr.toInt();
      if (strcmp(data.dataPath().c_str(), "/lux_colMax") == 0)
        obj["lux_colMax"] = strtoul(data.payload().c_str(), NULL, 10);
      if (strcmp(data.dataPath().c_str(), "/lux_colMin") == 0)
        obj["lux_colMin"] = strtoul(data.payload().c_str(), NULL, 10);
      if (strcmp(data.dataPath().c_str(), "/lux_colDef") == 0)
        obj["lux_colDef"] = strtoul(data.payload().c_str(), NULL, 10);

      if (strcmp(data.dataPath().c_str(), "/ppm_max") == 0)
        obj["ppm_max"] = jsonStr.toInt();
      if (strcmp(data.dataPath().c_str(), "/ppm_min") == 0)
        obj["pmm_min"] = jsonStr.toInt();
      if (strcmp(data.dataPath().c_str(), "/ppm_colMax") == 0)
        obj["ppm_colMax"] = strtoul(data.payload().c_str(), NULL, 10);
      if (strcmp(data.dataPath().c_str(), "/ppm_colMin") == 0)
        obj["ppm_colMin"] = strtoul(data.payload().c_str(), NULL, 10);
      if (strcmp(data.dataPath().c_str(), "/ppm_colDef") == 0)
        obj["ppm_colDef"] = strtoul(data.payload().c_str(), NULL, 10);

      if (strcmp(data.dataPath().c_str(), "/update") == 0)
      {
        bool valueBool = strToBool(data.payload().c_str());
        obj["update"] = valueBool;
        taskCompleted = valueBool;
        //obj["update"] = strtoul(data.payload().c_str(), NULL, 10);
      }

      if (strcmp(data.dataPath().c_str(), "/last_ac") == 0)
        obj["last_ac"] = strtoul(data.payload().c_str(), NULL, 10);

      if (strcmp(data.dataPath().c_str(), "/events") >= 0)//(key == ("events"))
      {
        DynamicJsonDocument event(512);
        deserializeJson(event, data.payload());
        String eventID = data.dataPath().substring(8);
        if (strcmp(data.dataType().c_str(), "null") == 0)
          obj["events"].remove(eventID);
        //obj["events"][eventID] = 0;
        else
          obj["events"][eventID] = event;

      }

      //saveConfigData();

    }

    if ((strcmp(data.eventType().c_str(), "put") == 0) &&  (strcmp(data.dataType().c_str(), "json") == 0) && (strcmp(data.dataType().c_str(), "null") != 0))
    {
      saveConfig = true;
      //    Serial.println(jsonStr);
      //deserializeJson(conf_doc,jsonStr);
      //printResult(data); // see addons/RTDBHelper.h
      FirebaseJson &json = data.jsonObject();
      String jsonStr;
      json.toString(jsonStr, true);
      Serial.println(jsonStr);
      size_t len = json.iteratorBegin();
      String key, value = "";
      int type = 0;
      for (size_t i = 0; i < len; i++)
      {
        json.iteratorGet(i, type, key, value);
        //Serial.print(i);
        //Serial.print(", ");
        //Serial.print("Type: ");
        //Serial.print(type);
        ////if (type == JSON_OBJECT)
        //{
        //  Serial.print(", Key: ");
        //  Serial.print(key);
        //}
        //Serial.print(", Value: ");
        //Serial.println(value);

        if (key == ("t_max"))
          obj[key] = value.toInt();
        if (key == ("t_min"))
          obj[key] = value.toInt();
        if (key == ("t_colDef"))
          obj[key] = strtoul(value.c_str(), NULL, 10);
        if (key == ("t_colMax"))
          obj[key] = strtoul(value.c_str(), NULL, 10);
        if (key == ("t_colMin"))
          obj[key] = strtoul(value.c_str(), NULL, 10);

        if (key == ("h_max"))
          obj[key] = value.toInt();
        if (key == ("h_min"))
          obj[key] = value.toInt();
        if (key == ("h_colDef"))
          obj[key] = strtoul(value.c_str(), NULL, 10);
        if (key == ("h_colMax"))
          obj[key] = strtoul(value.c_str(), NULL, 10);
        if (key == ("h_colMin"))
          obj[key] = strtoul(value.c_str(), NULL, 10);

        if (key == ("uv_max"))
          obj[key] = value.toInt();
        if (key == ("uv_min"))
          obj[key] = value.toInt();
        if (key == ("uv_colDef"))
          obj[key] = strtoul(value.c_str(), NULL, 10);
        if (key == ("uv_colMax"))
          obj[key] = strtoul(value.c_str(), NULL, 10);
        if (key == ("uv_colMin"))
          obj[key] = strtoul(value.c_str(), NULL, 10);

        if (key == ("db_max"))
          obj[key] = value.toInt();
        if (key == ("db_min"))
          obj[key] = value.toInt();
        if (key == ("db_colDef"))
          obj[key] = strtoul(value.c_str(), NULL, 10);
        if (key == ("db_colMax"))
          obj[key] = strtoul(value.c_str(), NULL, 10);
        if (key == ("db_colMin"))
          obj[key] = strtoul(value.c_str(), NULL, 10);

        if (key == ("lux_max"))
          obj[key] = value.toInt();
        if (key == ("lux_min"))
          obj[key] = value.toInt();
        if (key == ("lux_colDef"))
          obj[key] = strtoul(value.c_str(), NULL, 10);
        if (key == ("lux_colMax"))
          obj[key] = strtoul(value.c_str(), NULL, 10);
        if (key == ("lux_colMin"))
          obj[key] = strtoul(value.c_str(), NULL, 10);

        if (key == ("ppm_max"))
          obj[key] = value.toInt();
        if (key == ("ppm_min"))
          obj[key] = value.toInt();
        if (key == ("ppm_colDef"))
          obj[key] = strtoul(value.c_str(), NULL, 10);
        if (key == ("ppm_colMax"))
          obj[key] = strtoul(value.c_str(), NULL, 10);
        if (key == ("ppm_colMin"))
          obj[key] = strtoul(value.c_str(), NULL, 10);

        if (key == ("update"))
        {
          bool valueBool = strToBool(value);
          obj[key] = valueBool;
          taskCompleted = valueBool;
        }
        if (key == ("last_ac"))
          obj[key] = strtoul(value.c_str(), NULL, 10);
        if (key == ("events"))
        {
          DynamicJsonDocument event(512);
          deserializeJson(event, value);
          obj[key] = event;
        }
      }
      json.iteratorEnd();
      //saveConfigData();
    }
    else if (strcmp(data.dataType().c_str(), "null") == 0)
    {
      nullData = true;
      // Send spiffs to firebase
    }

  }
  else if (strcmp(data.dataPath().c_str(), "/") != 0)
  {
    if ((strcmp(data.dataPath().c_str(), "/events") >= 0) && (strcmp(data.dataType().c_str(), "json") == 0))//(key == ("events"))
    {
      Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                    data.streamPath().c_str(),
                    data.dataPath().c_str(),
                    data.dataType().c_str(),
                    data.eventType().c_str());

      FirebaseJson &json = data.jsonObject();
      String jsonStr;
      json.toString(jsonStr, true);
      Serial.println(jsonStr);
      size_t len = json.iteratorBegin();
      String key, value = "";
      int type = 0;
      for (size_t i = 0; i < len; i++)
      {
        json.iteratorGet(i, type, key, value);
        //if (key == ("events"))
        //{
        DynamicJsonDocument event(512);
        deserializeJson(event, value);
        obj["events"][key] = event;
        //}
      }
      json.iteratorEnd();
      saveConfig = true;
      //saveConfigData();

    }

  }

  //Serial.println();

  // This is the size of stream payload received (current and max value)
  // Max payload size is the payload size under the stream path since the stream connected
  // and read once and will not update until stream reconnection takes place.
  // This max value will be zero as no payload received in case of ESP8266 which
  // BearSSL reserved Rx buffer size is less than the actual stream payload.
  //Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());

  // Due to limited of stack memory, do not perform any task that used large memory here especially starting connect to server.
  // Just set this flag and check it status later.
  dataChanged = true;
}


// ---------------------------------------------------------------------------------------------------- streamTimeoutCallback
void streamTimeoutCallback(bool timeout)
{
  // if (timeout)
  //   Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}
