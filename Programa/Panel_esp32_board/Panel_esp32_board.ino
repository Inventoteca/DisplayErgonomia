/* NeoDigito example code: Hello

  Print "Hello" on display, color select

  Created by Inventoteca

  https://github.com/Inventoteca/NeoDigito

  This example code is in the public domain.
  Remember that you must have installed Adafruit_NeoPixel library.
*/

// -------------------------- library
#include <Arduino.h>
#include <NeoDigito.h>
#include "DHT.h"
#include <MQUnifiedsensor.h>
#include <WiFi.h> //import for wifi functionality
//#include <WiFiMulti.h>
#include <WebSocketsServer.h> //import for websocket
#include "SPIFFS.h"
#include "FS.h"
#include <ArduinoJson.h>
//#include "mjson.h"  // Sketch -> Add File -> Add mjson.h
//#include <WiFiUdp.h>
//#include "heltec.h"
//#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
//#include <PubSubClient.h>
#include "time.h"
#include <Firebase_ESP_Client.h>
//#include <FirebaseJson.h>
#include <addons/TokenHelper.h>         // Provide the token generation process info.
#include <addons/RTDBHelper.h>          // Provide the RTDB payload printing info and other helper functions.
#include <esp_task_wdt.h>



// ------------- Pins Definitios
//LUX sensor pins are esp32 heltec lora v2 oled,
//      SCL =       15
//      SDA =       4
//    OLED_RST      16
//   LoRa_MISO      19
//   Lora_CS        18
//   LoRa_SCK       5
//   LoRa_RST       14
//   LoRa_MOSI      27
//   LoRa_DIO0      26
//   LoRa_DIO1      35
//   LoRa_DIO2      34
//          RX      3
//          TX      1
//      Prog        0
//      Prog        2
#define DHTPIN      4    // Digital pin, DHT sensor
#define VR_PIN      39    // Voltage referenci pin 3.3V
#define UV_PIN      34    // Analog input for UV sensor
#define MQ_PIN      36    // Analog input for Air sensor
#define NEO_PIN     25    // Pin where the display will be attached
#define Sound_RX    13    // gpio 16 es OLED_Reset
#define Sound_TX    12    // gpio 17 es neopixels
#define PIX_STATUS  17    // Decimal poimt as LED status
#define FACTORY_BT  0     // gpio 2 as factory reset button
//int gas_sensor = 36; //Sensor pin

// -------------- neodigits
#define DIGITS      17    // Number of NeoDigitos connected
#define PIXPERSEG   2    // NeoPixels per segment

// --------------  DHT & MQ Definitions & Sound
#define DHTTYPE     DHT22   // DHT 22  (AM2302), AM2321

//#define placa       ("ESP-32")
//#define Voltage_Resolution (3.3)
//#define mq_type     ("MQ-135") //MQ135
//#define ADC_Bit_Resolution (12) // For arduino UNO/MEGA/NANO
//#define RatioMQ135CleanAir (60)//RS / R0 = 3.6 ppm
//float cFactor = 0;



//--------------- LoRa definitios
#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6
#define NONE      0
#define SERIAL    1
#define LORA      2

//3 seconds WDT
#define WDT_TIMEOUT 15

// ----------------- Objects
NeoDigito display1 = NeoDigito(DIGITS, PIXPERSEG, NEO_PIN, NEO_GRB + NEO_KHZ800); // For more info abaut the last argumets check Adafruit_Neopixel documentation.
//NeoDigito display1 = NeoDigito(0, 0, -1);  // Params Loaded on loadConfig
DHT dht(DHTPIN, DHTTYPE);
//MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, mq_pin, mq_type);
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
sensors_event_t event;
JsonObject obj;
JsonObject conf;
JsonArray dev;
//JsonObject sen;
StaticJsonDocument<512> dev_doc;
StaticJsonDocument<1736> doc;
StaticJsonDocument<1736> conf_doc;
//StaticJsonDocument<1024> sen_doc;

File file;

//--------------- MQTT
//WiFiClient espClient;
//PubSubClient client(espClient);

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool taskCompleted = false;
FirebaseJson json;
String nodeName;
volatile bool dataChanged = false;
FirebaseData stream;

//WebSocketsServer webSocket = WebSocketsServer(81); //websocket init with port 81
//void webSocketEvent(uint8_t num, WStype_t w_type, uint8_t * payload, size_t length);
//WiFiMulti wifiMulti;



//Declare Sensor

//JsonObject getJSonFromFile(DynamicJsonDocument *doc, String filename, bool forceCleanONJsonError = true );

// ---------------- sensors variables
int t;
int h;
int db;
//double ppm;
unsigned int ppm;
float uv;
unsigned long lux;

// ------ auxiliar variables for sensors
float outputVoltage;
int adc;
int adc_33;
bool sensors_init = false;
char ch_db[10];
int last_db;
bool neo_digits_status = false;
bool tsl_status = false;
short color_status[3] = {0, 0, 0};

//mq-135
float m = -0.318; //Slope
float b = 1.133; //Y-Intercept
//float R0 = 11.820; //Sensor Resistance in fresh air f
float R0 = 9.000; //Sensor Resistance in fresh air f

// --------- file variables
const char *filedefault = "/default.json";
const char *filename = "/config.json";
const char *sen_filename = "/sensors.json";
const char *device_list = "/devices.json";
bool correct = false;
bool smart_config = false;
int wifi_trys;
boolean isSaved = false;
char buf[1024];
int rpc_input = NONE;


// ---------------- timing
const int sampleWindow = 50;                              // Sample window width in mS (50 mS = 20Hz)
int sample;
const int tempSample = 60000;                              // Sample interval for sensors
unsigned long tempRefresh = 0;
const int soundSample = 1500;                             // Sample window width in mS (50 mS = 20Hz)
unsigned long soundRefresh = 0;
const int airSample = 1000;                              // Sample window width in mS (50 mS = 20Hz)
unsigned long airRefresh = 0;
unsigned long startMillis = 0;
unsigned long printRefresh = 0;
unsigned long printTime = 1000;
unsigned long factory_time = 0;
unsigned long prev_factory_time = 0;
bool factory_press = false;
bool reset_time = false;
int contador;
const uint32_t connectTimeoutMs = 3000;
unsigned long  timestamp;
//static int ledOn = 0;  // Current LED status
int i;
unsigned int count;
uint8_t numx;
unsigned int bt_count;
bool blk = false;

