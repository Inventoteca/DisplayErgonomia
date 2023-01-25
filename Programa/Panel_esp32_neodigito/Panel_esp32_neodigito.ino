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
#include "mjson.h"  // Sketch -> Add File -> Add mjson.h
//#include <WiFiUdp.h>
#include "heltec.h"
//#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <PubSubClient.h>



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
#define DHTPIN      23    // Digital pin, DHT sensor
#define VR_PIN      38    // Voltage referenci pin 3.3V
#define UV_PIN      37    // Analog input for UV sensor
#define MQ_PIN      36    // Analog input for Air sensor
#define NEO_PIN     25    // Pin where the display will be attached
#define Sound_RX    13    // gpio 16 es OLED_Reset
#define Sound_TX    12    // gpio 17 es neopixels
//#define PIX_STATUS  1    // Decimal poimt as LED status
#define FACTORY_BT  0     // gpio 2 as factory reset button
//int gas_sensor = 36; //Sensor pin

// -------------- neodigits
#define DIGITS      17    // Number of NeoDigitos connected
#define PIXPERSEG    2    // NeoPixels per segment

// --------------  DHT & MQ Definitions & Sound
#define DHTTYPE     DHT22   // DHT 22  (AM2302), AM2321




//--------------- LoRa definitios
#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6
#define NONE      0
#define SERIAL    1
#define LORA      2

// ----------------- Objects
NeoDigito display1 = NeoDigito(DIGITS, PIXPERSEG, NEO_PIN, NEO_GRB + NEO_KHZ800); // For more info abaut the last argumets check Adafruit_Neopixel documentation.
//NeoDigito display1 = NeoDigito(0, 0, -1, NEO_GRB + NEO_KHZ800);
//NeoDigito display1 = new NeoDigito(0, 0, -1, NEO_GRB + NEO_KHZ800);
DHT dht(DHTPIN, DHTTYPE);
//MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, mq_pin, mq_type);
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
sensors_event_t event;
JsonObject obj;
JsonArray dev;
StaticJsonDocument<1024> dev_doc;
StaticJsonDocument<1736> doc;

File file;

//--------------- MQTT
WiFiClient espClient;
PubSubClient client(espClient);


//JsonObject getJSonFromFile(DynamicJsonDocument *doc, String filename, bool forceCleanONJsonError = true );

// ---------------- sensors variables
int t;
int h;
int db;
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

//mq-135
float m = -0.318; //Slope
float b = 1.133; //Y-Intercept
//float R0 = 11.820; //Sensor Resistance in fresh air f
float R0 = 9.000; //Sensor Resistance in fresh air f

// --------- file variables
const char *filedefault = "/default.json";
const char *filename = "/config.json";
const char *device_list = "/devices.json";
bool correct = false;
bool wifi_config = false;
int wifi_trys;
boolean isSaved = false;
char buf[1024];
int rpc_input = NONE;


// ---------------- timing
unsigned long sensorRefresh = 0;            // Sample interval for sensors
short conn_count;                           // Counting retrys to connect wifi
const uint32_t connectTimeoutMs = 2000;     // Maximun Milliseconds for check connection MQTT & WiFi
unsigned long  timestamp;                   // Running time
unsigned int reboot_wifi_count;             // Counting reboots to connect to wifi
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

// ------------------------------------------------------------------------------ reconnect
void reconnect() {
  // Loop until we're reconnected
  if (!client.connected())
  {
    //Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "";
    String userEmail = "";
    String msgPanel = "";

    // Attempt to connect
    clientId = obj["id"].as<String>();
    userEmail = "smart/users/" + obj["email"].as<String>() + "/add";
    //msgPanel = "{\"id\":\"\" + clientId  + "",\"type\":\"" + obj["type"].as<String>() + "\",\"name\":\"" + obj["name"].as<String>() + "\",\"mod\":true}";

    DynamicJsonDocument msg(1024);

    msg["id"] = obj["id"];
    msg["name"] = obj["name"];
    msg["type"] = obj["type"];
    msg["mod"] = true; // email registrated
    serializeJson(msg, msgPanel);


    if (client.connect(clientId.c_str()))
    {
      //Serial.println("connected");
      // Once connected, publish an announcement...
      if (obj["registered"].as<bool>()) // Device registered on user db
      {
        client.publish(userEmail.c_str(), "{}", true);
      }
      else
      {
        client.publish(userEmail.c_str(), msgPanel.c_str(), true);
        Serial.println(msgPanel);
      }

      // ... and resubscribe
      //  char buftop[100];
      //  char bufsen[10];
      //  const char* topicRoot = "test/panels/%s/%s";
      //  sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "t");
      //  sprintf(bufsen, "%i", t);
      //client.publish(buftop, bufsen);


      clientId = "smart/panels/" + clientId + "/app";
      client.subscribe(clientId.c_str());
      Serial.println("{\"mqtt\":true}");
    }
    else
    {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 1 seconds");
      // Wait 5 seconds before retrying
      //delay(5000);
      Serial.println("{\"mqtt\":false}");
    }
  }
}

// ----------------------------------------------------------------------------------- callback
void callback(char* topic, byte* payload, unsigned int length) {

  String dataConfig = "";

  Serial.print("{\"mqtt_msg_rec\":{\"topic\":\"");
  Serial.print(topic);
  Serial.print("\",\"payload\":");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    dataConfig += (char)payload[i];
  }
  Serial.print("}}");
  Serial.println();

  String msgPanel;
  String userEmail  = "smart/users/" + obj["email"].as<String>() + "/add";
  String clientId = "smart/panels/" + obj["id"].as<String>() + "/conf/response";

  //Serial.println(dataConfig);
  StaticJsonDocument<1736> docConf;
  deserializeJson(docConf, dataConfig);
  JsonObject objConf;
  objConf = docConf.as<JsonObject>();


  if (objConf.isNull())
  {
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
      Serial.println("{\"user\":false}");
      // it is active low on the ESP-01)
    } else if ((char)payload[0] == '1') {
      //msg["sensors"]["t"] = obj["sensors"]["t"];
      //msg["sensors"]["h"] = obj["sensors"]["h"];
      //msg["sensors"]["uv"] = obj["sensors"]["uv"];
      //msg["sensors"]["db"] = obj["sensors"]["db"];
      //msg["sensors"]["lux"] = obj["sensors"]["lux"];
      //msg["sensors"]["ppm"] = obj["sensors"]["ppm"];
      serializeJson(obj, msgPanel);
      Serial.println(msgPanel);
      // No cabe toda la configuracion por mqtt al parecer
      client.publish(clientId.c_str(), msgPanel.c_str(), true);
      Serial.println("{\"user\":true}");
    }

  }
  else if (objConf["method"] == "Config.Set")
  {
    serializeJsonPretty(objConf, Serial);
    obj = objConf["params"];
    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
  }

  //DynamicJsonDocument msg(1024);




  if (obj["registered"] == false)
  {
    client.publish(userEmail.c_str(), "{}", true);
    obj["registered"] = true;
    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
  }



}


