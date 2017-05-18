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
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const int pin_reset = 4;
const int pin_cs = 2; 
const int pin_interrupt = 5; 
Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);

// define wifi ssid and password
const char* SSID = "ESL_Lab1";
const char* PASS = "wifi@esl";

//const char* SSID = "294/40";
//const char* PASS = "1qaz2wsx";

WiFiUDP Udp;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  Serial.println("connect wifi");
  WiFi.begin(SSID, PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("can't connect WiFi");
    return;
  }
}

void initMRF() {
  SPI.setFrequency(8000000ul);
  mrf.reset();
  mrf.init();
  mrf.write_short(MRF_RFCON3, 0x00);
  
  mrf.set_pan(22);
  Serial.println(mrf.get_pan());
  
  // This is _our_ address
  mrf.address16_write(0x1292);

  // uncomment if you want to enable PA/LNA external control
  mrf.set_palna(true);

  // set interrupt
  attachInterrupt(5, interrupt_routine, FALLING);
  interrupts();
}

void setup() {
  Serial.begin(115200);
  Serial.println("...");

  connectWiFi();

  initMRF();
}

void interrupt_routine() {
  mrf.interrupt_handler(); // mrf24 object interrupt routine
}

void loop() {
  mrf.check_flags(&handle_rx, &handle_tx);
}

void handle_rx() {
  Udp.beginPacket("192.168.1.182", 21567);
  Serial.println("\r\nASCII data (relevant data):");
  for (int i = 0; i < mrf.rx_datalength(); i++) {
    Serial.write(mrf.get_rxinfo()->rx_data[i]);
    Udp.write(mrf.get_rxinfo()->rx_data[i]);
  }
  Udp.flush();
  Udp.endPacket();
  delay(10);
}

void handle_tx() {

}