//------------------- LoRa Variables
//unsigned int counter = 0;
String rssi = "RSSI --";
String packSize = "--";
String packet ;
String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
byte localAddress;     // address of this device
byte destination = 6;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends
String message;
//char c_msg[30];

// ----------------- MQTT vars
unsigned long lastMsg = 0;




// ################################################################## CodeFunctions ###########################################

//static void update_ota(struct jsonrpc_request * r) {
//  // Firebase.ready() should be called repeatedly to handle authentication tasks.
//
//  if (Firebase.ready() && !taskCompleted)
//  {
//    taskCompleted = true;
//
//    // If you want to get download url to use with your own OTA update process using core update library,
//    // see Metadata.ino example
//
//    Serial.println("\nDownload firmware file...\n");
//
//    // In ESP8266, this function will allocate 16k+ memory for internal SSL client.
//    if (!Firebase.Storage.downloadOTA(&fbdo, obj["firebase"]["bucked_id"].as<String>() /* Firebase Storage bucket id */, obj["firebase"]["firmware_path"].as<String>()  /* path of firmware file stored in the bucket */, fcsDownloadCallback /* callback function */))
//      Serial.println(fbdo.errorReason());
//  }
//}



//void fcsDownloadCallback(FCS_DownloadStatusInfo info)
//{
//  if (info.status == fb_esp_fcs_download_status_init)
//  {
//    Serial.printf("Downloading firmware %s (%d)\n", info.remoteFileName.c_str(), info.fileSize);
//  }
//  else if (info.status == fb_esp_fcs_download_status_download)
//  {
//    Serial.printf("Downloaded %d%s, Elapsed time %d ms\n", (int)info.progress, "%", info.elapsedTime);
//  }
//  else if (info.status == fb_esp_fcs_download_status_complete)
//  {
//    Serial.println("Update firmware completed.");
//    Serial.println();
//    Serial.println("Restarting...\n\n");
//    delay(2000);
//#if defined(ESP32) || defined(ESP8266)
//    ESP.restart();
//#elif defined(PICO_RP2040)
//    rp2040.restart();
//#endif
//  }
//  else if (info.status == fb_esp_fcs_download_status_error)
//  {
//    Serial.printf("Download firmware failed, %s\n", info.errorMsg.c_str());
//  }
//}
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

// ------------------------------------------------------------------------------ reconnect
//void reconnect() {
//  // Loop until we're reconnected
//  if (!client.connected())
//  {
//    //Serial.print("Attempting MQTT connection...");
//    // Create a random client ID
//    String clientId = "";
//    String userEmail = "";
//    String msgPanel = "";
//
//    // Attempt to connect
//    clientId = obj["id"].as<String>();
//    userEmail = "smart/users/" + obj["email"].as<String>() + "/add";
//    //msgPanel = "{\"id\":\"\" + clientId  + "",\"type\":\"" + obj["type"].as<String>() + "\",\"name\":\"" + obj["name"].as<String>() + "\",\"mod\":true}";
//
//    DynamicJsonDocument msg(1024);
//
//    msg["id"] = obj["id"];
//    msg["name"] = obj["name"];
//    msg["type"] = obj["type"];
//    msg["mod"] = true; // email registrated
//    serializeJson(msg, msgPanel);
//
//
//    if (client.connect(clientId.c_str()))
//    {
//      //Serial.println("connected");
//      // Once connected, publish an announcement...
//      if (obj["registered"].as<bool>()) // Device registered on user db
//      {
//        client.publish(userEmail.c_str(), "{}", false);
//      }
//      else
//      {
//        client.publish(userEmail.c_str(), msgPanel.c_str(), false);
//        Serial.println(msgPanel);
//      }
//
//      // ... and resubscribe
//      //  char buftop[100];
//      //  char bufsen[10];
//      //  const char* topicRoot = "test/panels/%s/%s";
//      //  sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "t");
//      //  sprintf(bufsen, "%i", t);
//      //client.publish(buftop, bufsen);
//
//
//      clientId = "smart/panels/" + clientId + "/app";
//      client.subscribe(clientId.c_str());
//      Serial.println("{\"mqtt\":true}");
//    }
//    else
//    {
//      //Serial.print("failed, rc=");
//      //Serial.print(client.state());
//      //Serial.println(" try again in 1 seconds");
//      // Wait 5 seconds before retrying
//      //delay(5000);
//      Serial.println("{\"mqtt\":false}");
//    }
//  }
//}

//// ----------------------------------------------------------------------------------- callback
//void callback(char* topic, byte* payload, unsigned int length) {
//
//  String dataConfig = "";
//
//  Serial.print("{\"mqtt_msg_rec\":{\"topic\":\"");
//  Serial.print(topic);
//  Serial.print("\",\"payload\":");
//  for (int i = 0; i < length; i++) {
//    Serial.print((char)payload[i]);
//    dataConfig += (char)payload[i];
//  }
//  Serial.print("}}");
//  Serial.println();
//
//  String msgPanel;
//  String userEmail  = "smart/users/" + obj["email"].as<String>() + "/add";
//  String clientId = "smart/panels/" + obj["id"].as<String>() + "/conf/response";
//
//  //Serial.println(dataConfig);
//  StaticJsonDocument<1024> docConf;
//  deserializeJson(docConf, dataConfig);
//  JsonObject objConf;
//  objConf = docConf.as<JsonObject>();
//
//
//  if (objConf.isNull())
//  {
//    // Switch on the LED if an 1 was received as first character
//    if ((char)payload[0] == '0') {
//      Serial.println("{\"user\":false}");
//      // it is active low on the ESP-01)
//    } else if ((char)payload[0] == '1')
//    {
//      serializeJson(obj, msgPanel);
//      Serial.println(msgPanel);
//      // No cabe toda la configuracion por mqtt al parecer
//      client.publish(clientId.c_str(), msgPanel.c_str(), false);
//      Serial.println("{\"user\":true}");
//    }
//
//  }
//  else if (objConf["method"] == "Config.Set")
//  {
//    serializeJsonPretty(objConf, Serial);
//    obj = objConf["params"];
//    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
//  }
//
//  //DynamicJsonDocument msg(1024);
//
//
//
//
//  if (obj["registered"] == false)
//  {
//    client.publish(userEmail.c_str(), "{}", false);
//    obj["registered"] = true;
//    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
//  }
//
//
//
//}