// -------------------------------------------------------------------------------------- lora_send
// {"method":"LoRa.Send","params":{"method":"Config.Get"}
// {"method":"LoRa.Send","params":{"method":"Config.Get","params":"\"{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}\""}}
static void lora_send(struct jsonrpc_request *r)
{

  char cmd[100];
  char cmd_params[100];
  StaticJsonDocument<124> doc_cmd;
  const char *buf;  // Get `b` sub-object
  int len;
  StaticJsonDocument<124> doc_params;
  //StaticJsonDocument<124> doc_aux;
  String s_cmd;
  //JsonObject cmd_params = doc_cmd.createNestedObject("params");




  if (mjson_get_string(r->params, r->params_len, "$.method", cmd, sizeof(cmd)) > 0)
  {
    doc_cmd["method"] = cmd;
    //Serial.println(cmd);
    //serializeJsonPretty(doc_cmd, Serial);
    if (mjson_find(r->params, r->params_len, "$.params", &buf, &len))
    {
      //Serial.println(buf);
      deserializeJson(doc_params, buf);
      doc_cmd["params"] = doc_params;
      //JsonObject root = doc_aux.createNestedObject();
      //

      //

      //

      //root["params"] = doc_params;
      //serializeJsonPretty(doc_params, Serial);
      //doc_cmd["params"].set(root);
      //message = cmd;
      //

    }
    serializeJson(doc_cmd, s_cmd);
    Serial.println(s_cmd);
    sendMessage(s_cmd);
    //serializeJsonPretty(doc_cmd, Serial);

  }



  if (obj["oled"].as<bool>())
  {
    Heltec.display->clear();
    Heltec.display->display();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);


    Heltec.display->drawString(0, 0, "Enviado");

    Heltec.display->drawString(0, 10, s_cmd);
    Heltec.display->display();
  }


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
  sendMessage(message);
}


// ------------------------------------------------------------------------------------------- onReceive
void onReceive(int packetSize)
{

  if (packetSize == 0) return;          // if there's no packet, return
  //Serial.println();
  //Serial.println("LoRa:");

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";
  unsigned int i = 0;

  while (LoRa.available()) {
    //incoming += (char)LoRa.read();
    buf[i] = LoRa.read();
    incoming += (char)buf[i];
    i++;
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    // return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  //if (recipient != localAddress && recipient != 0xFF)
  if (recipient != obj["lora"]["local"] && recipient != 0xFF)
  {
    //Serial.println("{\"lora_rec\":true}");

    //Serial.println("This message is not for me.");
    Serial.println("LoRa: " + String(sender) + "->" + String(recipient));
    //Serial.println("Sent to: " + String(recipient));

    // Serial.println("Message ID: " + String(incomingMsgId));
    // Serial.println("Message length: " + String(incomingLength));
    // Serial.println("Message: ");
    // serializeJsonPretty(objx, Serial);
    // Serial.println("RSSI: " + String(LoRa.packetRssi()));
    // Serial.println("Snr: " + String(LoRa.packetSnr()));
    // Serial.println();
    //return;                             // skip rest of function
  }
  else
  {
    // Json RPC
    StaticJsonDocument<1024> docx;
    //JsonObject root = docx.createNestedObject(F(dev_id));
    deserializeJson(docx, incoming);
    JsonObject objx = docx.as<JsonObject>();
    //

    if (objx.isNull())
      Serial.println("objx is null");
    else
    {
      if (objx["method"].isNull())
      {
        //Serial.println("method is null");
      }
      else
      {
        Serial.println("method Received");
        rpc_input = LORA;
        jsonrpc_process(buf, sizeof(buf), senderLoRa, NULL, NULL);
        rpc_input = NONE;

      }

      if (objx["error"].isNull());
      //Serial.println("error is null");
      else
      {
        Serial.println("error Received");
      }

      if (objx["result"].isNull());
      //Serial.println("result is null");
      else
      {
        Serial.println("Result Received");
      }

      if (objx["sensors"].isNull());
      //Serial.println("sensors is null");
      else
      {
        // Serial.println("Sensors Received");
        t = objx["sensors"]["t"];
        h = objx["sensors"]["h"];
        uv = objx["sensors"]["uv"];
        db = objx["sensors"]["db"];
        lux = objx["sensors"]["lux"];
        ppm = objx["sensors"]["ppm"];
        //PrintOut();

        serializeJson(objx, Serial);
        Serial.println();
        //Serial.println(buf);
        //client.publish("smartOut", buf);

        if (obj["oled"].as<bool>() || obj["neodisplay"].as<bool>())
          PrintOut();

        if (obj["wifi"]["sta"]["enable"].as<bool>() && (obj["mqtt"]["enable"].as<bool>()) && (client.connected()))
        {
          char buftop[100];
          char bufsen[10];
          const char* topicRoot = "smart/panels/%s/sensors/%s";
          sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "t");
          sprintf(bufsen, "%i", t);
          client.publish(buftop, bufsen);

          sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "h");
          sprintf(bufsen, "%i", h);
          client.publish(buftop, bufsen);

          sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "uv");
          sprintf(bufsen, "%.1f", uv);
          client.publish(buftop, bufsen);

          sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "db");
          sprintf(bufsen, "%i", db);
          client.publish(buftop, bufsen);

          sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "lux");
          sprintf(bufsen, "%i", lux);
          client.publish(buftop, bufsen);

          sprintf(buftop, topicRoot, obj["id"].as<const char*>(), "ppm");
          sprintf(bufsen, "%i", ppm);
          client.publish(buftop, bufsen);
          Serial.println("{\"mqtt_send\":true}");
        }
        else
          Serial.println("{\"mqtt_send\":false}");

      }
    }

    // if message is for this device, or broadcast, print details:



    //Heltec.display->clear();
    //Heltec.display->display();
    //Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    //Heltec.display->drawString(0, 0, incoming);
    //Heltec.display->display();

  }


}


