/**
   Example code for using a microchip mrf24j40 module to receive only
   packets using plain 802.15.4
   Requirements: 3 pins for spi, 3 pins for reset, chip select and interrupt
   notifications
   This example file is considered to be in the public domain
   Originally written by Karl Palsson, karlp@tweak.net.au, March 2011
*/
#include <SPI.h>
#include <mrf24j.h>

const int pin_reset = 4;
const int pin_cs = 2; // default CS pin on ATmega8/168/328
const int pin_interrupt = 5; // default interrupt pin on ATmega8/168/328

Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);

word gateWayAddr = 0x1292;

#define PAYLOAD_BUF_SIZE 115
char payloadBuf[PAYLOAD_BUF_SIZE+1];
char payloadDummy[PAYLOAD_BUF_SIZE+1];

enum states {
  ST_NONE = 0, ST_RST, ST_ARMED, ST_SAMPLING, ST_READY
};

enum commands {
  CMD_NONE = 0, CMD_GET, CMD_SET, CMD_ACK, CMD_TEMP
};

int cmd = 0;

typedef enum states state_t;
state_t state = ST_RST;

void setup() {
  Serial.begin(115200);
  
  for(uint8_t i=0; i<PAYLOAD_BUF_SIZE; i++){
    payloadDummy[i] = ' ';
  }
  payloadDummy[PAYLOAD_BUF_SIZE] = '\0';
  
  SPI.setFrequency(4000000ul);
  mrf.reset();
  mrf.init();
  mrf.write_short(MRF_RFCON3, 0x00);
  mrf.set_pan(0x16);
  Serial.print("PAN ");
  Serial.println(mrf.get_pan());
  // This is _our_ address
  mrf.address16_write(0x1234);
  Serial.print("Addr ");
  Serial.println(mrf.address16_read());
  // uncomment if you want to enable PA/LNA external control
  mrf.set_palna(true);

  attachInterrupt(5, interrupt_routine, FALLING); // interrupt 0 equivalent to pin 2(INT0) on ATmega8/168/328
  interrupts();
}

void interrupt_routine() {
  mrf.interrupt_handler(); // mrf24 object interrupt routine
}

void loop() {
  mrf.check_flags(&handle_rx, &handle_tx);
  switch (state) {
    case ST_RST:
      if (cmd == CMD_GET) {
        break;
      } else if (cmd == CMD_ACK) {
        memcpy(payloadBuf, "#ACKED", PAYLOAD_BUF_SIZE);
        payloadBuf[PAYLOAD_BUF_SIZE] = '\0';
        Serial.print(payloadBuf);
        Serial.println("ss");
        mrf.send16(gateWayAddr, payloadBuf);
        strcpy(payloadBuf, payloadDummy);
        cmd = CMD_NONE;
      } else if (cmd == CMD_TEMP) {
        strcpy(payloadBuf, "129");
        payloadBuf[PAYLOAD_BUF_SIZE] = '\0';
        mrf.send16(gateWayAddr, payloadBuf);
        strcpy(payloadBuf, payloadDummy);
        cmd = CMD_NONE;
      }
    default:
      state = ST_RST;
      break;
  }
}

void checkCMD(char * input){
  cmd = CMD_NONE;
  if(strtok(input, " ") != NULL){
    if(!strcmp(input, "CHECK")){
      cmd = CMD_ACK;
    }else if(!strcmp(input, "GET/TEMP")){
      cmd = CMD_TEMP;
    }
  }
}

void handle_rx() {
  cmd = CMD_NONE;
  char * payloadTmp;
  Serial.write(mrf.get_rxinfo()->rx_data, mrf.rx_datalength());
  Serial.println(" rxData");
  payloadTmp = strtok((char *)mrf.get_rxinfo()->rx_data, " ");//
//  Serial.println(payloadTmp);
  if (payloadTmp != NULL) {
    checkCMD(payloadTmp);
  }
}

void handle_tx() {
  // code to transmit, nothing to do
  if (mrf.get_txinfo()->tx_ok) {
    //    mrf.rx_flush();
    cmd = CMD_NONE;
    state = ST_RST;
  }
}
