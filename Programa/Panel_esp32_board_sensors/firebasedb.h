#ifndef  FIREBASEDB_H
#define FIREBASEDB_H

#include "system.h"

// Define Firebase Data object
extern FirebaseData fbdo;
extern FirebaseAuth auth;
extern FirebaseConfig config;
extern FirebaseJson json;
//extern FirebaseJson events_json;
extern FirebaseJson conf;
extern FirebaseJson data_json;
extern String nodeName;
extern volatile bool updated;
extern volatile bool dataChanged;
extern volatile bool nullData;
extern volatile bool saveConfig;
extern volatile bool fire_stream;
extern volatile bool update_events;
extern FirebaseData stream;
extern String route;


void fcsDownloadCallback(FCS_DownloadStatusInfo info);
void SendData();
void streamCallback(FirebaseStream data);
void prepareData();
void streamTimeoutCallback(bool timeout);
void connectFirebase();
void copyJsonObject(FirebaseJson& firebaseJson, JsonObject& jsonObject);
//void merge(JsonVariant dst, JsonVariantConst src);

#endif  // FIREBASEDB_H
