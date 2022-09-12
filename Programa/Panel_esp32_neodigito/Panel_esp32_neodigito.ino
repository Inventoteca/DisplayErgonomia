/* NeoDigito example code: Hello

  Print "Hello" on display, color select

  Created by Inventoteca 

  https://github.com/Inventoteca/NeoDigito

  This example code is in the public domain.
  Remember that you must have installed Adafruit_NeoPixel library.
*/

#include <NeoDigito.h>
#include "DHT.h"
#include <MQUnifiedsensor.h>


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
//Declare Sensor
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, mq_pin, type);
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



//------------------------------------------------------------------------------- setup
void setup()
{
  Serial.begin(115200);
  dht.begin();
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(605.18); MQ135.setB(-3.937); // Configure the equation to calculate CO concentration value
  MQ135.init(); 
  pinMode(MIC,INPUT);
  display1.begin();             // This fuction calls Adafruit_NeoPixel.begin() to configure.
  display1.clear();  
  //display1.setColor(yellow); // Color specified by a 32bit hex, or 8bit numbers (red, green, blue), Also colors names, red, white, yellow, etc.    
  display1.print("0123456789ABCDEFG");      // It prints the value.
  display1.updateColor(Rainbow);
  display1.show();              // Lights up the pixels.
  Serial.println("hecho");
  delay(3000);
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
  delay(3000);
  display1.clear();
  display1.updateColor(Random,13,16);

  Serial.print("Calibrating MQ135 please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!.");
  
  
  tempRefresh = millis();
  soundRefresh = millis();
  airRefresh = millis();
}


//----------------------------------------------------------------------------- loop
void loop()
{
  
  ReadSensors();
  delay(500);

}

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
      Serial.print("--");
      Serial.print("\t");
      Serial.print("--");
      Serial.print("\t");
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
    Serial.print(CO);
    Serial.println();
    
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
  

  //display1.updateColor(random(0,0xFFFFFF),13,16);
  //display1.print(contador,0,0,255);      // It prints the value.
  display1.show();
}