//----------------------------------------------------------------------------------------- sendMessage
void sendMessage(String outgoing)
{
  if (dev.size() >= 0) // list of devices
  { //  Serial.println("Sending LoRa...");
    for (JsonArray::iterator it = dev.begin(); it != dev.end(); ++it)
    {
      if ((*it)["network"] == "lora")
      {
        digitalWrite(25, HIGH);
        LoRa.beginPacket();                   // start packet
        //if (strcmp((*it)["id"], dev_id) == 0)
        LoRa.write((*it)["local"]);              // add destination address
        LoRa.write(obj["lora"]["local"]);             // add sender address
        LoRa.write(msgCount);                 // add message ID
        LoRa.write(outgoing.length());        // add payload length  //aqui debo cambiar el print para ampliar el maximo
        LoRa.print(outgoing);                 // add payload7
        LoRa.endPacket();                     // finish packet and send it
        msgCount++;                           // increment message ID
        Serial.println("{\"lora_send\":true}");
        digitalWrite(25, LOW);
      }
      //else
      //  Serial.println("{\"lora_send\":false}");
    }
  }
  else
  {
    Serial.println("{\"lora_send\":false}");
  }



}



// ----------------------------------------------------------------------------------------- neoConfig
void neoConfig()
{

  if (!wifi_config) // Si aun no se inicia la config
  {
    //if (neo_digits_status == true)
    {
      //display1.updatePoint(obj["neodisplay"]["status"].as<unsigned int>(), 255, 255, 255);
      display1.updatePoint(1, 255, 255, 255);
      display1.show();
      delay(100);

    }
    
    //obj["wifi"]["sta"]["count"] = 0;
    //Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP_STA);
    wifi_config = WiFi.beginSmartConfig(SC_TYPE_ESPTOUCH_V2);
    Serial.print("{\"SmartConfig\":");
    Serial.print(wifi_config);
    Serial.println("}");
    

    //while (!WiFi.smartConfigDone());
    //if (!wifi_config) ESP.restart();
  }
  //else // Configuracion iniciada
  //{
  //Serial.print("Wait conection response");
  if (WiFi.smartConfigDone()) // Configuracion correcta
  {
    //WiFi.stopSmartConfig();
    wifi_config = false;
    wifi_trys = 0;
    Serial.print("SmartConfig Started Done");
  }

}


// --------------------------------------------------------------------------------------- PrintOut
void PrintOut()
{
  //if (millis() - printRefresh > printTime)
  {
    if (obj["oled"].as<bool>())
    {
      Heltec.display->clear();
      Heltec.display->display();
      Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);


      Heltec.display->drawString(0, 0, String(t));
      Heltec.display->drawString(40, 0, "°C");
      Heltec.display->drawString(0, 10, String(h));
      Heltec.display->drawString(40, 10, "%");


      Heltec.display->drawString(0, 20, String(uv));
      Heltec.display->drawString(40, 20, "UV");


      Heltec.display->drawString(0, 30, String(db));
      Heltec.display->drawString(40, 30, "dB");


      Heltec.display->drawString(0, 40, String(lux));
      Heltec.display->drawString(40, 40, "Lux");

      Heltec.display->drawString(0, 50, String(ppm));
      Heltec.display->drawString(40, 50, "PPM");

      Heltec.display->display();
    }


    if (neo_digits_status == true)
    {
      display1.setCursor(0);
      display1.print(t, (t >= (obj["sensors"]["t"]["max"].as<int>())) ?
                     (obj["sensors"]["t"]["colMax"].as<uint32_t>()) : (t <= (obj["sensors"]["t"]["min"].as<int>())) ?
                     (obj["sensors"]["t"]["colMin"].as<uint32_t>()) : (obj["sensors"]["t"]["colDef"].as<uint32_t>()));
      display1.setCursor(2);
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
      display1.show();

    }

    // Prepare for LoRa
    //SendData();
    //printRefresh = millis();

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
    if (!Serial1.available())
    {
      //Serial.println("Send");
      short index_char = 0;
      db = 0;
      Serial1.write(0x01);
      Serial1.write(0x03);
      Serial1.write(0x00);
      Serial1.write(0x00);
      Serial1.write(0x00);
      Serial1.write(0x01);
      Serial1.write(0x84);
      Serial1.write(0x0A);
      while (!Serial1.available());
      while (Serial1.available())
      {
        buf[index_char] = Serial1.read();
        //Serial.println(ch[i], HEX);
        index_char++;

        if (index_char >= 7)
        {
          index_char = 0;
          //Serial.print(ch[3],HEX);
          //Serial.println(ch[4],HEX);
          db = ((buf[3] * 256) + buf[4]) / 10;
          //Serial.println(db);
        }
      }
      if (last_db > db)
      {
        db = last_db;
      }
      last_db = db;
    }

    if ((millis() - sensorRefresh) >= obj["sensors"]["time"].as<unsigned int>() /*tempSample*/)
    {
      sensorRefresh = millis();

      // ------------------------------------- Temperature,Humidity , UV

      // -------------------------- Temperature
      t = dht.readTemperature();
      t = t + (obj["sensors"]["t"]["cal"].as<int>());

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
      adc_33 = analogRead(VR_PIN);
      outputVoltage = 3.3 / adc_33 * adc;
      uv = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0);
      if (uv < 0) uv = 0;  // zero for error
      else uv = roundf(uv * 100) / 100; // 2 decimals





      // ----------------------------  Lux
      tsl.getEvent(&event);
      lux = event.light;
      if (lux > 9999)lux = 9999; //oversaturated



      //if (lux <= 0)
      //{
      //Serial.println("Sensor overload");
      //lux = 40000;
      //}


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

JsonArray getJSonArrayFromFile(StaticJsonDocument<1024> *dev_doc, String filename, bool forceCleanONJsonError = true )
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
    Serial1.begin(9600, SERIAL_8N1, Sound_RX, Sound_TX);
    Serial1.write(0x01);
    Serial1.write(0x03);
    Serial1.write(0x00);
    Serial1.write(0x00);
    Serial1.write(0x00);
    Serial1.write(0x01);
    Serial1.write(0x84);
    Serial1.write(0x0A);
    while (!Serial1.available());
    short index_char = 0;
    while (Serial1.available())
    {
      ch_db[index_char] = Serial1.read();
      //Serial.println(ch[i], HEX);
      index_char++;
      if (index_char >= 7)
      {
        Serial.println("{\"db\":true}");
        //i = 0;
        //return;
      }
    }
    if (index_char <= 0)
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
    if (!tsl.begin())
    {
      /* There was a problem detecting the TSL2561 ... check your connections */
      //Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
      Serial.println("{\"tsl\":false}");
      //while(1);
    }
    else
    {
      /* You can also manually set the gain or enable auto-gain support */
      // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
      // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
      tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */

      /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
      // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
      // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
      tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

      sensor_t sensor;
      tsl.getSensor(&sensor);

      Serial.println("{\"tsl\":true}");
      //tsl.getEvent(&event);
    }


    sensors_init = true;
  }
}

