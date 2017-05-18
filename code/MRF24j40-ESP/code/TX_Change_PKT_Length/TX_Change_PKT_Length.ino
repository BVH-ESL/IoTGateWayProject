#include <SPI.h>
#include <mrf24j.h>

// init MRF24 Pin
const int pin_reset = 4;
const int pin_cs = 2; // default CS pin on ATmega8/168/328
const int pin_interrupt = 5; // default interrupt pin on ATmega8/168/328s
Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);
long last_time;
long tx_interval = 5;

#define BUF_SIZE 64
char BUF[BUF_SIZE];
int pkt_size [] = {2, 4, 8, 16, 32, 64};
uint8_t pkt_index = 0;
String pkt;

#define debugPin 15

void change_pkt_size(){
  if (digitalRead(0) == LOW)
    pkt_index++;
}

void toggleDebug(){
  digitalWrite(debugPin, LOW);
  delayMicroseconds(10);
  digitalWrite(debugPin, HIGH);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("...");
    pinMode(debugPin, OUTPUT);
  digitalWrite(debugPin, HIGH);
  // init buff packet
  for(uint8_t i = 0; i<BUF_SIZE; i++){
    pkt+='a';
  }
//  Serial.println(pkt.substring(BUF_SIZE - pkt_size[pkt_index%6]));
  attachInterrupt(0, change_pkt_size, FALLING);
  
  // init MRF
  SPI.setFrequency(4000000ul);
  mrf.reset();
  mrf.init();
//  mrf.write_short(MRF_RFCON3, 0x00);

  mrf.set_pan(22);
  Serial.println(mrf.get_pan());
  // This is _our_ address
  mrf.address16_write(0x6001);

  // uncomment if you want to receive any packet on this channel
  mrf.set_promiscuous(false);

  // uncomment if you want to enable PA/LNA external control
  mrf.set_palna(true);

  // uncomment if you want to buffer all PHY Payload
  mrf.set_bufferPHY(false);

  attachInterrupt(pin_interrupt, interrupt_routine, FALLING); // interrupt 0 equivalent to pin 2(INT0) on ATmega8/168/328
  last_time = millis();
  interrupts();
}

void interrupt_routine() {
//  toggleDebug();
  mrf.interrupt_handler(); // mrf24 object interrupt routine
}

void loop() {
  // put your main code here, to run repeatedly:
  mrf.check_flags(&handle_rx, &handle_tx);
  unsigned long current_time = millis();
  if (current_time - last_time > tx_interval) {
//    toggleDebug();
    last_time = current_time;
//    Serial.println("txxxing...");
 
    char pkt_tmp[pkt_size[pkt_index%6]];
    pkt.toCharArray(pkt_tmp, pkt_size[pkt_index%6]+1);
//    toggleDebug();
    mrf.send16(0x4202, pkt_tmp);
  }
}

void handle_rx(){
  
}

void handle_tx() {
//  toggleDebug();
//  if (mrf.get_txinfo()->tx_ok) {
//    Serial.println("TX went ok, got ack");
//  } else {
//    Serial.print("TX failed after "); Serial.print(mrf.get_txinfo()->retries); Serial.println(" retries\n");
//  }
}
