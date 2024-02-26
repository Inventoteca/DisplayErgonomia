#ifndef FILESPIFFS_H
#define FILESPIFFS_H

#include "system.h"

#define FILE_SIZE   2048

extern JsonObject obj;
extern StaticJsonDocument<FILE_SIZE> doc;
extern JsonArray dev;
extern StaticJsonDocument<FILE_SIZE> dev_doc;
//extern StaticJsonDocument<1023> conf_doc;
//extern JsonObject events_obj;
//extern StaticJsonDocument<1023> events_doc;

extern const char *device_list;
extern const char *filename;
extern const char *filedefault;
extern const char *sen_filename;
//extern const char *events_file;

extern File file;
//bool forceCleanONJsonError = true;

JsonObject getJSonFromFile(/*DynamicJsonDocument *doc*/ StaticJsonDocument<FILE_SIZE> *doc, String filename,bool forceCleanONJsonError = true);
JsonArray getJSonArrayFromFile(StaticJsonDocument<FILE_SIZE> *dev_doc, String filename,bool forceCleanONJsonError = true);
/*static*/ void Cfg_get(/*struct jsonrpc_request * r*/);
bool saveJSonToAFile(JsonObject * doc, String filename);
bool saveJSonArrayToAFile(JsonArray * dev_doc, String filename);
void saveConfigData();
bool spiffs_init();
//void saveEventsData();

#endif  // FILESPIFFS_H