// -------------------------------------------------------------------------------------- lora_send
// {"method":"LoRa.Send","params":{"method":"Config.Get"}
// {"method":"LoRa.Send","params":{"method":"Config.Get","params":"\"{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}\""}}
static void lora_send(struct jsonrpc_request *r)
{

  //  char cmd[100];
  //  char cmd_params[100];
  //  StaticJsonDocument<124> doc_cmd;
  //  const char *buf;  // Get `b` sub-object
  //  int len;
  //  StaticJsonDocument<124> doc_params;
  //  //StaticJsonDocument<124> doc_aux;
  //  String s_cmd;
  //  //JsonObject cmd_params = doc_cmd.createNestedObject("params");
  //
  //
  //
  //
  //  if (mjson_get_string(r->params, r->params_len, "$.method", cmd, sizeof(cmd)) > 0)
  //  {
  //    doc_cmd["method"] = cmd;
  //    //Serial.println(cmd);
  //    //serializeJsonPretty(doc_cmd, Serial);
  //    if (mjson_find(r->params, r->params_len, "$.params", &buf, &len))
  //    {
  //      //Serial.println(buf);
  //      deserializeJson(doc_params, buf);
  //      doc_cmd["params"] = doc_params;
  //      //JsonObject root = doc_aux.createNestedObject();
  //      //
  //
  //      //
  //
  //      //
  //
  //      //root["params"] = doc_params;
  //      //serializeJsonPretty(doc_params, Serial);
  //      //doc_cmd["params"].set(root);
  //      //message = cmd;
  //      //
  //
  //    }
  //    serializeJson(doc_cmd, s_cmd);
  //    Serial.println(s_cmd);
  //    sendMessage(s_cmd);
  //    //serializeJsonPretty(doc_cmd, Serial);
  //
  //  }
  //
  //
  //
  //  if (obj["oled"].as<bool>())
  //  {
  //    //Heltec.display->clear();
  //    //Heltec.display->display();
  //    //    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  //    //
  //    //
  //    //    Heltec.display->drawString(0, 0, "Enviado");
  //    //
  //    //    Heltec.display->drawString(0, 10, s_cmd);
  //    //    Heltec.display->display();
  //  }


}


// -------------------------------------------------------------------------------------- SendData
void SendData()
{
  DynamicJsonDocument msg(1024);
  String message;
  //message = "HeLoRa World!";   // send a message
  //msg["method"] =
  msg["sensors"]["t"] = t;
  msg["sensors"]["h"] = h;
  msg["sensors"]["uv"] = uv;
  msg["sensors"]["db"] = db;
  msg["sensors"]["lux"] = lux;
  msg["sensors"]["ppm"] = ppm;
  last_db = 0;
  serializeJson(msg, message);
  Serial.println(message);
  if (obj["lora"]["enable"].as<bool>())
  {
    sendMessage(message);
  }

  if (Firebase.ready() && (WiFi.status() == WL_CONNECTED))
  {
    json.set("t", t);
    json.set("h", h);
    json.set("uv", uv);
    json.set("db", db);
    json.set("lux", lux);
    json.set("ppm", ppm);
    // now we will set the timestamp value at Ts
    //json.set("Ts/.sv", "timestamp"); // .sv is the required place holder for sever value which currently supports only string "timestamp" as a value
    //nodeName = String(millis());;
    // Set data with timestamp
    //Serial.printf("%s\n", Firebase.RTDB.updateNode(&fbdo, "/panels/01/actual", &json) ? /*fbdo.to<FirebaseJson>().raw()*/"" : fbdo.errorReason().c_str());

    String route = "/panels/" + obj["id"].as<String>() + "/actual";
    //Serial.println(route);

    //(String route = "/panels/40:91:51:93:45:B8/actual";

    if (Firebase.RTDB.updateNode(&fbdo, route, &json) == false)
    {
      Serial.printf("%s\n", fbdo.errorReason().c_str());
    }
  }
  else
  {
    Serial.println("{\"firebase\":false}");
  }


}


