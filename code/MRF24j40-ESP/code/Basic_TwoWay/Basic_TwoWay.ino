/**
   Example code for using a microchip mrf24j40 module to send and receive
   packets using plain 802.15.4
   Requirements: 3 pins for spi, 3 pins for reset, chip select and interrupt
   notifications
   This example file is considered to be in the public domain
   Originally written by Karl Palsson, karlp@tweak.net.au, March 2011
*/
#include <SPI.h>
#include <mrf24j.h>
//#include <ES

const int pin_reset = 4;
const int pin_cs = 2; // default CS pin on ATmega8/168/328
const int pin_interrupt = 5; // default interrupt pin on ATmega8/168/328

Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);

long last_time;
long tx_interval = 1000;

#define debugPin 15

void toggleDebug() {
  digitalWrite(debugPin, LOW);
  delayMicroseconds(10);
  digitalWrite(debugPin, HIGH);
}

void setup() {
  //  WiFi.mode(WIFI_OFF);
  //  WiFi.forceSleepBegin();
  pinMode(debugPin, OUTPUT);
  digitalWrite(debugPin, HIGH);
  Serial.begin(115200);
  Serial.println();
  Serial.println("...");
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

  attachInterrupt(5, interrupt_routine, FALLING); // interrupt 0 equivalent to pin 2(INT0) on ATmega8/168/328
  last_time = millis();
  interrupts();
}

void interrupt_routine() {
  toggleDebug();
  mrf.interrupt_handler(); // mrf24 object interrupt routine
}

void loop() {
  mrf.check_flags(&handle_rx, &handle_tx);
  unsigned long current_time = millis();
  if (current_time - last_time > tx_interval) {
    last_time = current_time;
    Serial.println("txxxing...");
    mrf.send16(0x4202, "abcd");
  }
}

void handle_rx() {
  Serial.print("received a packet "); Serial.print(mrf.get_rxinfo()->frame_length, DEC); Serial.println(" bytes long");

  if (mrf.get_bufferPHY()) {
    Serial.println("Packet data (PHY Payload):");
    for (int i = 0; i < mrf.get_rxinfo()->frame_length; i++) {
      Serial.print(mrf.get_rxbuf()[i]);
    }
  }

  Serial.println("\r\nASCII data (relevant data):");
  for (int i = 0; i < mrf.rx_datalength(); i++) {
    Serial.write(mrf.get_rxinfo()->rx_data[i]);
  }

  Serial.print("\r\nLQI/RSSI=");
  Serial.print(mrf.get_rxinfo()->lqi, DEC);
  Serial.print("/");
  Serial.println(mrf.get_rxinfo()->rssi, DEC);
}

void handle_tx() {
  toggleDebug();
  if (mrf.get_txinfo()->tx_ok) {
    Serial.println("TX went ok, got ack");
  } else {
    Serial.print("TX failed after "); Serial.print(mrf.get_txinfo()->retries); Serial.println(" retries\n");
  }
}
