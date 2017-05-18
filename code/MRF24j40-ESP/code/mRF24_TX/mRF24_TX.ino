#include <SPI.h>
#include <mrf24j.h>

const int pin_reset = 4;
const int pin_cs = 2; // default CS pin on ATmega8/168/328
const int pin_interrupt = 5; // default interrupt pin on ATmega8/168/328s
Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);
long last_time;
long tx_interval = 1000;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("...");

  // init mrf module
  SPI.setFrequency(8000000ul);
  mrf.reset();
  mrf.init();
  
  // set pan 
  mrf.set_pan(22);
  Serial.println(mrf.get_pan());
  
  // This is _our_ address
  mrf.address16_write(0x1234);

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
  mrf.interrupt_handler(); // mrf24 object interrupt routine
}

void loop() {
  // put your main code here, to run repeatedly:
  mrf.check_flags(&handle_rx, &handle_tx);
  unsigned long current_time = millis();
  if (current_time - last_time > tx_interval) {
    last_time = current_time;
    Serial.println("txxxing...");
    mrf.send16(0x4202, "hello");
  }
}

void handle_rx(){
}

void handle_tx() {
  if (mrf.get_txinfo()->tx_ok) {
    Serial.println("TX went ok, got ack");
  } else {
    Serial.print("TX failed after "); Serial.print(mrf.get_txinfo()->retries); Serial.println(" retries\n");
  }
}
