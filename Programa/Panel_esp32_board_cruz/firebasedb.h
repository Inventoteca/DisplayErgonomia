#ifndef  FIREBASEDB_H
#define FIREBASEDB_H

#include <Firebase_ESP_Client.h>
//#include <addons/TokenHelper.h>         // Provide the token generation process info.
//#include <addons/RTDBHelper.h>          // Provide the RTDB payload printing info and other helper functions.
#include "system.h"
#include "sensors.h"

// Define Firebase Data object
extern FirebaseData fbdo;
extern FirebaseAuth auth;
extern FirebaseConfig config;
extern FirebaseJson json;
extern FirebaseJson json_events;
extern FirebaseJson conf;
extern FirebaseJson data_json;
extern String nodeName;
extern volatile bool dataChanged;
extern volatile bool nullData;
extern volatile bool saveConfig;
extern FirebaseData stream;


void fcsDownloadCallback(FCS_DownloadStatusInfo info);
void SendData();
void streamCallback(FirebaseStream data);
void prepareData();
void streamTimeoutCallback(bool timeout);

#endif  // FIREBASEDB_H