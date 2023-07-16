#include "filespiffs.h"

JsonObject obj;
StaticJsonDocument<FILE_SIZE> doc;
JsonArray dev;
StaticJsonDocument<FILE_SIZE> dev_doc;

const char* device_list = "/devices.json";
const char* filename = "/config.json";
const char *sen_filename = "/sensors.json";
const char *filedefault = "/default.json";

File file;

// ------------------------------------------------------------------------------------------------------ Cfg_get
/*static*/ void Cfg_get(/*struct jsonrpc_request * r*/)
//  {"method":"Config.Get"}
{
  // open file to load config

  obj = getJSonFromFile(&doc, filename);
  dev = getJSonArrayFromFile(&dev_doc, device_list);



  if (obj.size() == 0)
  {
    Serial.println("{\"config_file\":\"empty\"}");
    //return;
    obj = getJSonFromFile(&doc, filedefault);
    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_default_restore\":true}" : "{\"file_default_restore\":false}");
  }
  
  if (obj["test"].as<bool>() == true)
  {
    // Comment for production
    serializeJson(obj, Serial);
    Serial.println();
    serializeJson(dev, Serial);
    Serial.println();
  }



}


// --------------------------------------------------------------------------------------------------- saveJSonToAFile
bool saveJSonToAFile(JsonObject * doc, String filename) {
  //SD.remove(filename);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  //Serial.println(F("Open file in write mode"));
  file = SPIFFS.open(filename, FILE_WRITE);
  if (file) {
    //Serial.print(F("Filename --> "));
    //Serial.println(filename);

    //Serial.print(F("Start write..."));

    serializeJson(*doc, file);

    //Serial.print(F("..."));
    // close the file:
    file.close();
    //Serial.println(F("done."));

    return true;
  } else {
    // if the file didn't open, print an error:
    //Serial.print(F("Error opening "));
    //Serial.println(filename);

    return false;
  }
}


// ------------------------------------------------------------------------------------------------ getJsonFromFile

JsonObject getJSonFromFile(/*DynamicJsonDocument *doc*/ StaticJsonDocument<FILE_SIZE> *doc, String filename, bool forceCleanONJsonError)
{
  // open the file for reading:
  file = SPIFFS.open(filename);;
  if (file)
  {
    //Serial.println("Opening File");

    size_t size = file.size();
    //Serial.println(size);

    if (size > FILE_SIZE)
    {
      //Serial.println("Too large file");

    }

    DeserializationError error = deserializeJson(*doc, file);
    if (error)
    {
      // if the file didn't open, print an error:
      //Serial.print(F("Error parsing JSON "));
      //Serial.println(error.c_str());

      if (forceCleanONJsonError)
      {
        Serial.println("{\"force_empty_json\": true}");
        return doc->to<JsonObject>();
      }
    }

    // close the file:
    file.close();

    return doc->as<JsonObject>();
  } else {
    // if the file didn't open, print an error:
    //Serial.print(F("Error opening (or file not exists) "));
    //Serial.println(filename);

    //Serial.println(F("Empty json created"));
    Serial.println("{\"empty_json\": true}");
    return doc->to<JsonObject>();
  }

}


// ------------------------------------------------------------------------------------------------ getJsonArrayFromFile

JsonArray getJSonArrayFromFile(StaticJsonDocument<FILE_SIZE> *dev_doc, String filename, bool forceCleanONJsonError)
{
  // open the file for reading:
  file = SPIFFS.open(filename);;
  if (file)
  {
    //Serial.println("Opening File");

    size_t size = file.size();
    //Serial.println(size);

    if (size > FILE_SIZE)
    {
      //Serial.println("Too large file");
      //return false;
    }

    DeserializationError error = deserializeJson(*dev_doc, file);
    if (error)
    {
      // if the file didn't open, print an error:
      //Serial.print(F("Error parsing JSON "));
      //Serial.println(error.c_str());

      if (forceCleanONJsonError)
      {
        return dev_doc->to<JsonArray>();
      }
    }

    // close the file:
    file.close();

    return dev_doc->as<JsonArray>();
  } else {
    // if the file didn't open, print an error:
    //Serial.print(F("Error opening (or file not exists) "));
    //Serial.println(filename);

    //Serial.println(F("Empty json created"));
    return dev_doc->to<JsonArray>();
  }

}


// --------------------------------------------------------------------------------------------------- saveJSonArrayToAFile
bool saveJSonArrayToAFile(JsonArray * dev_doc, String filename)
{
  //SD.remove(filename);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  //Serial.println(F("Open file in write mode"));
  file = SPIFFS.open(filename, FILE_WRITE);
  if (file) {
    //Serial.print(F("Filename --> "));
    //Serial.println(filename);

    //Serial.print(F("Start write..."));

    serializeJson(*dev_doc, file);

    //Serial.print(F("..."));
    // close the file:
    file.close();
    //Serial.println(F("done."));

    return true;
  } else {
    // if the file didn't open, print an error:
    //Serial.print(F("Error opening "));
    //Serial.println(filename);

    return false;
  }
}

// --------------------------------------------------------------------------------------------------- saveConfigData
void saveConfigData()
{
  Serial.println(saveJSonToAFile(&obj, filename) ? "{\"config_update_spiffs\":true}" : "{\"conifg_update_spiffs\":false}");
  serializeJson(obj, Serial);
}