// ------------------------------------------------------------------------------------------- onReceive
void onReceive(int packetSize)
{
  //
  //  if (packetSize == 0) return;          // if there's no packet, return
  //  //Serial.println();
  //  //Serial.println("LoRa:");
  //
  //  // read packet header bytes:
  //  int recipient = LoRa.read();          // recipient address
  //  byte sender = LoRa.read();            // sender address
  //  byte incomingMsgId = LoRa.read();     // incoming msg ID
  //  byte incomingLength = LoRa.read();    // incoming msg length
  //
  //  String incoming = "";
  //  unsigned int i = 0;
  //
  //  while (LoRa.available()) {
  //    //incoming += (char)LoRa.read();
  //    buf[i] = LoRa.read();
  //    incoming += (char)buf[i];
  //    i++;
  //  }
  //
  //  if (incomingLength != incoming.length()) {   // check length for error
  //    Serial.println("error: message length does not match length");
  //    // return;                             // skip rest of function
  //  }
  //
  //  // if the recipient isn't this device or broadcast,
  //  //if (recipient != localAddress && recipient != 0xFF)
  //  if (recipient != obj["lora"]["local"] && recipient != 0xFF)
  //  {
  //    //Serial.println("{\"lora_rec\":true}");
  //
  //    //Serial.println("This message is not for me.");
  //    Serial.println("LoRa: " + String(sender) + "->" + String(recipient));
  //    //Serial.println("Sent to: " + String(recipient));
  //
  //    // Serial.println("Message ID: " + String(incomingMsgId));
  //    // Serial.println("Message length: " + String(incomingLength));
  //    // Serial.println("Message: ");
  //    // serializeJsonPretty(objx, Serial);
  //    // Serial.println("RSSI: " + String(LoRa.packetRssi()));
  //    // Serial.println("Snr: " + String(LoRa.packetSnr()));
  //    // Serial.println();
  //    //return;                             // skip rest of function
  //  }
  //  else
  //  {
  //    // Json RPC
  //    StaticJsonDocument<1024> docx;
  //    //JsonObject root = docx.createNestedObject(F(dev_id));
  //    deserializeJson(docx, incoming);
  //    JsonObject objx = docx.as<JsonObject>();
  //    //
  //
  //    if (objx.isNull())
  //      Serial.println("objx is null");
  //    else
  //    {
  //      if (objx["method"].isNull())
  //      {
  //        //Serial.println("method is null");
  //      }
  //      else
  //      {
  //        Serial.println("method Received");
  //        rpc_input = LORA;
  //        jsonrpc_process(buf, sizeof(buf), senderLoRa, NULL, NULL);
  //        rpc_input = NONE;
  //
  //      }
  //
  //      if (objx["error"].isNull());
  //      //Serial.println("error is null");
  //      else
  //      {
  //        Serial.println("error Received");
  //      }
  //
  //      if (objx["result"].isNull());
  //      //Serial.println("result is null");
  //      else
  //      {
  //        Serial.println("Result Received");
  //      }
  //
  //      if (objx["sensors"].isNull());
  //      //Serial.println("sensors is null");
  //      else
  //      {
  //        // Serial.println("Sensors Received");
  //        t = objx["sensors"]["t"];
  //        h = objx["sensors"]["h"];
  //        uv = objx["sensors"]["uv"];
  //        db = objx["sensors"]["db"];
  //        lux = objx["sensors"]["lux"];
  //        ppm = objx["sensors"]["ppm"];
  //        //PrintOut();
  //
  //        serializeJson(objx, Serial);
  //        Serial.println();
  //        //Serial.println(buf);
  //        //client.publish("smartOut", buf);
  //
  //        if (obj["oled"].as<bool>() || obj["neodisplay"]["enable"].as<bool>())
  //          PrintOut();
  //
  //        if (obj["wifi"]["sta"]["enable"].as<bool>() && (obj["mqtt"]["enable"].as<bool>()) && (client.connected()))
  //        {
  //          char buftop[100];
  //          char bufsen[100];
  //          const char* topicRoot = "smart/panels/%s/sensors/%s";
  //
  //
  //          sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "t");
  //          sprintf(bufsen, "%i", t);
  //          client.publish(buftop, bufsen);
  //
  //          sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "h");
  //          sprintf(bufsen, "%i", h);
  //          client.publish(buftop, bufsen);
  //
  //          sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "uv");
  //          sprintf(bufsen, "%.1f", uv);
  //          client.publish(buftop, bufsen);
  //
  //          sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "db");
  //          sprintf(bufsen, "%i", db);
  //          client.publish(buftop, bufsen);
  //
  //          sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "lux");
  //          sprintf(bufsen, "%i", lux);
  //          client.publish(buftop, bufsen);
  //
  //          sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "ppm");
  //          sprintf(bufsen, "%i", ppm);
  //          client.publish(buftop, bufsen);
  //
  //
  //          // All object channel
  //          sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "all");
  //          //sprintf(bufsen, "%i", t);
  //          client.publish(buftop, buf);
  //
  //          //printLocalTime();
  //
  //          Serial.println("{\"mqtt_send\":true}");
  //        }
  //        else
  //          Serial.println("{\"mqtt_send\":false}");
  //
  //      }
  //    }
  //
  //    // if message is for this device, or broadcast, print details:
  //
  //
  //
  //    //Heltec.display->clear();
  //    //Heltec.display->display();
  //    //Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  //    //Heltec.display->drawString(0, 0, incoming);
  //    //Heltec.display->display();
  //
  //  }


}