// ---------------------------------------------------------------------------------------------------------- dispalyInit
void displayInit()
{

  pinMode(obj["neodisplay"]["pin"].as<unsigned int>(), OUTPUT);
  //NeoDigito display1 = NeoDigito(obj["neodisplay"]["digits"].as<unsigned int>(), obj["neodisplay"]["pixels"].as<unsigned int>(), obj["neodisplay"]["pin"].as<unsigned int>(), NEO_GRB + NEO_KHZ800);
  //&display1 = _display;
  //display1 = new NeoDigito(obj["neodisplay"]["digits"].as<unsigned int>(), obj["neodisplay"]["pixels"].as<unsigned int>(), obj["neodisplay"]["pin"].as<unsigned int>(), NEO_GRB + NEO_KHZ800);
  display1.begin();             // This fuction calls Adafruit_NeoPixel.begin() to configure.
  display1.clear();
  display1.updateColor(white);
  //display1.setColor(yellow); // Color specified by a 32bit hex, or 8bit numbers (red, green, blue), Also colors names, red, white, yellow, etc.

  display1.clear();
  display1.show();
  if (obj["type"].as<String>() == "ergo")
  {
    for (int disp_num = 0; disp_num < obj["neodisplay"]["digits"].as<unsigned int>(); disp_num++)
    {
      display1.setCursor(disp_num);
      display1.print("'8.");      // It prints the value.
      display1.show();              // Lights up the pixels.
      delay(300);
      display1.setCursor(disp_num);
      display1.print(" ");
      //display1.clear();
    }

    display1.print("oC", red);
    display1.print("% ", green);
    display1.print("uV", purple);
    display1.print("dB ", blue);
    display1.print("luxe", white);
    display1.print("PPN.N", cian);
    //display1.print("8:8.",white);
    //display1.show();
    delay(1000);
    //display1.clear();

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
    display1.updateColor(Random,0,3); //After for each digit
  }

  display1.show();
  //Serial.println("Display  done!.");
  Serial.println("{\"neodigits\":true}");
  neo_digits_status = true;

}


