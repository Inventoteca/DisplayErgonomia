#include "display.h"

bool neo_digits_status = false;
bool blk = false;
short color_status[3] = {0, 0, 0};
uint32_t color = 0x00FF00;
//unsigned long printRefresh = 0;
//unsigned long printTime = 1000;

NeoDigito display1 = NeoDigito(DIGITS, PIXPERSEG, NEO_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip(238, NEO_PIN, NEO_GRB + NEO_KHZ800);
int pixelCruz[] = {0, 2, 7, 12, 16, 21, 27, 30, 37, 43, 51, 55, 62, 69, 75, 82, 90, 95, 104, 112, 123, 130, 140, 150, 159, 169, 180, 188, 200, 211, 222, 232, 239};


// ---------------------------------------------------------------------------------------------------------- dispalyInit
void displayInit()
{

  if (neo_digits_status == false)
  {
    if (obj["type"].as<String>() == "ergo")
    {
      display1.setPin(obj["neo_pin"].as<int>());
      //display1.setPin(25);
      display1.updateDigitType(obj["digits"].as<int>(), obj["pixels"].as<int>());
      //display1.updateDigitType(17,2);

      display1.begin();             // This fuction calls Adafruit_NeoPixel.begin() to configure.
      //display1.clear();
      //display1.show();

    }
    else if (obj["type"].as<String>() == "cruz")
    {
      Serial.print("{\"neo_pin\":"); Serial.print(obj["neo_pin"].as<int>()); Serial.println("}");
      strip.setPin(obj["neo_pin"].as<int>());
      strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
      strip.clear();
      if (obj["test"].as<bool>())
        strip.fill(0xFFFFFF);
      strip.show();            // Turn OFF all pixels ASAP
      strip.setBrightness(255);

      Serial.print("{\"neo_pin_days\":"); Serial.print(obj["neo_pin_days"].as<int>()); Serial.println("}");
      display1.setPin(obj["neo_pin_days"].as<int>());
      display1.updateDigitType(obj["digits_days"].as<int>(), obj["pixels_day"].as<int>());
      display1.begin();
      display1.clear();
      if (obj["test"].as<bool>())
        display1.print(8888, 0xFFFFFF);

      display1.show();


      Serial.print("{\"neo_pin_year\":"); Serial.print(obj["neo_pin_year"].as<int>()); Serial.println("}");
      display1.updateDigitType(obj["digits_year"].as<int>(), obj["pixels_year"].as<int>());
      display1.setPin(obj["neo_pin_year"].as<int>());
      display1.begin();
      display1.clear();
      if (obj["test"].as<bool>())
        display1.print(8888, 0xFFFFFF);
      display1.show();

      Serial.println("{\"strip_croix\":true}");
      if (obj["test"].as<bool>())
        delay(1000);
    }

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


}


// --------------------------------------------------------------------------------------- PrintOut
void PrintOut()
{
  if (neo_digits_status)
  {
    //Serial.println("printout");
    if (obj["enable_oled"].as<bool>())
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

    if (obj["enable_neo"].as<bool>())
    {
      //Serial.println("neodisplay");
      if (obj["type"].as<String>() == "ergo")
      {
        /*   display1.setCursor(0);
           if (t < 10)
             display1.print(" ");
           display1.print(t, (t >= (obj["t_max"].as<int>())) ?
                          (obj["t_colMax"].as<uint32_t>()) : (t <= (obj["t_min"].as<int>())) ?
                          (obj["t_colMin"].as<uint32_t>()) : (obj["t_colDef"].as<uint32_t>()));
           display1.setCursor(2);
           if (h < 10)
             display1.print(" ");
           display1.print(h, (h >= (obj["h_max"].as<int>())) ?
                          (obj["h_colMax"].as<uint32_t>()) : (h <= (obj["h_min"].as<int>())) ?
                          (obj["h_colMin"].as<uint32_t>()) : (obj["h_colDef"].as<uint32_t>()));
           display1.setCursor(4);
           if (uv == 0)
             display1.print("0.0", (uv >= (obj["uv_max"].as<int>())) ?
                            (obj["uv_colMax"].as<uint32_t>()) : (uv <= (obj["uv_min"].as<int>())) ?
                            (obj["uv_colMin"].as<uint32_t>()) : (obj["uv_colDef"].as<uint32_t>()));
           else
             display1.print(String(uv, 1), (uv >= (obj["uv_max"].as<int>())) ?
                            (obj["uv_colMax"].as<uint32_t>()) : (uv <= (obj["uv_min"].as<int>())) ?
                            (obj["uv_colMin"].as<uint32_t>()) : (obj["uv_colDef"].as<uint32_t>()));
           display1.setCursor(6);
           if (db < 10)
             display1.print("  ");
           else if (db < 100)
             display1.print(" ");
           display1.print(db, (db >= (obj["db_max"].as<int>())) ?
                          (obj["db_colMax"].as<uint32_t>()) : (db <= (obj["db_min"].as<int>())) ?
                          (obj["db_colMin"].as<uint32_t>()) : (obj["db_colDef"].as<uint32_t>()));
           display1.setCursor(9);
           if (lux < 10)
             display1.print("   ");
           else if (lux < 100)
             display1.print("  ");
           else if (lux < 1000)
             display1.print(" ");
           display1.print(lux, (lux >= (obj["lux_max"].as<int>())) ?
                          (obj["lux_colMax"].as<uint32_t>()) : (lux <= (obj["lux_min"].as<int>())) ?
                          (obj["lux_colMin"].as<uint32_t>()) : (obj["lux_colDef"].as<uint32_t>()));
           display1.setCursor(13);
           if (ppm < 10)
             display1.print("   ");
           else if (ppm < 100)
             display1.print("  ");
           else if (ppm < 1000)
             display1.print(" ");
           else if (ppm > 9999)ppm = 9999;
           display1.print(ppm, (ppm >= (obj["ppm_max"].as<int>())) ?
                          (obj["ppm_colMax"].as<uint32_t>()) : (ppm <= (obj["ppm_min"].as<int>())) ?
                          (obj["ppm_colMin"].as<uint32_t>()) : (obj["ppm_colDef"].as<uint32_t>()));
           display1.show();
          }*/
      }
      else if (obj["type"].as<String>() == "cruz")
      {
        Serial.print("{\"days\":");
        //Serial.print(int(round(round(now.unixtime() - last_ac.unixtime()) / 86400L)));
        Serial.print(dias);
        Serial.println("}");

        if (obj["enable_neo"].as<bool>())
        {
          display1.setPin(obj["neo_pin_days"].as<int>());
          display1.updateDigitType(obj["digits_days"].as<int>(), obj["pixels_day"].as<int>());
          display1.begin();
          display1.clear();

          if (dias < 1000)
            display1.print(" ");
          if (dias < 100)
            display1.print(" ");
          if (dias < 10)
            display1.print(" ");

          display1.print(dias, color);
          display1.show();



          display1.updateDigitType(obj["digits_year"].as<int>(), obj["pixels_year"].as<int>());
          display1.setPin(obj["neo_pin_year"].as<int>());
          display1.begin();
          display1.clear();
          if (mes < 10)
            display1.print("0");
          display1.print(mes, color);
          anio = anio - 2000;
          if (anio < 10)
            display1.print(" ");
          display1.print(anio, color);
          display1.show();
        }
      }

      strip.clear();
      int days_index;
      int pix_start, pix_end;
      if (color <= 0)
        color = 0x00FF00;
      for (days_index = 1; days_index <= dia_hoy; days_index++)
        //for (days_index = 1; days_index <= int(now.day()); days_index++)
        //for (days_index = 1; days_index <= 31; days_index++)
      {
        pix_start = pixelCruz[days_index - 1];
        pix_end = (pixelCruz[days_index]) - (pixelCruz[days_index - 1]);
        Serial.print(days_index);
        Serial.print("\t");
        serializeJson(obj["events"][String(days_index)], Serial);
        Serial.print("\t");
        Serial.print(pix_start);
        Serial.print("\t");
        Serial.println(pix_end);
        //Serial.print(" ");
        //

        if ((obj["events"][String(days_index)].isNull() == false) /*&& (obj["events"].isNull() == false)*/)
        {
          if (obj["events"][String(days_index)] == 1)       // Casi accidente
          {
            color = 0xFFA500;
            strip.fill(color, pix_start, pix_end);       // Orange
          }


          else if (obj["events"][String(days_index)] == 2)  // Primer auxilio
          {
            color = 0xFF;
            strip.fill(color, pix_start, pix_end);           // Blue
          }

          else if (obj["events"][String(days_index)] == 3) // No incapacitante
          {
            color = 0xFFFF00;
            strip.fill(color, pix_start, pix_end);      // Yellow
          }

          else if (obj["events"][String(days_index)] == 4)  // Accidente
          {
            color = 0xFF0000;
            strip.fill(color, pix_start, pix_end);       // Red
          }

          else
          {
            color = 0x00FF00;
            strip.fill(color, pix_start, pix_end);   // Sin accidente
          }

        }
        else
        {
          color = 0x00FF00;
          strip.fill(color, pix_start, pix_end);   // Sin accidente
        }
        //strip.clear();
        //esp_task_wdt_reset();
        //delay(500);
      }
      strip.show();



      // ------------------------------------- Status BLink
      if (WiFi.status() == WL_CONNECTED)
      {
        if (blk == true)
        {
          if (obj["type"].as<String>() == "ergo")
            display1.updatePoint(obj["status_pix"].as<int>(), obj["ppm_colDef"].as<uint32_t>());

          blk = false;
          color_status[0] = color_status[1] = color_status[2] = 1;
          //display1.show();
        }
        else
        {
          if (obj["type"].as<String>() == "ergo")
            display1.updatePoint(obj["status_pix"].as<int>(), 0, 0, 0);

          blk = true;
          color_status[0] = color_status[1] = color_status[2] = 0;
          //display1.show();
        }
      }
      // ------------------------------------- Status WiFiEvent
      else
      {
        display1.updatePoint(obj["status_pix"].as<int>(), color_status[0], color_status[1], color_status[2]);
      }
    }
  }
  else
    displayInit();
}