//----------------------------------------------------------------------------------------- sendMessage
void sendMessage(String outgoing)
{
  //  if (dev.size() >= 0) // list of devices
  //  { //  Serial.println("Sending LoRa...");
  //    for (JsonArray::iterator it = dev.begin(); it != dev.end(); ++it)
  //    {
  //      if ((*it)["network"] == "lora")
  //      {
  //        //digitalWrite(25, HIGH);
  //        LoRa.beginPacket();                   // start packet
  //        //if (strcmp((*it)["id"], dev_id) == 0)
  //        LoRa.write((*it)["local"]);              // add destination address
  //        LoRa.write(obj["lora"]["local"]);             // add sender address
  //        LoRa.write(msgCount);                 // add message ID
  //        LoRa.write(outgoing.length());        // add payload length  //aqui debo cambiar el print para ampliar el maximo
  //        LoRa.print(outgoing);                 // add payload7
  //        LoRa.endPacket();                     // finish packet and send it
  //        msgCount++;                           // increment message ID
  //        Serial.println("{\"lora_send\":true}");
  //        //digitalWrite(25, LOW);
  //      }
  //      //else
  //      //  Serial.println("{\"lora_send\":false}");
  //    }
  //  }
  //  else
  {
    Serial.println("{\"lora_send\":false}");
  }



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


// --------------------------------------------------------------------------------------- PrintOut
void PrintOut()
{
  //if (millis() - printRefresh > printTime)
  {
    if (obj["oled"].as<bool>())
    {
      //      Heltec.display->clear();
      //      Heltec.display->display();
      //      Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
      //      Heltec.display->setFont(ArialMT_Plain_16);
      //
      //
      //      Heltec.display->drawString(0, 0, obj["name"]);
      //
      //      //Heltec.display->setFont(ArialMT_Plain_10);
      //      Heltec.display->drawString(0, 16, String(t));
      //      Heltec.display->drawString(25, 16, "°C");
      //      Heltec.display->drawString(0, 32, String(h));
      //      Heltec.display->drawString(25, 32, "%");
      //      Heltec.display->drawString(0, 48, String(uv, 1));
      //      Heltec.display->drawString(25, 48, "uv");
      //
      //
      //      Heltec.display->drawString(60, 16, String(db));
      //      Heltec.display->drawString(90, 16, "dB");
      //      Heltec.display->drawString(55, 32, String(lux));
      //      Heltec.display->drawString(95, 32, "Lux");
      //      Heltec.display->drawString(55, 48, String(ppm));
      //      Heltec.display->drawString(95, 45, "ppm");
      //
      //      Heltec.display->display();
    }


    if (obj["neodisplay"]["enable"].as<bool>())
    {
      display1.setCursor(0);
      if (t < 10)
        display1.print(" ");
      display1.print(t, (t >= (obj["sensors"]["t"]["max"].as<int>())) ?
                     (obj["sensors"]["t"]["colMax"].as<uint32_t>()) : (t <= (obj["sensors"]["t"]["min"].as<int>())) ?
                     (obj["sensors"]["t"]["colMin"].as<uint32_t>()) : (obj["sensors"]["t"]["colDef"].as<uint32_t>()));
      display1.setCursor(2);
      if (h < 10)
        display1.print(" ");
      display1.print(h, (h >= (obj["sensors"]["h"]["max"].as<int>())) ?
                     (obj["sensors"]["h"]["colMax"].as<uint32_t>()) : (h <= (obj["sensors"]["h"]["min"].as<int>())) ?
                     (obj["sensors"]["h"]["colMin"].as<uint32_t>()) : (obj["sensors"]["h"]["colDef"].as<uint32_t>()));
      display1.setCursor(4);
      if (uv == 0)
        display1.print("0.0", (uv >= (obj["sensors"]["uv"]["max"].as<int>())) ?
                       (obj["sensors"]["uv"]["colMax"].as<uint32_t>()) : (uv <= (obj["sensors"]["uv"]["min"].as<int>())) ?
                       (obj["sensors"]["uv"]["colMin"].as<uint32_t>()) : (obj["sensors"]["uv"]["colDef"].as<uint32_t>()));
      else
        display1.print(String(uv, 1), (uv >= (obj["sensors"]["uv"]["max"].as<int>())) ?
                       (obj["sensors"]["uv"]["colMax"].as<uint32_t>()) : (uv <= (obj["sensors"]["uv"]["min"].as<int>())) ?
                       (obj["sensors"]["uv"]["colMin"].as<uint32_t>()) : (obj["sensors"]["uv"]["colDef"].as<uint32_t>()));
      display1.setCursor(6);
      if (db < 10)
        display1.print("  ");
      else if (db < 100)
        display1.print(" ");
      display1.print(db, (db >= (obj["sensors"]["db"]["max"].as<int>())) ?
                     (obj["sensors"]["db"]["colMax"].as<uint32_t>()) : (db <= (obj["sensors"]["db"]["min"].as<int>())) ?
                     (obj["sensors"]["db"]["colMin"].as<uint32_t>()) : (obj["sensors"]["db"]["colDef"].as<uint32_t>()));
      display1.setCursor(9);
      if (lux < 10)
        display1.print("   ");
      else if (lux < 100)
        display1.print("  ");
      else if (lux < 1000)
        display1.print(" ");
      display1.print(lux, (lux >= (obj["sensors"]["lux"]["max"].as<int>())) ?
                     (obj["sensors"]["lux"]["colMax"].as<uint32_t>()) : (lux <= (obj["sensors"]["lux"]["min"].as<int>())) ?
                     (obj["sensors"]["lux"]["colMin"].as<uint32_t>()) : (obj["sensors"]["lux"]["colDef"].as<uint32_t>()));
      display1.setCursor(13);
      if (ppm < 10)
        display1.print("   ");
      else if (ppm < 100)
        display1.print("  ");
      else if (ppm < 1000)
        display1.print(" ");
      else if (ppm > 9999)ppm = 9999;
      display1.print(ppm, (ppm >= (obj["sensors"]["ppm"]["max"].as<int>())) ?
                     (obj["sensors"]["ppm"]["colMax"].as<uint32_t>()) : (ppm <= (obj["sensors"]["ppm"]["min"].as<int>())) ?
                     (obj["sensors"]["ppm"]["colMin"].as<uint32_t>()) : (obj["sensors"]["ppm"]["colDef"].as<uint32_t>()));


      // ------------------------------------- Status BLink
      if (WiFi.status() == WL_CONNECTED)
      {
        if (blk == true)
        {
          display1.updatePoint(obj["neodisplay"]["status"].as<int>(), obj["sensors"]["ppm"]["colDef"].as<uint32_t>());
          blk = false;
          color_status[0] = color_status[1] = color_status[2] = 1;
          //display1.show();
        }
        else
        {
          display1.updatePoint(obj["neodisplay"]["status"].as<int>(), 0, 0, 0);
          blk = true;
          color_status[0] = color_status[1] = color_status[2] = 0;
          //display1.show();
        }
      }
      // ------------------------------------- Status WiFiEvent
      else
      {
        display1.updatePoint(obj["neodisplay"]["status"].as<int>(), color_status[0], color_status[1], color_status[2]);
      }


      display1.show();

    }

    // Prepare for LoRa
    //SendData();
    printRefresh = millis();

  }

}


// ----------------------------------------------------------------------------------- mapFloat
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


// --------------------------------------------------------------------------------ReadSensors()
void ReadSensors()
{

  if (obj["sensors"]["enable"] == true) // Sensors available
  {
    if (sensors_init == false)          // Sensors not already init
    {
      sensorInit();
    }

    //----------------------------- Sound
    //    if (!Serial1.available())
    //    {
    //      //Serial.println("Send");
    //      i = 0;
    //      db = 0;
    //      Serial1.write(0x01);
    //      Serial1.write(0x03);
    //      Serial1.write(0x00);
    //      Serial1.write(0x00);
    //      Serial1.write(0x00);
    //      Serial1.write(0x01);
    //      Serial1.write(0x84);
    //      Serial1.write(0x0A);
    //      while (!Serial1.available());
    //      while (Serial1.available())
    //      {
    //        buf[i] = Serial1.read();
    //        //Serial.println(ch[i], HEX);
    //        i++;
    //
    //        if (i >= 7)
    //        {
    //          i = 0;
    //          //Serial.print(ch[3],HEX);
    //          //Serial.println(ch[4],HEX);
    //          db = ((buf[3] * 256) + buf[4]) / 10;
    //          //Serial.println(db);
    //        }
    //      }
    //      if (last_db > db)
    //      {
    //        db = last_db;
    //      }
    //      last_db = db;
    //    }

    if ((millis() - tempRefresh) >= obj["sensors"]["time"].as<unsigned int>() /*tempSample*/)
    {
      tempRefresh = millis();


      // Demo Sound sensor
      db = analogRead(VR_PIN);
      db = map(db, 1300, 1390, 60, 40);
      db = abs(db);
      if (db > 95)db = 95;

      // ------------------------------------- Temperature,Humidity , UV

      // -------------------------- Temperature
      t = dht.readTemperature();
      //t = t + (sen["sensors"]["t"]["cal"].as<int>());

      // -------------------------- Humidity
      h = dht.readHumidity();

      // Check if any reads failed.
      if (isnan(h) || isnan(t) || (h > 100) || (t > 100) || (t < 0))
      {
        h = 0;
        t = 0;

      }

      // ---------------------------- UV
      adc = analogRead(UV_PIN);
      //adc_33 = analogRead(VR_PIN);  // Comented for no reference pin
      adc_33 = 4094;
      outputVoltage = 3.3 / adc_33 * adc;
      uv = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0);
      if (uv < 0) uv = 0;  // zero for error
      else uv = roundf(uv * 100) / 100; // 2 decimals





      // ----------------------------  Lux
      if (tsl_status == true)
      {
        tsl.getEvent(&event);
        lux = event.light;
        //lux = lux * float(sen["sensors"]["lux"]["cal"].as<float>()); // Porceltual adjust
        //lux = lux * float(1.1);
        if (lux > 9999)lux = 9999; //oversaturated


        //if (lux <= 0)
        //{
        //Serial.println("Sensor overload");
        //lux = 40000;
        //}
      }



      // ------------------------ Air Sensor
      // ------------------------------ air
      // MQ135.setA(605.18); MQ135.setB(-3.937); // Configure the equation to calculate CO concentration value
      // MQ135.update();
      // MQ135.readSensor();

      float sensor_volt; //Define variable for sensor voltage
      float RS_gas; //Define variable for sensor resistance
      float ratio; //Define variable for ratio
      float sensorValue = analogRead(MQ_PIN); //Read analog values of sensor
      sensor_volt = sensorValue * (3.3 / 4094.0); //Convert analog values to voltage
      RS_gas = ((3.3 * 10.0) / sensor_volt) - 10.0; //Get value of RS in a gas
      ratio = RS_gas / R0; // Get ratio RS_gas/RS_air

      double ppm_log = (log10(ratio) - b) / m; //Get ppm value in linear scale according to the the ratio value
      ppm = pow(10, ppm_log); //Convert ppm value to log scale
      ppm = ppm + (obj["sensors"]["ppm"]["cal"].as<int>()); // Fresh air
      if (ppm > 9999) ppm = 9999;
      //double percentage = ppm / 10000; //Convert to percentage

      PrintOut();
      SendData();


    }
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

JsonObject getJSonFromFile(/*DynamicJsonDocument *doc*/ StaticJsonDocument<1736> *doc, String filename, bool forceCleanONJsonError = true )
{
  // open the file for reading:
  file = SPIFFS.open(filename);;
  if (file)
  {
    //Serial.println("Opening File");

    size_t size = file.size();
    //Serial.println(size);

    if (size > 1736)
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
    return doc->to<JsonObject>();
  }

}


// ------------------------------------------------------------------------------------------------ getJsonArrayFromFile

JsonArray getJSonArrayFromFile(StaticJsonDocument<512> *dev_doc, String filename, bool forceCleanONJsonError = true )
{
  // open the file for reading:
  file = SPIFFS.open(filename);;
  if (file)
  {
    //Serial.println("Opening File");

    size_t size = file.size();
    //Serial.println(size);

    if (size > 1024)
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
bool saveJSonArrayToAFile(JsonArray * dev_doc, String filename) {
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


// ---------------------------------------------------------------------------------------------- sensorInit
void sensorInit()
{
  if (sensors_init == false)
  {
    // --------------------------- temperature & humidity
    dht.begin();
    Serial.println("{\"dht\":true}");

    // --------------------------- ultraviolet radiation
    Serial.println("{\"uv\":true}");

    //----------------------------- sound sensor
    //    Serial1.begin(9600, SERIAL_8N1, Sound_RX, Sound_TX);
    //    Serial1.write(0x01);
    //    Serial1.write(0x03);
    //    Serial1.write(0x00);
    //    Serial1.write(0x00);
    //    Serial1.write(0x00);
    //    Serial1.write(0x01);
    //    Serial1.write(0x84);
    //    Serial1.write(0x0A);
    //    unsigned int mic_wait = 0;
    //    while ((!Serial1.available()) && (mic_wait >=1000))
    //    {
    //      mic_wait++;
    //      delayMicroseconds(1);
    //    }
    //    while (Serial1.available())
    //    {
    //      ch_db[i] = Serial1.read();
    //      //Serial.println(ch[i], HEX);
    //      i++;
    //      if (i >= 7)
    //      {
    //        Serial.println("{\"db\":true}");
    //        //i = 0;
    //        //return;
    //      }
    //    }
    //    if (i <= 0)
    {
      Serial.println("{\"db\":false}");
    }



    // --------------------------- air quality (CO)
    //MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
    //MQ135.init();

    //Serial.print("Calibrating MQ135 please wait.");
    //float calcR0 = 0;

    //for (int i = 1; i <= 10; i ++)
    //{
    //MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
    //calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    //Serial.println(calcR0);
    //}
    //MQ135.setR0(calcR0 / 10);

    //Serial.println("  done!.");


    //if (isinf(calcR0)) {
    //Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
    //Serial.println("{\"mq135\":false}");
    //while (1);
    //}
    //else if (calcR0 == 0) {
    //Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
    //while (1);
    //Serial.println("{\"mq135\":false}");
    //}

    //else
    Serial.println("{\"mq135\":true}");


    // --------------------------- Lux Sensor
    if (!tsl.begin()) // BH1750 IS NEEDED
    {
      /* There was a problem detecting the TSL2561 ... check your connections */
      //Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
      Serial.println("{\"tsl\":false}");
      tsl_status = false;
      //while(1);
    }
    else
    {
      /* You can also manually set the gain or enable auto-gain support */
      // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
      //tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
      tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */

      /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
      // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
      // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
      tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

      sensor_t sensor;
      tsl.getSensor(&sensor);

      Serial.println("{\"tsl\":true}");
      tsl_status = true;
      //tsl.getEvent(&event);
    }


    sensors_init = true;
  }
}

// ---------------------------------------------------------------------------------------------------------- dispalyInit
void displayInit()
{

  display1.setPin(obj["neodisplay"]["pin"].as<int>());
  //display1.setPin(25);
  display1.updateDigitType(obj["neodisplay"]["digits"].as<int>(), obj["neodisplay"]["pixels"].as<int>());
  //display1.updateDigitType(17,2);

  display1.begin();             // This fuction calls Adafruit_NeoPixel.begin() to configure.
  //display1.clear();
  //display1.show();

  // Testing display
  /*
    if (obj["type"].as<String>() == "ergo")
    {
    for (int disp_num = 0; disp_num < obj["neodisplay"]["digits"].as<unsigned int>(); disp_num++)
    {
      display1.setCursor(disp_num);
      display1.print("'8.");      // It prints the value.
      display1.show();              // Lights up the pixels.
      delay(200);
      display1.setCursor(disp_num);
      display1.print(" ");
      //
    }

    display1.clear();
    display1.print("oC", Red);
    display1.print("% ", Green);
    display1.print("uV", Purple);
    display1.print("dB ", Blue);
    display1.print("luxe", White);
    display1.print("PPN.N", Cian);
    display1.show();
    delay(1000);
    display1.clear();
    display1.show();


    }

    if (obj["type"].as<String>() == "neo")
    {
    for (int disp_num = 0; disp_num < obj["neodisplay"]["digits"].as<unsigned int>(); disp_num++)
    {
      display1.updateColor(Random); //Before for all display
      display1.setCursor(disp_num);
      display1.print(disp_num);      // It prints the value.
      display1.show();              // Lights up the pixels.
      delay(300);
      //display1.setCursor(disp_num);
      //display1.print(" ");
      //display1.clear();
    }

    //display1.updateColor(Random); //Before for all display
    display1.clear();
    display1.print("12:00");
    display1.updateColor(Random, 0, 3); //After for each digit
    }

    //Serial.println("Display  done!.");
    //display1.show();
  */
  Serial.println("{\"neodigits\":true}");
  neo_digits_status = true;

}


// ---------------------------------------------------------------------------------------------------- loadConfig
// Update a new config in the file an change behivor
void loadConfig()
{


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

  //---------------- Sensors Init
  if (obj["sensors"]["enable"].as<bool>())
  {
    sensorInit();
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
  Serial.println("{\"Config\":true}");

}

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



      /*case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        {

          Serial.println("{\"wifi_event\":\"disconnected\"}");
          contador++;
          if ((obj["wifi"]["sta"]["registered"] == false) && ((contador > 10)))
          {
            if (obj["neodisplay"]["enable"].as<bool>())
            {
              //display1.updatePoint(obj["neodisplay"]["status"].as<int>(), 255, 0, 0);
              //display1.show();
              color_status[0] = 255;
              color_status[1] = 0;
              color_status[2] = 0;
              //PrintOut();
            }
            count++;
            obj["wifi"]["sta"]["count"] = count;
            //obj["wifi"]["sta"]["registered"] = false;
            //obj["wifi"]["sta"]["enable"] = true;
            //obj["wifi"]["sta"]["ssid"] = "";
            //obj["wifi"]["sta"]["pass"] = "";
            Serial.println(saveJSonToAFile(&obj, filename) ? "{\"wifi_disconected_saved\":true}" : "{\"file_saved\":false}");
            loadConfig();
          }

          //reconnect();



        }
        break;

      */

      /*
        case SYSTEM_EVENT_STA_STOP:
        {
          Serial.println("{\"wifi_event\":\"stop\"}");
          String auxssid = obj["wifi"]["sta"]["ssid"].as<String>();

          if (obj["neodisplay"]["enable"].as<bool>())
          {
            //display1.updatePoint(obj["neodisplay"]["status"].as<int>(), 255, 0, 0);
            //display1.show();
            color_status[0] = 255;
            color_status[1] = 0;
            color_status[2] = 0;
            //PrintOut();
          }
          //obj["wifi"]["sta"]["ssid"] = "";
          //obj["wifi"]["sta"]["pass"] = "";
          //loadConfig();
          //

          // New wifi fail, so reset wifi to new
          if ((obj["wifi"]["sta"]["registered"] == false ) && ( (auxssid.length() != 0)))
          {
            //WiFi.disconnect(true);
            obj["wifi"]["sta"]["ssid"] = "";
            obj["wifi"]["sta"]["pass"] = "";
            Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved_wifi_stop\":true}" : "{\"file_saved\":false}");
            //ESP.restart();
            loadConfig();
            //WiFi.mode(WIFI_AP_STA);
            //neoConfig();

          }
        }
        break;
      */

      /*case 10: // ARDUINO_EVENT_WPS_ER_FAILED
        {
          Serial.println("{\"wifi_event\":\"enrollee error\"}");
          display1.updatePoint(PIX_STATUS, 255, 0, 0);
          display1.show();
          if (obj["wifi"]["sta"]["registered"] == false)
          {
            obj["wifi"]["sta"]["ssid"] = "";
            obj["wifi"]["sta"]["pass"] = "";
            Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
            //WiFi.mode(WIFI_AP_STA);
            loadConfig();
            //neoConfig();
          }
          //else
          //  loadConfig();

        }
        break;*/


      /*default:
        {
          Serial.printf("{\"wifi_event\":\"%s\"}", String(event));
          Serial.println();
        }
        break;*/
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

  if ((millis() - timestamp) >= connectTimeoutMs) // check to an interval of time
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

        Firebase.reconnectWiFi(true);

        String route_config = "/panels/" + obj["id"].as<String>() + "/config";
        if (!Firebase.RTDB.beginStream(&stream, "/panels/01/config"))
          Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());
        Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

        // Timeout, prevent to program halt
        //config.timeout.wifiReconnect = 10 * 1000; // 10 Seconds to 2 min
        //config.timeout.socketConnection = 1 * 1000; // 1 sec to 1 min
        //config.timeout.sslHandshake = 1 * 1000; // 1 sec to 1 min
        //config.timeout.rtdbKeepAlive = 45 * 1000;    // 20 sec to 2 min
        //config.timeout.rtdbStreamReconnect = 1 * 1000;  //1 sec to 1 min
        //config.timeout.rtdbStreamError = 3 * 1000;    // 3 sec to 30 sec

      }
      else //if (Firebase.ready() /*&& !taskCompleted*/)
      {
        Serial.println("{\"firebase_connected\":true}");
        blk = !blk;
        //
        //{
        // Get previous pushed data
        //String route_config = "/panels/" + obj["id"].as<String>() + "/config";
        String route_config = "/panels/01/config";




        //if (Firebase.RTDB.getJSON(&fbdo, route_config))
        //{
        //Serial.println(fbdo.to<FirebaseJson>().raw());
        // String s = fbdo.to<String>();
        //deserializeJson(conf_doc, s);
        //serializeJsonPretty(conf_doc, Serial);

        //}
        //else
        //{
        //Serial.println(fbdo.errorReason().c_str());


        //if (strcmp(fbdo.errorReason().c_str(), "path not exist") == 0)
        //{
        //json.set("sensors", obj);
        //json.set("h", h);
        //json.set("uv", uv);
        //json.set("db", db);
        //json.set("lux", lux);
        //json.set("ppm", ppm);
        // now we will set the timestamp value at Ts
        //json.set("Ts/.sv", "timestamp"); // .sv is the required place holder for sever value which currently supports only string "timestamp" as a value
        //nodeName = String(millis());;
        // Set data with timestamp
        //Serial.printf("%s\n", Firebase.RTDB.updateNode(&fbdo, "/panels/01/actual", &json) ? /*fbdo.to<FirebaseJson>().raw()*/"" : fbdo.errorReason().c_str());

        //String route = "/panels/" + obj["id"].as<String>() + "/config";
        //if (Firebase.RTDB.updateNode(&fbdo, route, &json) == false)
        //{
        //Serial.printf("%s\n", fbdo.errorReason().c_str());
        //}
        //else
        //{
        //Serial.println("{\"upload_config\":true}");
        //}

        //}

        //Serial.printf("Push data with timestamp... %s\n", Firebase.RTDB.pushJSON(&fbdo, "/panels/01/data", &json) ? "ok" : fbdo.errorReason().c_str());
        //delay(1000);
        //}
        //}

        //if (dataChanged)
        //{
        // dataChanged = false;
        // When stream data is available, do anything here...
        //}
      }
      
      //if (!Firebase.RTDB.beginStream(&stream, "/test/data"))
      //  Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

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
    timestamp = millis();
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

// -------------------------------------------------------------------------------------------------- sender (Serial)
static int sender(const char *frame, int frame_len, void *privdata)
{


  //if (rpc_input == LORA)
  //{
  //sendMessage(frame);
  //rpc_input = NONE;
  //Serial.println(rpc_input);
  //return 0;
  //Serial.println(frame);
  //}

  //else// (rpc_input == "serial")
  //{
  //return Serial.write(frame, frame_len);
  //rpc_input = NONE;
  return Serial.write(frame, frame_len);
  // }

}

// -------------------------------------------------------------------------------------------------- senderLoRa (LoRa)
static int senderLoRa(const char *frame, int frame_len, void *privdata) {
  //return Serial.write(frame, frame_len);
  sendMessage(frame);
  Serial.println(frame);
}


// ------------------------------------------------------------------------------------------------------ Cfg_get
/*static*/ void Cfg_get(/*struct jsonrpc_request * r*/)
//  {"method":"Config.Get"}
{
  // open file to load config

  obj = getJSonFromFile(&doc, filename);
  dev = getJSonArrayFromFile(&dev_doc, device_list);



  if (obj.isNull())
  {
    Serial.println("{\"config_file\":null}");
    //obj = getJSonFromFile(&doc, filedefault);
    //Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
  }
  serializeJson(obj, Serial);
  Serial.println();
  serializeJson(dev, Serial);
  Serial.println();
}


// ----------------------------------------------------------------------------------------------- factory_reset
void IRAM_ATTR factory_reset1() {
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

void streamCallback(FirebaseStream data)
{
  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); // see addons/RTDBHelper.h
  Serial.println();

  // This is the size of stream payload received (current and max value)
  // Max payload size is the payload size under the stream path since the stream connected
  // and read once and will not update until stream reconnection takes place.
  // This max value will be zero as no payload received in case of ESP8266 which
  // BearSSL reserved Rx buffer size is less than the actual stream payload.
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());

  // Due to limited of stack memory, do not perform any task that used large memory here especially starting connect to server.
  // Just set this flag and check it status later.
  dataChanged = true;
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
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
  timestamp == millis();



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


  if (obj["sensors"]["enable"].as<bool>()) // Sensor Panel normal
  {
    ReadSensors();
  }

  esp_task_wdt_reset();

}
