#include "sensors.h"

bool sensors_init = false;
DHT dht(DHTPIN, DHTTYPE);
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
bool tsl_status = false;
unsigned long tempRefresh = 0;
int last_t;
int last_h;
int last_db;

int t;
int h;
int db;
//double ppm;
unsigned int ppm;
float uv;
unsigned long lux;
sensors_event_t event;

float m = -0.318; //Slope
float b = 1.133; //Y-Intercept
//float R0 = 11.820; //Sensor Resistance in fresh air f
float R0 = 9.000; //Sensor Resistance in fresh air f
int adc;
int adc_33;
float outputVoltage;



// ---------------------------------------------------------------------------------------------- sensorInit
void sensorInit()
{
  if (sensors_init == false)
  {
    // --------------------------- temperature & humidity
     Wire.begin(); 
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


// --------------------------------------------------------------------------------ReadSensors()
void ReadSensors()
{

 if (obj["sensors_enable"] == true) // Sensors available
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

    if ((millis() - tempRefresh) >= obj["sensors_time"].as<unsigned int>() /*tempSample*/)
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
      //t = t + (obj["t_cal"].as<int>());

      // -------------------------- Humidity
      h = dht.readHumidity();

      // Check if any reads failed.
      if (isnan(h) || isnan(t) || (h > 100) || (t > 100) || (t <= 0))
      {
        h = last_h;
        t = last_t;
        Serial.println("{\"dht\":false}");

      }
      else  //temperature and humidity ok
      {
        last_t = t;
        last_h = h;
      }


      // ---------------------------- UV
      adc = analogRead(UV_PIN);
      //adc_33 = analogRead(VR_PIN);  // Comented for no reference pin
      adc_33 = 4094;
      outputVoltage = 3.3 / adc_33 * adc;

      uv = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0);
      if (uv <= 0) uv = 0.0;  // zero for error
      else uv = roundf(uv * 10) / 10; // 2 decimals
      uv = floor(uv * 10) / 10.0;



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
      ppm = ppm + (obj["ppm_cal"].as<int>()); // Fresh air
      if (ppm > 9999) ppm = 9999;
      //double percentage = ppm / 10000; //Convert to percentage

      //PrintOut();
      //SendData();

    }
  }

}

// ----------------------------------------------------------------------------------- mapFloat
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
