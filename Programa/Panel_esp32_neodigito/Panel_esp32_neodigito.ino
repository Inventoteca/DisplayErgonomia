/* NeoDigito example code: Hello

  Print "Hello" on display, color select

  Created by Inventoteca 

  https://github.com/Inventoteca/NeoDigito

  This example code is in the public domain.
  Remember that you must have installed Adafruit_NeoPixel library.
*/
#include <Arduino.h>
#include <NeoDigito.h>
#include "DHT.h"
#include <MQUnifiedsensor.h>
#include <WiFi.h> //import for wifi functionality
#include <WiFiMulti.h>
#include <WebSocketsServer.h> //import for websocket
#include "SPIFFS.h"
#include "FS.h"
#include <ArduinoJson.h>
#include "mjson.h"  // Sketch -> Add File -> Add mjson.h
#include "sensors.h"

JsonObject obj;
DynamicJsonDocument doc(1024);
//const char *ssid =  "SmartPanel";   //Wifi SSID (Name)   
//const char *pass = "12345678"; //wifi password
const char *filename = "/config.json";
File file;



#define MIC 36 

#define DHTPIN 4     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#define PIN 21        // Pin where the display will be attached
#define DIGITS 17      // Number of NeoDigitos connected
#define PIXPERSEG 2   // NeoPixels per segment
//Definitions
#define placa "Arduino UNO"
#define Voltage_Resolution 5
#define mq_pin 37 //Analog input
#define type "MQ-135" //MQ135
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  
//float cFactor = 0;

// Specified the number of displays and the number of neopixels per segment, some arguments of the neopixel strip used must be added. 
NeoDigito display1 = NeoDigito(DIGITS, PIXPERSEG, PIN, NEO_GRB + NEO_KHZ800); // For more info abaut the last argumets check Adafruit_Neopixel documentation.
DHT dht(DHTPIN, DHTTYPE);

WebSocketsServer webSocket = WebSocketsServer(81); //websocket init with port 81
void webSocketEvent(uint8_t num, WStype_t w_type, uint8_t * payload, size_t length);
WiFiMulti wifiMulti;



//Declare Sensor
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, mq_pin, type);
//JsonObject getJSonFromFile(DynamicJsonDocument *doc, String filename, bool forceCleanONJsonError = true );
int t;
int h;
int db;


const int sampleWindow = 50;                              // Sample window width in mS (50 mS = 20Hz)
int sample;
const int tempSample = 2000;                              // Sample window width in mS (50 mS = 20Hz)
unsigned long tempRefresh= 0;
const int soundSample =1500;                              // Sample window width in mS (50 mS = 20Hz)
unsigned long soundRefresh= 0;
const int airSample = 1000;                              // Sample window width in mS (50 mS = 20Hz)
unsigned long airRefresh= 0;
unsigned long startMillis= 0;
int contador;
const uint32_t connectTimeoutMs = 60000;
unsigned long  timestamp;
//static int ledOn = 0;  // Current LED status
char buf[800];
int i;
uint8_t numx;

