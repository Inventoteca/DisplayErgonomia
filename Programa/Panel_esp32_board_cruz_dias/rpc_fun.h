//#include "mjson.h"  // Sketch -> Add File -> Add mjson.h
//int rpc_input = NONE;

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
