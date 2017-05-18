#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define BUF_SIZE (32)
// init CE and CSN pin
RF24 radio(9, 10);

byte address[][5] = { 0xCC, 0xCE, 0xCC, 0xCE, 0xCC , 0xCE, 0xCC, 0xCE, 0xCC, 0xCE};

char buf[BUF_SIZE];
uint8_t buf_index = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);         // DEBUG pin
  Serial1.begin(38400);
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.enableAckPayload();                         // We will be using the Ack Payload feature, so please enable it
  radio.enableDynamicPayloads();
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(1, address[1]);

  attachInterrupt(digitalPinToInterrupt(2), checkRadio, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:
  getCMD();
}

void checkRadio() {
  bool tx, fail, rx;
  radio.whatHappened(tx, fail, rx);
  if (tx) {
    Serial.println("Send : OK");
  }

  if (fail) {
    Serial.println("Send : Failed");
  }

  if (rx || radio.available()) {
    radio.read(&buf, sizeof(buf));
    Serial.print("got payload");
    Serial1.println(buf);
  }
}

void getCMD() {
  if ( Serial1.available() > 0 ) {
    char ch = Serial1.read();
    if ( (buf_index < BUF_SIZE) && (ch != '\n') ) {
      buf[ buf_index++ ] = ch;
    }
    if ( ch == ':' ) {
      buf[ buf_index ] = '\0';
      buf_index = 0;
      radio.startWrite(&buf, BUF_SIZE, 0);
    }
    
  }
}