// -------------------------------------------------------------------------------ReadSensors()
void ReadSensors()
{
   float peakToPeak = 0;                                  // peak-to-peak level
   unsigned int signalMax = 0;                            //minimum value
   unsigned int signalMin = 1024;                         //maximum value
   unsigned int sum = 0;
  
  
  // ------------------------------------- Temperature and Humidity
  if(millis() - tempRefresh > tempSample)
  {
    t = dht.readTemperature();
    h = dht.readHumidity();
    tempRefresh = millis();
    // Check if any reads failed and exit early (to try again).
    display1.setCursor(0);
    if (isnan(h) || isnan(t) || (h > 100) || (t > 100)) 
    {
      //Serial.print(F("Failed to read from DHT sensor!"));
      display1.print("--");
      display1.print("--");
      //Serial.print("--");
      //Serial.print("\t");
      //Serial.print("--");
      //Serial.print("\t");
      //return;
      
    }
    else
    {
      display1.print(t);
      display1.print(h); 
      //Serial.print(t);
      //Serial.print("\t");
      //Serial.print(h);
      //Serial.print("\t");
      //Serial.println();
    }

    // Set colors 
    if(t>25)
      display1.updateColor(red,0,1);
    else if(t>21)
      display1.updateColor(white,0,1);
    else
      display1.updateColor(blue,0,1);

    if(h>60)
      display1.updateColor(red,2,3);
    else
      display1.updateColor(white,2,3);
  }
 
   // ---------------------- Sound meter
  if(millis() - soundRefresh > soundSample)
  {
    for(int i=0;i<3;i++)
    {
     startMillis = millis();                   // Start of sample window
     while (millis() - startMillis < sampleWindow)
     {
       sample = analogRead(MIC);                    //get reading from microphone
       if (sample < 1024)                                  // toss out spurious readings
       {
          if (sample > signalMax)
           {
              signalMax = sample;                           // save just the max levels
           }
           else if (sample < signalMin)
           {
              signalMin = sample;                           // save just the min levels
           }
        }
     }
   
     peakToPeak = signalMax - signalMin;                    // max - min = peak-peak amplitude
     db = map(peakToPeak,10,700,49.5,90);             //calibrate for deciBels
     sum = sum + db;
   }

   db = sum/3;
   if(db>95)
      db = 95;

   //Serial.print(db);
   //Serial.println();
   display1.setCursor(6);
   if(db < 10)
      display1.print(" ");
   else if(db < 100)
      display1.print(" ");
   display1.print(db);
   
   if(db>70)
      display1.updateColor(red,6,8);
   else
      display1.updateColor(white,6,8);
   
   soundRefresh = millis();
  }

  // -------------------------------- Air Quality
  // ------------------------------------- Temperature and Humidity
  if(millis() - airRefresh > airSample)
  {
    float CO;
    
    for(int i = 0;i < 10; i++)
    {
      MQ135.update();
      CO = CO + MQ135.readSensor();
      delay(50);
    }
    //cFactor = getCorrectionFactor(t, h);
    //float CO = MQ135.readSensor(false, cFactor); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
    //MQ135.setA(605.18); MQ135.setB(-3.937); 
    CO = CO/10;
    if(CO>9999)
        CO = 9999;
    //Serial.print(CO);
    //Serial.println();
    
    display1.setCursor(13);
    display1.print("    ");
  
    if(CO < 10)
       display1.setCursor(15);
    else if(CO < 100)
       display1.setCursor(15);
    else if(CO < 1000)
       display1.setCursor(14);
    else
       display1.setCursor(13);
     
     display1.print(String(CO,0));
     
     if(CO>500)
        display1.updateColor(red,13,16);
     else
        display1.updateColor(white,13,16);

     airRefresh = millis();
  }
  display1.show();
}


// -------------------------------------------------- saveJSonToAFile
bool saveJSonToAFile(JsonObject *doc, String filename) {
    //SD.remove(filename);
 
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    Serial.println(F("Open file in write mode"));
    file = SPIFFS.open(filename, FILE_WRITE);
    if (file) {
        Serial.print(F("Filename --> "));
        Serial.println(filename);
 
        Serial.print(F("Start write..."));
 
        serializeJson(*doc, file);
 
        Serial.print(F("..."));
        // close the file:
        file.close();
        Serial.println(F("done."));
 
        return true;
    } else {
        // if the file didn't open, print an error:
        Serial.print(F("Error opening "));
        Serial.println(filename);
 
        return false;
    }
}


