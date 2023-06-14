//#include <PubSubClient.h>

//--------------- MQTT
//WiFiClient espClient;
//PubSubClient client(espClient);


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