// ---------------------------------------------------------------------------------------------------- loadConfig
// Update a new config in the file an change behivor
void loadConfig()
{
  // ----------------------------------- oled
  if (obj["oled"].as<bool>())
  {
    Heltec.display->clear();
    Heltec.display->display();
    Heltec.display->flipScreenVertically();
  }

  // ----------------------------------- neodisplay
  if (obj["neodisplay"]["enable"].as<bool>())
  {
    // Display Init
    if (neo_digits_status == false)
      displayInit();
  }


  // ----------------------------------- wifi_sta
  if (obj["wifi"]["sta"]["enable"].as<bool>() /*&& (WiFi.status() != WL_CONNECTED)*/)
  {
    String auxssid = obj["wifi"]["sta"]["ssid"].as<String>();
    reboot_wifi_count  = obj["wifi"]["sta"]["count"];  // retrys and reboots
    if (((obj["wifi"]["sta"]["registered"].as<bool>() == false) && (reboot_wifi_count >= 2)) || (auxssid.length() == 0))
    {
      //WiFi.disconnect(true);
      neoConfig();
      //obj["lora"]["enable"] = false;
    }

    else if ((obj["wifi"]["sta"]["registered"].as<bool>() == true) || (obj["wifi"]["sta"]["count"] < 2))
    {
      //Serial.println("Connecting to WiFi");
      WiFi.begin(obj["wifi"]["sta"]["ssid"].as<const char*>(), obj["wifi"]["sta"]["pass"].as<const char*>());
      //Serial.print("WiFi connecting: \t");
      Serial.print("{\"wifi\":{\"ssid\":\"");
      Serial.print(obj["wifi"]["sta"]["ssid"].as<const char*>());
      Serial.println("\"}}");


      // ----------------------------------- mqtt
      if (obj["mqtt"]["enable"].as<bool>())
      {
        client.setBufferSize(1736);
        client.setServer(obj["mqtt"]["broker"].as<const char*>(), obj["mqtt"]["port"].as<unsigned int>());
        //client.setServer(obj["mqtt"]["broker"].as<const char*>(),1883);
        //client.setServer("inventoteca.com", 1883);
        client.setCallback(callback);
        // Serial.println(obj["mqtt"]["port"].as<unsigned int>());

      }
    }

  }
  else
  {
    //
    WiFi.disconnect(true);
    Serial.print("{\"wifi\":{\"ssid\":\"");
    Serial.print(obj["wifi"]["sta"]["enable"].as<const char*>());
    Serial.println("\"}}");
  }

  // ----------------------------------- wifi_ap
  if (obj["wifi"]["ap"]["enable"].as<bool>())
  {
    ap_Init(obj["wifi"]["ap"]["ssid"].as<const char*>(), obj["wifi"]["ap"]["pass"].as<const char*>());

    // WiFi.softAP();
  }
  else
  {
    WiFi.softAPdisconnect(true);
  }

  // ----------------------------------- sensors
  if (obj["sensors"]["enable"].as<bool>())
  {
    sensorInit();

    sensorRefresh = millis();
    //soundRefresh = millis();
    //   airRefresh = millis();
    timestamp = millis();
  }

  // ----------------------------------- id
  String s_aux;
  s_aux = obj["id"].as<String>();
  int len = s_aux.length();
  int i_aux  = obj["lora"]["local"].as<int>();
  char aux_buf[50];


  // if ((strcmp(s_aux.c_str(), WiFi.macAddress().c_str()) != 0) && (len == 0))
  if (len == 0)
  {
    Serial.println("{\"update_id\":true}");

    obj["id"].set( WiFi.macAddress());
    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}" );
    serializeJson(obj, Serial);
  }

  // ----------------------------------- LoRa id
  if (i_aux == 0)
  {
    Serial.println("{\"update_lora\":true}");
    strcpy(aux_buf, s_aux.c_str());
    localAddress = 0;

    for ( int i = 0; i < len; i++)
      localAddress += aux_buf[i];

    if (localAddress == 0)localAddress = random(1, 255);
    obj["lora"]["local"].set(localAddress);
    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}" );

    //Serial.println( localAddress, HEX);
  }



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

  switch (event) {

    case ARDUINO_EVENT_SC_SCAN_DONE:
      {
        Serial.println("{\"wifi_event\":\"scan\"}");
        conn_count = 0;
       // if (neo_digits_status == true)
        {
          display1.updatePoint(1, 255, 255, 255);
          //display1.updatePoint(obj["neodisplay"]["status"].as<unsigned int>(), 255, 255, 255);
          display1.show();
          
          //display1.show();
        }

      }
      break;

    case ARDUINO_EVENT_SC_FOUND_CHANNEL:
      {
        //Serial.println("Found channel");
        Serial.println("{\"wifi_event\":\"found\"}");
        if (neo_digits_status == true)
        {
          display1.updatePoint(obj["neodisplay"]["status"].as<unsigned int>(), 0, 255, 255);
          display1.show();
        }
        conn_count = 0;

      }
      break;

    case ARDUINO_EVENT_SC_GOT_SSID_PSWD:
      {

        Serial.println("{\"wifi_event\":\"config\"}");

        if (info.sc_got_ssid_pswd.type == SC_TYPE_ESPTOUCH_V2) {
          ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
          memcpy(email, rvd_data, sizeof(rvd_data));

        }

        memcpy(pass, info.sc_got_ssid_pswd.password, sizeof(info.sc_got_ssid_pswd.password) + 1);
        memcpy(ssid, info.sc_got_ssid_pswd.ssid, sizeof(info.sc_got_ssid_pswd.ssid) + 1);

        //Serial.println("Got SSID and password");
        conn_count = 0;

        Serial.printf("SSID:%s\n", ssid);
        Serial.printf("PASSWORD:%s\n", pass);

        // Save config
        // obj not save complete ssid, better use doc
        doc["wifi"]["sta"]["ssid"] = ssid;
        doc["wifi"]["sta"]["pass"] = pass;
        doc["wifi"]["sta"]["enable"] = true;
        doc["wifi"]["sta"]["count"] = 0;
        doc["wifi"]["sta"]["registered"] = false;

        if (sizeof(email) > 0)doc["email"] = email;
        else doc["email"] = "inventotk@gmail.com";
        //obj["mq"]
        //wifi_config = false;
        correct = false;
        // COMENTED FOR TEST DEV, UNCOMENT FOR PROD
        Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}" );
        display1.updatePoint(obj["neodisplay"]["status"].as<unsigned int>(), 0, 0, 255);
        display1.show();

      }
      break;

    case ARDUINO_EVENT_SC_SEND_ACK_DONE:
      {
        //Serial.println("SC_EVENT_SEND_ACK_DONE");
        Serial.println("{\"wifi_event\":\"ack\"}");
        correct = true;
        conn_count = 0;
        doc["wifi"]["sta"]["registered"] = true;
        // COMENTED FOR TEST DEV, UNCOMENT FOR PROD
        Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}" );
        display1.updatePoint(obj["neodisplay"]["status"].as<unsigned int>(), 0, 255, 0);
        display1.show();
      }
      break;



    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      {

        Serial.println("{\"wifi_event\":\"disconected\"}");
        conn_count++;
        //count++;
        if ((obj["wifi"]["sta"]["registered"] == false) && ((conn_count > 5)))
        {
          display1.updatePoint(obj["neodisplay"]["status"].as<unsigned int>(), 255, 0, 0);
          display1.show();
          reboot_wifi_count++;
          obj["wifi"]["sta"]["count"] = reboot_wifi_count;
          //obj["wifi"]["sta"]["registered"] = false;
          obj["wifi"]["sta"]["enable"] = true;
          obj["wifi"]["sta"]["ssid"] = "";
          obj["wifi"]["sta"]["pass"] = "";
          Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
          loadConfig();
        }

        //reconnect();



      }
      break;



    case SYSTEM_EVENT_STA_STOP:
      {
        Serial.println("{\"wifi_event\":\"stop\"}");
        display1.updatePoint(obj["neodisplay"]["status"].as<unsigned int>(), 255, 0, 0);
        display1.show();
        //obj["wifi"]["sta"]["ssid"] = "";
        //obj["wifi"]["sta"]["pass"] = "";
        //loadConfig();
        //
        if (obj["wifi"]["sta"]["registered"] == false)
        {
          //WiFi.disconnect(true);
          obj["wifi"]["sta"]["ssid"] = "";
          obj["wifi"]["sta"]["pass"] = "";
          Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
          //ESP.restart();
          loadConfig();
          //WiFi.mode(WIFI_AP_STA);
          //neoConfig();

        }
      }
      break;

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


    default:
      {
        Serial.printf("{\"wifi_event\":\"%s\"}", String(event));
        Serial.println();
      }
      break;
  }
}