// ------------------------------------------- getJsonFromFile
JsonObject getJSonFromFile(DynamicJsonDocument *doc, String filename, bool forceCleanONJsonError = true ) 
{
    // open the file for reading:
    file = SPIFFS.open(filename);;
    if (file) 
    {
      Serial.println("Opening File");
      //return;

    size_t size = file.size();
    Serial.println(size);
    
    if(size > 1024)
    {
      Serial.println("Too large file");
      //return false;
    }
 
    DeserializationError error = deserializeJson(*doc, file);
    if (error) 
    {
      // if the file didn't open, print an error:
      Serial.print(F("Error parsing JSON "));
      Serial.println(error.c_str());
 
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
        Serial.print(F("Error opening (or file not exists) "));
        Serial.println(filename);
 
        Serial.println(F("Empty json created"));
        return doc->to<JsonObject>();
    }
 
}




//----------------------------------------------------------------- webSender
//void webSender(uint8_t num,int len,char cmd[200],)
//{
 
//}

// -------------------------- websocket_event
void webSocketEvent(uint8_t num, WStype_t w_type, uint8_t * payload, size_t length)
{
  //webscket event method
 // StaticJsonDocument<200> webDoc;

    char cmd[200];
    int len;
    
    switch(w_type) {
        case WStype_DISCONNECTED:
            Serial.println("Websocket is disconnected");
            //case when Websocket is disconnected
            break;
        case WStype_CONNECTED:{
            //wcase when websocket is connected
            Serial.println("Websocket is connected");
            Serial.println(webSocket.remoteIP(num).toString());
            webSocket.sendTXT(num, "connected");}
            break;
        case WStype_TEXT:
            //cmd[0] = '\0';// Empty the cmd
            memset(cmd,0,length+1);
            for(int i = 0; i < length; i++) {
              //  cmd = cmd + (char) payload[i]; 
               cmd[i] = (char)payload[i]; 
            } //merging payload to single string
            //Serial.println(cmd);

            //if(cmd == "poweron")
            //{ //when command from app is "poweron"
                //Serial.println(obj["wifi"]["sta"]["ssid"].as<const char*>());
                //wifi_Init(obj["wifi"]["sta"]["ssid"].as<const char*>(),obj["wifi"]["sta"]["pass"].as<const char*>());
               // wifiMulti.addAP(obj["wifi"]["sta"]["ssid"].as<const char*>(),obj["wifi"]["sta"]["pass"].as<const char*>());
               // obj["wifi"]["sta"]["enable"] = true;  
               // obj["wifi"]["ap"]["enable"] = false; 
               // obj["cmd"]=cmd;       

            //deserializeMsgPack(doc, cmd);
              //deserializeJson(doc, cmd);
              //deserializeJson(webDoc, cmd);
              
              len = strlen(cmd);
              numx=num;
              jsonrpc_process(cmd, len, senderWS, NULL, NULL);
              //webSender(num,cmd,len);
              //serializeJson(obj,Serial);
              
                       
                //boolean isSaved = saveJSonToAFile(&obj, filename);
 
                //if (isSaved)
                //{
                //  Serial.println("File saved!");
                //}
                //else
                //{
                //  Serial.println("Error on save File!");
                //}
                //digitalWrite(ledpin, HIGH);   //make ledpin output to HIGH  
                //display1.updateColor(Rainbow);
                //display1.show();
           // }
            //else if(cmd == "poweroff")
            //{
            //    display1.updateColor(Rainbow);
            //    display1.show();
                //digitalWrite(ledpin, LOW);    //make ledpin output to LOW on 'pweroff' command.
            //}

             //webSocket.sendTXT(num, cmd);
             //send response to mobile, if command is "poweron" then response will be "poweron:success"
             //this response can be used to track down the success of command in mobile app.
            break;
        case WStype_FRAGMENT_TEXT_START:
            break;
        case WStype_FRAGMENT_BIN_START:
            break;
        //case WStype_BIN:
        //    hexdump(payload, length);
            break;
        default:
            break;
    }
}

// ---------------------------------- sensorInit
void sensorInit()
{
  dht.begin();
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(605.18); MQ135.setB(-3.937); // Configure the equation to calculate CO concentration value
  MQ135.init();
  Serial.print("Calibrating MQ135 please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10); 
  pinMode(MIC,INPUT);
}

// --------------------------------------- dispalyInit
void displayInit()
{
  display1.begin();             // This fuction calls Adafruit_NeoPixel.begin() to configure.
  display1.clear();  
  //display1.setColor(yellow); // Color specified by a 32bit hex, or 8bit numbers (red, green, blue), Also colors names, red, white, yellow, etc.    
  display1.print("0123456789ABCDEFG");      // It prints the value.
  display1.updateColor(Rainbow);
  display1.show();              // Lights up the pixels.
  Serial.println("Display ready");
  delay(300);
  display1.clear();  
  display1.show();
  display1.print("oC",red); 
  display1.print("% ",green);
  display1.print("uV",purple);
  display1.print("dB ",blue);
  display1.print("luxe",white);
  display1.print("PPN.N",cian);
  //display1.print("8:8.",white);
  display1.show();
  delay(300);
  display1.clear();
  display1.updateColor(Random,13,16);
  Serial.println("Display  done!.");
  //display1.updateColor(random(0,0xFFFFFF),13,16);
  //display1.print(contador,0,0,255);      // It prints the value.
  display1.show();
}


// ---------------------------------------- loadConfig
void loadConfig()
{
  //serializeJson(obj,Serial);
  //StaticJsonDocument<1024> doc;
  //deserializeJson(doc,file);
  //obj = doc.as<JsonObject>();
  
  if(obj["wifi"]["sta"]["enable"].as<bool>() && (wifiMulti.run() != WL_CONNECTED))
  {
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(obj["wifi"]["sta"]["ssid"].as<const char*>(),obj["wifi"]["sta"]["pass"].as<const char*>());
    Serial.println("Connecting to WiFi");
    //WiFi.begin(doc["wifi"]["sta"]["ssid"].as<const char*>(),doc["wifi"]["sta"]["pass"].as<const char*>());
    //WiFi.begin(ssid,pass);
    //Serial.println(ssid);
    //Serial.println(pass);
    if(wifiMulti.run() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println(WiFi.SSID());
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      //obj["wifi"]["ap"]["enable"] = false;      
    }
  }
  
  if(obj["wifi"]["ap"]["enable"].as<bool>())
  {
    ap_Init(obj["wifi"]["ap"]["ssid"].as<const char*>(),obj["wifi"]["ap"]["pass"].as<const char*>());
  }
  else
  {
    WiFi.softAPdisconnect(true);
  }

  //serializeJson(obj,Serial);
  Serial.println("Config Ready");
  
}

// ------------------------------------------------ ap_init
void ap_Init(const char *ssid, const char *pass)
{
  Serial.println("Starting AP");
  IPAddress apIP(192, 168, 0, 1);   //Static IP for wifi gateway
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); //set Static IP gateway on NodeMCU
  WiFi.softAP(ssid, pass); //turn on WIFI 
  Serial.println("AP Ready");
  Serial.println(ssid);
  websocketInit();
}


// --------------------------------------------- checkServer
void checkServer()
{
    if(millis() - timestamp > connectTimeoutMs)  //
  { 
    if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
      Serial.print("WiFi connected: ");
      Serial.print(WiFi.SSID());
      Serial.print(" ");
      Serial.println(WiFi.RSSI());
      //obj["wifi"]["ap"]["enable"] = false;
      //WiFi.softAPdisconnect(true);
      
    }
    else {
      Serial.println("WiFi not connected!");
      //obj["wifi"]["ap"]["enable"] = true;
      
    }

    loadConfig();
    Serial.println();

    timestamp = millis();
  }
}


