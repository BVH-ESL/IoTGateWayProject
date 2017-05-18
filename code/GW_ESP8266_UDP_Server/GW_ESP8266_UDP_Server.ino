#include <SPI.h>
#include <mrf24j.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const int pin_reset = 4;
const int pin_cs = 2; // default CS pin on ATmega8/168/328
const int pin_interrupt = 5; // default interrupt pin on ATmega8/168/328

Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);

// define wifi ssid and password
const char* SSID = "ESL_Lab1";
const char* PASS = "wifi@esl";

WiFiUDP UDPServer;
unsigned int UDPPort = 8081;

#define packetSize 32
byte packetBuffer[packetSize];

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  Serial.println("connect wifi");
  WiFi.begin(SSID, PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("can't connect WiFi");
    return;
  }
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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

void handleUDPServer() {
  int cb = UDPServer.parsePacket();
  if (cb) {
    UDPServer.read(packetBuffer, packetSize);
    for(int i = 0; i < packetSize; i++) {
     Serial.write(packetBuffer[i]);
    }
    Serial.println(" ");
    mrf.send16(0x1234, (char *)packetBuffer);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("...");

  connectWiFi();

  initMRF();

  UDPServer.begin(UDPPort);
}

void interrupt_routine() {
  mrf.interrupt_handler(); // mrf24 object interrupt routine
}

void loop() {
  // put your main code here, to run repeatedly:
  mrf.check_flags(&handle_rx, &handle_tx);
  handleUDPServer();
}

void handle_rx() {
  
}

void handle_tx(){
  if (mrf.get_txinfo()->tx_ok) {
    Serial.println("TX went ok, got ack");
  } else {
    Serial.print("TX failed after "); Serial.print(mrf.get_txinfo()->retries); Serial.println(" retries\n");
  }
}

