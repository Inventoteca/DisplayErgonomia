#ifndef FILESPIFFS_H
#define FILESPIFFS_H



#include <ArduinoJson.h>
#include "SPIFFS.h"
#include "FS.h"

extern JsonObject obj;
extern StaticJsonDocument<1736> doc;
extern JsonArray dev;
extern StaticJsonDocument<512> dev_doc;
extern StaticJsonDocument<1736> conf_doc;

extern const char *device_list;
extern const char *filename;

extern File file;
//bool forceCleanONJsonError = true;

JsonObject getJSonFromFile(/*DynamicJsonDocument *doc*/ StaticJsonDocument<1736> *doc, String filename,bool forceCleanONJsonError = true);
JsonArray getJSonArrayFromFile(StaticJsonDocument<512> *dev_doc, String filename,bool forceCleanONJsonError = true);
/*static*/ void Cfg_get(/*struct jsonrpc_request * r*/);
bool saveJSonToAFile(JsonObject * doc, String filename);
bool saveJSonArrayToAFile(JsonArray * dev_doc, String filename);
void saveConfigData();

#endif  // FILESPIFFS_H