// ------------------------------------------------------------------------------------------------------- checkServer
void checkServer()
{

  /*if (WiFi.smartConfigDone() && correct == false) //save config succes
    {
    Serial.println("SmartConfig Success");
    WiFi.printDiag(Serial);
    correct = true;
    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
    WiFi.stopSmartConfig();

    }*/

  if (millis() - timestamp > connectTimeoutMs) //
  {


    if (WiFi.status() == WL_CONNECTED)
    {
      //if ((strip->getPixelColor(32) != 0) && (Color != 0))
      //{
      // strip->setPixelColor(32, Color);
      //strip->setPixelColor(i, (strip -> gamma8(R)),(strip -> gamma8(G)),(strip -> gamma8(B)));
      //strip->setPixelColor(i, (strip -> gamma32(R)),(strip -> gamma32(G)),(strip -> gamma32(B)));
      //}
      if (blk == true)
      {
        display1.updatePoint(obj["neodisplay"]["status"].as<unsigned int>(), obj["sensors"]["ppm"]["colDef"].as<uint32_t>());
        //display1.updatePoint(PIX_STATUS,0,255,0);
        display1.show();
      }
      else
      {
        display1.updatePoint(obj["neodisplay"]["status"].as<unsigned int>(), 0, 0, 0);
        display1.show();
      }
      blk = !blk;

      wifi_config = false;
      if (obj["wifi"]["sta"]["registered"] == false)
      {
        obj["wifi"]["sta"]["registered"] = true;
        obj["wifi"]["sta"]["count"] = 0;
        // COMENTED FOR TEST DEV, UNCOMENT FOR PROD
        Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");

      }

      if (obj["mqtt"]["enable"].as<bool>())
      {
        if (!client.connected())
        {
          reconnect();
        }
      }




      //if(correct == false)
      //{
      //  obj["wifi"]["sta"]["enable"] = true;
      //  correct = true;
      // Serial.println(saveJSonToAFile(&obj, filename)? "File saved!":"Error on save File!");
      // }
      //}
    }
    else
    {



      if (obj["wifi"]["sta"]["registered"] == true)
      {
        //WiFi.reconnect();
        // aqui debo contar o distinguir si hay error
        if (obj["wifi"]["sta"]["enable"].as<bool>())
        {
          //Serial.println("WiFi not connected!");
          //WiFi.reconnect();


          //wifi_trys++;
          //if (wifi_trys < 5)
          {
            //Serial.println("Connecting to WiFi");
            Serial.println("{\"wifi_event\":\"reconnect\"}");
            WiFi.begin(obj["wifi"]["sta"]["ssid"].as<const char*>(), obj["wifi"]["sta"]["pass"].as<const char*>());
            if (blk == true)
            {
              display1.updatePoint(obj["neodisplay"]["status"].as<unsigned int>(), 255, 0, 0);
              display1.show();
            }
            else
            {
              display1.updatePoint(obj["neodisplay"]["status"].as<unsigned int>(), 0, 0, 0);
              display1.show();
            }
            blk = !blk;
          }
          //else
          //{
          //Serial.println("WiFi not available... new config");
          //WiFi.stopSmartConfig();
          //neoConfig();

          // }

        }
        else
        {
          //Serial.println("WiFi not enabled!");
          //if (obj["wifi"]["sta"]["registered"].as<bool>() == false)
          // {
          //   WiFi.stopSmartConfig();
          //   neoConfig();
          // }


        }
      }

    }

    //Serial.println();
    timestamp = millis();
  }
}


// -------------------------------------------------------------------------------------------------------------- ap_init
void ap_Init(const char *ap_ssidx, const char *ap_passx)
{
  Serial.println("Starting AP");
  IPAddress apIP(192, 168, 0, 1);   //Static IP for wifi gateway
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); //set Static IP gateway on NodeMCU
  WiFi.softAP(ap_ssidx, ap_passx); //turn on WIFI
  Serial.println("AP Ready");
  Serial.println(ap_ssidx);
  //websocketInit();
}


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


// ------------------------------------------------- senderWS (WS)
//static int senderWS(const char *frame, int frame_len, void *privdata) {
//return webSocket.sendTXT(numx, frame);
//return webSocket.sendTXT(fra, cmd);
//}


//---------------------------------------------------------------------------------------------------- reportState
static void reportState(struct jsonrpc_request * r) {
  mjson_printf(sender, NULL,
               "{\"method\":\"Shadow.Report\",\"params\":{\"on\":%s}}\n",
               digitalRead(25) ? "true" : "false");
}

//-------------------------------------------------------------------------------------------------------- count
//static void counter(struct jsonrpc_request * r) {
//  i++;
//  mjson_printf(sender, NULL, "%d", i);
//return i;
//}


// ----------------------------------------------------------------------------------------------------- resultState
//static void resultState(void) {
//  mjson_printf(sender, NULL,
//               "{\"result\":{\"on\":%s}}\n",
//               digitalRead(25) ? "true" : "false");
//}



// ------------------------------------------------------------------------------------------------------ Cfg_get
static void Cfg_get(struct jsonrpc_request * r)
//  {"method":"Config.Get"}
{
  // open file to load config

  obj = getJSonFromFile(&doc, filename);
  dev = getJSonArrayFromFile(&dev_doc, device_list);

  //StaticJsonDocument<2048> doc_cfg;
  //doc_cfg["result"]["config"] = obj;
  //doc_cfg["result"]["devices"] = dev;

  //if (rpc_input == NONE)
  //{
  if (obj.isNull())
  {
    Serial.println("{\"default\":true}");
    obj = getJSonFromFile(&doc, filedefault);
    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
  }
  serializeJson(obj, Serial);
  Serial.println();
  serializeJson(dev, Serial);
  Serial.println();
  //}
  //else
  //{
  //serializeJson(dev, buf);
  //mjson_printf(sender, NULL, buf);
  //}

}

// --------------------------------------------------------------------------------------------------------- Cfg_set
// "device":{"id":"CC:50:E3:99:25:94","type":"ergo","name":"Panel de prueba","network":"lora","local":203}
// {"method":"Config.Set","params":{"enable":false, "ssid": "SmartPanel", "pass": "12345678", "type":"ap"}}
static void Cfg_set(struct jsonrpc_request * r) {

  int wifiOn;
  char ssid[30];
  char pass[30];
  char type[10];

  if (mjson_get_string(r->params, r->params_len, "$.type", type, sizeof(type)) <= 0) // Default sta
  {
    strcpy("sta", type);
  }

  if ((strcmp(type, "sta") == 0) || (strcmp(type, "ap") == 0))
  {
    if (mjson_get_bool(r->params, r->params_len, "$.enable", &wifiOn) != 0) // Return 0 if not found, non-0 otherwise.
    {
      obj["wifi"][type]["enable"] = wifiOn ? true : false;
    }

    if (mjson_get_string(r->params, r->params_len, "$.ssid", ssid, sizeof(ssid)) > 0)
    {
      obj["wifi"][type]["ssid"] = ssid;

    }
    //If a string is not found, return -1. If a string is found, return the length of unescaped string

    if ( mjson_get_string(r->params, r->params_len, "$.pass", pass, sizeof(pass)) > 0)
    {
      obj["wifi"][type]["pass"] = pass;
    }

    Serial.println();
    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
    loadConfig();                 // update behivor
    jsonrpc_return_success(r, "%s", message, r->userdata); // Report success
    Serial.println();
    serializeJson(obj, Serial);

  }


}



