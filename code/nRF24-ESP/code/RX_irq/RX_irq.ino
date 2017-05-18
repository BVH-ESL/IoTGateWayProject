#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(4, 2);
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };
//byte address[][5] = { 0xCC,0xCE/,0xCC,0xCE,0xCC , 0xCE,0xCC,0xCE,0xCC,0xCE};

char buf[32];
void setup(){
  Serial.begin(115200);

  radio.begin();
  radio.enableAckPayload();                         // We will be using the Ack Payload feature, so please enable it
  radio.enableDynamicPayloads();
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();
  radio.printDetails();
  pinMode(5, INPUT_PULLUP);
  attachInterrupt(5, check_radio, FALLING);
}

void loop(/* arguments */) {
  /* code */
  // unsigned long time = millis();                   // Take the time, and send it.
  // Serial.print(F("Now sending "));
  // Serial.println(time);
  // radio.startWrite( "Test Send Form ESP", sizeof("Test Send Form ESP") ,0);
  // delay(2000);
}

void check_radio(){
  bool tx,fail,rx;
  radio.whatHappened(tx,fail,rx);
//  Serial.println("Found Interrupt");/

  if(tx){
    Serial.println(F("Ack Payload:Sent"));
  }

  if(fail){ Serial.println("Ack Payload:Failed");}

  if(rx){
    radio.read( &buf, sizeof(buf) );
    Serial.print(F("Got payload "));
    Serial.println(buf);
    radio.writeAckPayload( 1, &buf, sizeof(buf) );
  }
}