// ---------------------------------------------- websockerInit
void websocketInit()
{
  webSocket.begin(); //websocket Begin
  webSocket.onEvent(webSocketEvent); //set Event for websocket
  Serial.println("Websocket is started");
}

// ------------------------------------------------- sender (Serial)
static int sender(const char *frame, int frame_len, void *privdata) {
  return Serial.write(frame, frame_len);
}

// ------------------------------------------------- senderWS (WS)
static int senderWS(const char *frame, int frame_len, void *privdata) {
  return webSocket.sendTXT(numx, frame);
  //return webSocket.sendTXT(fra, cmd);
}


//---------------------------------------------------- reportState
static void reportState(struct jsonrpc_request *r) {
  mjson_printf(sender, NULL,
               "{\"method\":\"Shadow.Report\",\"params\":{\"on\":%s}}\n",
               digitalRead(25) ? "true" : "false");
}

//---------------------------------------------------- count
static void counter(struct jsonrpc_request *r) {
  i++;
  mjson_printf(senderWS, NULL,"%d",i);
  //return i;    
}


// ---------------------------------------------- resultState
static void resultState(void) {
  mjson_printf(sender, NULL,
               "{\"result\":{\"on\":%s}}\n",
               digitalRead(25) ? "true" : "false");
}

// ---------------------------------------------- Cfg_get
static void Cfg_get(struct jsonrpc_request *r) {
  int ledOn = 0;
  mjson_get_bool(r->params, r->params_len, "$.on", &ledOn);
  digitalWrite(25,ledOn);              // Set LED to the "on" value
  //obj["wifi"]["ap"]["enable"] = ledOn;
  //resultState();                    // Let shadow know our new state
  //jsonrpc_return_success(r, NULL);  // Report success
  counter(r);
}


//------------------------------------------------------------------------------- setup
void setup()
{
  Serial.begin(115200);
  jsonrpc_init(NULL, NULL);
  jsonrpc_export("Config.Set", Cfg_get);
  jsonrpc_export("cmd", reportState);
  jsonrpc_export("count", counter);
  pinMode(25, OUTPUT);  // Configure LED pin

  // Sensors Init
  sensorInit();
   
  // Display Init
  displayInit();
   

  // SPIFFS Init
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  obj = getJSonFromFile(&doc, filename);
  
  
  tempRefresh = millis();
  soundRefresh = millis();
  airRefresh = millis();
  timestamp = millis();
  //ap_Init(obj["wifi"]["ap"]["ssid"].as<const char*>(),obj["wifi"]["ap"]["pass"].as<const char*>());
  loadConfig();
  reportState(NULL);
  
}


//----------------------------------------------------------------------------- loop
void loop()
{
  
  ReadSensors();
  webSocket.loop(); //keep this line on loop method
  //checkServer();

  
  if (Serial.available() > 0) {
    int len = Serial.readBytes(buf, sizeof(buf));
    jsonrpc_process(buf, len, sender, NULL, NULL);
  }
  //delay(500);

}