// --------------------------------------------------------------------------------------------------------- Wifi_set
// {"method":"WiFi.Set","params":{"enable":false, "ssid": "Inventoteca_2G", "pass": "science_7425", "type":"sta"}}
// {"method":"WiFi.Set","params":{"enable":false, "ssid": "Inventoteca_2G", "pass": "science_7425"}}
// {"method":"WiFi.Set","params":{"enable":false, "ssid": "SmartPanel", "pass": "12345678", "type":"ap"}}
static void Wifi_set(struct jsonrpc_request * r) {

  int wifiOn;
  char ssid[65];
  char pass[65];
  char type[33] = "sta";
  int registered;


  if (mjson_get_string(r->params, r->params_len, "$.type", type, sizeof(type)) > 0); // Default sta
  else
  {
    strcpy("sta", type);
  }

  if ((strcmp(type, "sta") == 0) || (strcmp(type, "ap") == 0))
  {
    if (mjson_get_bool(r->params, r->params_len, "$.registered", &registered) != 0) // Return 0 if not found, non-0 otherwise.
    {
      obj["wifi"][type]["registered"] = wifiOn ? true : false;
    }

    if (mjson_get_bool(r->params, r->params_len, "$.enable", &wifiOn) != 0) // Return 0 if not found, non-0 otherwise.
    {
      obj["wifi"][type]["enable"] = wifiOn ? true : false;
    }

    if (mjson_get_string(r->params, r->params_len, "$.ssid", ssid, sizeof(ssid)) > 0)
    {
      obj["wifi"][type]["ssid"] = ssid;

    }
    //If a string is not found, return -1. If a string is found, return the length of unescaped string

    if ( mjson_get_string(r->params, r->params_len, "$.pass", pass, sizeof(pass)) > 0)
    {
      obj["wifi"][type]["pass"] = pass;
    }

    Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
    loadConfig();                 // update behivor
    //jsonrpc_return_success(r, "%s", message, r->userdata); // Report success
    //serializeJsonPretty(obj, Serial);

  }


}

// --------------------------------------------------------------------------------------------------------- server_set
static void server_set(struct jsonrpc_request * r) {
  char buf[100];
  int ser;
  mjson_get_bool(r->params, r->params_len, "$.enable", &ser);
  mjson_get_string(r->params, r->params_len, "$.type", buf, sizeof(buf));
  //digitalWrite(25,ledOn);              // Set LED to the "on" value
  obj["server"]["enable"] = ser ? true : false;
  obj["server"]["type"] = buf;
  //isSaved = ;

  Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
  //resultState();                    // Let shadow know our new state
  jsonrpc_return_success(r, NULL);  // Report success
  //counter(r);
  loadConfig();
  serializeJson(obj, Serial);
  Serial.println();
}

// --------------------------------------------------------------------------------------------------------- sensors_set
// {"method":"Sensors.Set", "params":{"enable":true}}
static void sensors_set(struct jsonrpc_request * r) {
  //char buf[100];
  int sensors;
  mjson_get_bool(r->params, r->params_len, "$.enable", &sensors);
  //mjson_get_string(r->params, r->params_len, "$.type", buf, sizeof(buf));
  //digitalWrite(25,ledOn);              // Set LED to the "on" value
  obj["sensors"]["enable"] = sensors ? true : false;
  //obj["server"]["type"] = buf;
  //isSaved = ;

  Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
  //resultState();                    // Let shadow know our new state
  jsonrpc_return_success(r, NULL);  // Report success
  //counter(r);
  loadConfig();
  Serial.println();
  serializeJson(obj, Serial);
}

// --------------------------------------------------------------------------------------------------------- sensors_get
// {"method":"Sensors.Get"}
static void sensors_get(struct jsonrpc_request * r) {
  //DynamicJsonDocument msg(1024);
  //String message;
  if (obj["sensors"]["enable"].as<bool>())
  {
    sensorInit();
    ReadSensors();
    //msg["t"] = t;
    //msg["h"] = h;
    //msg["u"] = uv;
    //msg["d"] = db;
    //msg["l"] = lux;
    //msg["c"] = CO;
    //serializeMsgPack(msg, message);
  }

  mjson_printf(sender, NULL,
               "{\"method\":\"Sensors.Report\",\"params\":{\"t\":%d, \"h\":%d, \"u\":%g, \"d\":%d, \"l\":%ld, \"c\":%g}}\n",
               t, h, uv, db, lux, ppm);

  if (obj["lora"]["enable"].as<bool>())
    SendData();

  if (obj["oled"].as<bool>() || obj["neodisplay"].as<bool>())
    PrintOut();
}



// --------------------------------------------------------------------------------------------------------- display_set
static void display_set(struct jsonrpc_request * r) {
  char buf[100];
  //int ser;
  //mjson_get_bool(r->params, r->params_len, "$.enable", &ser);
  mjson_get_string(r->params, r->params_len, "$.msg", buf, sizeof(buf));
  //digitalWrite(25,ledOn);              // Set LED to the "on" value
  //obj["server"]["enable"] = ser ? true : false;
  //obj["display"]["msg"] = buf;
  //isSaved = ;

  //Serial.println(saveJSonToAFile(&obj, filename) ? "File saved!" : "Error on save File!");
  //resultState();                    // Let shadow know our new state
  jsonrpc_return_success(r, NULL);  // Report success
  //counter(r);
  //loadConfig();
  display1.clear();
  display1.print(buf, white);
  display1.show();
}

