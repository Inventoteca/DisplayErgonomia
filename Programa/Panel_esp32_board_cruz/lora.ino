//--------------- LoRa definitios
#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6
#define NONE      0
#define SERIAL    1
#define LORA      2

//------------------- LoRa Variables
//unsigned int counter = 0;
String rssi = "RSSI --";
String packSize = "--";
String packet ;
String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages

byte destination = 6;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends
String message;
//char c_msg[30];


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
