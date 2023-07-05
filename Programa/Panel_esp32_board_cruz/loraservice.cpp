#include "loraservice.h"

bool spy = false; //set to 'true' to sniff all packets on the same network
int nodeid; //= obj["nodeid"].as<int>();
int nodeid_remote; //= obj["nodeid"].as<int>();
int networkid;// = obj["networkid"].as<int>();
const char* lora_key;// = obj["lora_key"].as<char *>();
byte ackCount = 0;
uint32_t packetCount = 0;
char payload[1024] = "";
byte sendSize = 0;

#ifdef ENABLE_ATC
RFM69_ATC radio;
#else
RFM69 radio;
#endif


// --------------------------------------------------------- init_lora
void init_lora()
{
  nodeid = obj["nodeid"].as<int>();
  nodeid_remote = obj["nodeid_remote"].as<int>();
  networkid = obj["networkid"].as<int>();
  lora_key = obj["lora_key"].as<const char*>();
  //spy = obj["lora_spy"].as<bool>();

  //size_t key_size = strlen(temp_key);
  //char lora_key[key_size + 1];  // +1 para el carácter de terminación nulo
  //strcpy(lora_key, temp_key);


  radio.initialize(RF69_915MHZ, nodeid, networkid);
#ifdef IS_RFM69HW_HCW
  radio.setHighPower(); //must include this only for RFM69HW/HCW!
#endif
  radio.encrypt(lora_key);
  radio.spyMode(spy);
  //radio.setFrequency(916000000); //set frequency to some custom frequency
  //char buff[50];
  //sprintf(buff, "\nListening at %d Mhz...", FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  //Serial.println(buff);
  //#ifdef ENABLE_ATC
  //Serial.println("RFM69_ATC Enabled (Auto Transmission Control)");
  //#endif
  Serial.println("{\"init_lora\":true}");
}


// -------------------------------------------------------- receive_lora
void receive_lora()
{
  if (radio.receiveDone())
  {
    memset(payload, 0, sizeof(payload));
    for (byte i = 0; i < radio.DATALEN; i++)
    {
      payload[i] = (char)radio.DATA[i];
    }
    Serial.print("{\"RX_RSSI\":"); Serial.print(radio.RSSI); Serial.println("}");
    Serial.println(payload);

    if (radio.ACKRequested())
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();
      Serial.println("{\"ACK_send\":true}");
    }
    else
      Serial.println  ("{\"ACK_send\":false}");

    DynamicJsonDocument lora_doc(1024);
    deserializeJson(lora_doc, payload);

    JsonObject lora_obj = lora_doc.as<JsonObject>();
    serializeJson(lora_obj, Serial);
    Serial.println();

    // -------------------------------------------------- cruz
    if (obj["type"].as<String>() == "cruz")
    {
      dias = lora_obj["dias"];
      mes = lora_obj["mes"];
      anio = lora_obj["anio"];
      dia_hoy = lora_obj["dia_hoy"];

    }
    PrintOut();
  }
}


// ------------------------------------------------------ send_lora
void send_lora()
{
  prepare_payload();
  sendSize = strlen(payload);
  //Serial.print("Sending[");
  //Serial.print(sendSize);
  //Serial.print("]: ");
  //for (byte i = 0; i < sendSize; i++)
  //  Serial.print((char)payload[i]);
  //Serial.println();

  Serial.print("{\"SendFrom\":"); Serial.print(nodeid); Serial.println("}");
  Serial.print("{\"SendTo\":"); Serial.print(nodeid_remote); Serial.println("}");
  
  if (radio.sendWithRetry(nodeid_remote, payload, sendSize))
  {
    Serial.println("{\"ACK\":true}");
  }
  else Serial.println("{\"ACK\":false}");

}


// ------------------------------------------------------ prepare_payload
void prepare_payload()
{
  //memset(payload, 0, sizeof(payload));
  DynamicJsonDocument msg(1024);
  String message;

  // ------------------------------------------ ergo
  if (obj["type"].as<String>() == "ergo")
  {
    //msg["sensors"]["t"] = t;
    //msg["sensors"]["h"] = h;
    //msg["sensors"]["uv"] = int(uv * 10); // avoid float
    //msg["sensors"]["db"] = db;
    //msg["sensors"]["lux"] = lux;
    //msg["sensors"]["ppm"] = ppm;
  }
  // ------------------------------------------ cruz
  else if (obj["type"].as<String>() == "cruz")
  {
    //sprintf(payload, "{\"dias\": %d}", dias);
    //dias = int(round(round(now.unixtime() - last_ac.unixtime()) / 86400L));
    //mes = now.month();
    //anio = now.year();
    //dia_hoy;
    msg["dias"] = dias;
    msg["mes"] = mes;
    msg["anio"] = anio;
    msg["dia_hoy"] = dia_hoy;
    serializeJson(msg, payload);

  }


  //serializeJson(msg, message);
  Serial.println(payload);
}