// --------------------------------------------------------------------------------------------------------- accesory_set
// {"method":"Accesory.Set","params":{"id":"01","name":"Inventoteca","type":"ergo","network":"lora","local":6}}
// {"method":"Accesory.Set","params":{"id":"01","name":"Inventoteca","type":"ergo","network":"lora","local":203}}
static void accesory_set(struct jsonrpc_request * r) {

  bool listed = false;
  char dev_id[30];
  char dev_type[30];
  char dev_net[30];
  char dev_name[30];
  double  dev_local;

  mjson_get_string(r->params, r->params_len, "$.id", dev_id, sizeof(dev_id));
  mjson_get_string(r->params, r->params_len, "$.type", dev_type, sizeof(dev_type));
  mjson_get_string(r->params, r->params_len, "$.network", dev_net, sizeof(dev_net));
  mjson_get_string(r->params, r->params_len, "$.name", dev_name, sizeof(dev_name));
  mjson_get_number(r->params, r->params_len, "$.local", &dev_local);

  StaticJsonDocument<124> docx;
  JsonObject root = docx.createNestedObject(F(dev_id));
  dev = getJSonArrayFromFile(&dev_doc, device_list); // Load file

  //Serial.println(dev.size());



  if (dev.size() == 0) // no device in list, new device
  {
    Serial.println("First device added");
    root["id"] = dev_id;
    root["type"] = dev_type;
    root["network"] = dev_net;
    root["name"] = dev_name;
    root["local"] = dev_local;

    dev.add(root);
    Serial.println(saveJSonArrayToAFile(&dev, device_list) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
    dev = getJSonArrayFromFile(&dev_doc, device_list);
    serializeJsonPretty(dev_doc, Serial);
  }
  else
  {
    for (JsonArray::iterator it = dev.begin(); it != dev.end(); ++it)
    {

      //if ((*it)["id"].as<const char*>() != dev_id) // ID not already set
      if (strcmp((*it)["id"], dev_id) == 0)
      {
        Serial.println("Device already in list");
        listed = true;

        break;
      }

    }
    if (listed == false) //
    {
      Serial.println("New device add to list");
      root["id"] = dev_id;
      root["type"] = dev_type;
      root["network"] = dev_net;
      root["name"] = dev_name;
      root["local"] = dev_local;

      dev.add(root);
      Serial.println(saveJSonArrayToAFile(&dev, device_list) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
      dev = getJSonArrayFromFile(&dev_doc, device_list);
      serializeJsonPretty(dev_doc, Serial);
    }
  }

}


// --------------------------------------------------------------------------------------------------------- accesory_del
// {"method":"Accesory.Del","params":{"id":"01"}}
static void accesory_del(struct jsonrpc_request * r) {

  char dev_id[30];
  mjson_get_string(r->params, r->params_len, "$.id", dev_id, sizeof(dev_id));

  dev = getJSonArrayFromFile(&dev_doc, device_list);


  for (JsonArray::iterator it = dev.begin(); it != dev.end(); ++it) {
    if ((*it)["id"] == dev_id) {
      dev.remove(it);
    }
  }


  Serial.println(saveJSonArrayToAFile(&dev, device_list) ? "{\"file_saved\":true}" : "{\"file_saved\":false}");
  serializeJsonPretty(dev_doc, Serial);



}


// ------------------------------------------------------------------------------------------------------ ReadSerial
// JSON RPC
void ReadSerial()
{
  if (Serial.available() > 0) {
    int len = Serial.readBytes(buf, sizeof(buf));
    //rpc_input = SERIAL;
    jsonrpc_process(buf, len, sender, NULL, NULL);
  }
}


//################################################################----------------------- setup--------------------- #############################
void setup()
{
  //Heltec.begin(true /*DisplayEnable */, true /*LoRa*/, true /*Serial Enable*/);

  Heltec.begin(true     /*DisplayEnable*/, true /*LoRa */, true /*Serial */, true /*PABOOST */, BAND /* BAND*/);
  //Wire.begin(); //


  //Heltec.begin(true, false, true);
  //
  //Serial.begin(115200);
  jsonrpc_init(NULL, NULL);
  jsonrpc_export("Config.Get", Cfg_get);
  jsonrpc_export("Config.Set", Cfg_set);
  jsonrpc_export("WiFi.Set", Wifi_set);
  jsonrpc_export("Server.Set", server_set);
  jsonrpc_export("Sensors.Set", sensors_set);
  jsonrpc_export("Sensors.Get", sensors_get);
  jsonrpc_export("Display.Set", display_set);
  jsonrpc_export("Accesory.Set", accesory_set);
  jsonrpc_export("Accesory.Del", accesory_del);
  jsonrpc_export("LoRa.Send", lora_send);
  //jsonrpc_export("count", counter);
  pinMode(FACTORY_BT, INPUT);

  // SPIFFS Init
  if (!SPIFFS.begin(true)) {
    //Serial.println("An Error has occurred while mounting SPIFFS");
    Serial.println("{\"spiffs\":false}");
    return;
  }

  Serial.println("{\"spiffs\":true}");
  Cfg_get(NULL);
  loadConfig();

  //reportState(NULL);
  //Heltec.display->flipScreenVertically();
  //Heltec.display->clear();
  //Heltec.display->display();
  //delay(100);
  //Heltec.display->drawString(0, 0, WiFi.macAddress());
  //Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  // Heltec.display->drawString(0, 10, String(ssid));
  //Heltec.display->display();
  //delay(100);

  //WiFi.disconnect(true); // remove wifi details stored already
  //WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP_STA);
  WiFi.onEvent(WiFiEvent);




}


//#################################--------------------------------------------- loop------------------------###################################
void loop()
{
  ReadSerial();

  if (obj["sensors"]["enable"].as<bool>()) // Sensor Panel normal
  {
    ReadSensors();
    //PrintOut();
    if (obj["lora"]["enable"].as<bool>())
    {
      SendData();
    }


  }

  if (obj["wifi"]["sta"]["enable"].as<bool>())
  {
    checkServer();
    if ((obj["mqtt"]["enable"].as<bool>()) && (client.connected()))
    {
      client.loop();
    }
  }


  if (obj["lora"]["enable"].as<bool>())
  {
    onReceive(LoRa.parsePacket());
  }

  if (digitalRead(FACTORY_BT) == LOW)
  {
    bt_count++;
    if (bt_count > 10000)
    {
      Serial.println("{\"factory_reset\":true}");
      // Save config
      // obj not save complete ssid, better use doc
      doc["wifi"]["sta"]["ssid"] = "";
      doc["wifi"]["sta"]["pass"] = "";
      doc["wifi"]["sta"]["enable"] = true;
      doc["wifi"]["sta"]["count"] = 0;
      doc["wifi"]["sta"]["registered"] = false;
      doc["email"] = "inventotk@gmail.com";
      Serial.println(saveJSonToAFile(&obj, filename) ? "{\"file_saved\":true}" : "{\"file_saved\":false}" );
      ESP.restart();
    }
    //Serial.println(bt_count);
  }
  else bt_count = 0;

}
