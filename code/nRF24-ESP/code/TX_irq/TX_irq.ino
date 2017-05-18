#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
//#include <ESP8266WiFi.h>/

//extern "C" {
//#include "c_types.h"
//#include "ets_sys.h"
//#include "os_type.h"
//#include "osapi.h"
//#include "mem.h"
//#include "user_interface.h"
//#include "smartconfig.h"
//#include "lwip/opt.h"
//#include "lwip/err.h"
//#include "lwip/dns.h"
//}

RF24 radio(4, 2);
// static uint32_t message_count = 0;

// Topology
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };
static uint32_t message_count = 0;

void setup(){
//  WiFi.mode(WIFI_OFF);/
//  WiFi.forceSleepBegin();/
  Serial.begin(115200);

  radio.begin();
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();               // Allow optional ack payloads
  radio.setRetries(5, 15);                 // Smallest time between retries, max no. of retries
  radio.enableDynamicPayloads();   
  // radio.setPayloadSize(1);
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.printDetails();
   radio.stopListening();
  pinMode(5, INPUT_PULLUP);
  attachInterrupt(5, check_radio, FALLING);
}
unsigned long times = millis();
void loop(/* arguments */) {
  radio.write( &message_count, sizeof(message_count) );
  Serial.print("send : ");
  Serial.println(message_count);
  delay(5000);
  /* code */
  // yield();
  // if(millis() - times > 2000){
  //
  //   times = millis();                   // Take the time, and send it.
  //   radio.write( &counter, 1 );
  // }
  // radio.startWrite( &time, sizeof(unsigned long) ,0);
  // delay(2000);
}

void check_radio(){
  // yield();
  bool tx,fail,rx;
  radio.whatHappened(tx,fail,rx);
//  Serial.println("found event");/
  if(tx){
     Serial.println("Send:OK");
    //  counter++;
//      radio.startListening();
 }

  if(fail){ Serial.println("Send:Failed");
//    radio.stopListening();
  }

  if(rx || radio.available()){
//    yield();/
//    delay(0);/
    radio.read(&message_count,sizeof(message_count));
    Serial.print(F("Got Ack: "));
    Serial.println(message_count);
    message_count++;
    // radio.stopListening();
    // radio.write( &counter, 1 );
  }
}
