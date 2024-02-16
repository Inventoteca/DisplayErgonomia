#ifndef LORASERVICE_H
#define LORASERVICE_H

#include "system.h"

//--------------- LoRa definitios
//#define NODEID        1    //should always be 1 for a Gateway
//#define NETWORKID     100  //the same on all nodes that talk to each other
#define FREQUENCY     RF69_915MHZ
//#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW_HCW  //uncomment only for RFM69HW/HCW! Leave out if you have RFM69W/CW!
#define ENABLE_ATC    //comment out this line to disable AUTO TRANSMISSION CONTROL

extern int nodeid;
extern int networkid;
extern const char* lora_key;
extern byte ackCount;
extern uint32_t packetCount;
extern char payload[];
extern byte sendSize;
extern bool success;

extern DynamicJsonDocument msg;


#ifdef ENABLE_ATC
  extern RFM69_ATC radio;
#else
  extern RFM69 radio;
#endif


extern bool spy; //set to 'true' to sniff all packets on the same network

void init_lora();
void receive_lora();
void send_lora();
void prepare_payload();

#endif  // 
