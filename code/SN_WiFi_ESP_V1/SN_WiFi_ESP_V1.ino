// pub data to broker
// v1 no sleep
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

extern "C" {
#include "c_types.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "lwip/opt.h"
#include "lwip/err.h"
#include "lwip/dns.h"
}

// WiFi AP variable
const char *ssid =  "ESL_Lab1";
const char *pass =  "wifi@esl";

// MQTT Broker IP
IPAddress server(192, 168, 1, 9);

// init wifi client and MQTT client
WiFiClient wclient;
PubSubClient client(wclient);

//
#define btnPin 5
#define mainProcessPin 4
#define BUFSIZE 32

char buf[BUFSIZE];

char oTopic[32];

volatile boolean ext_irq_detected = false;

// connect WiFi AP
void connectWiFi() {
  Serial.println("connecting WiFi AP ...");
  WiFi.begin(ssid, pass);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("can't connect wifi");
    return;
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

void connectMQTTBroker() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      client.setServer(server, 1883);
      if (client.connect("arduinoClient")) {
        client.setCallback(callback);
      }
    }
    if (client.connected())
      client.loop();
  }
}

// mqtt callback when msg arrive
void callback(char* topic, byte* payload, unsigned int length) {

}

void trigger_isr() {
//  if (!ext_irq_detected) {
    ext_irq_detected = true;
//    state = ST_TESTING;
//  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("..");
  pinMode(mainProcessPin, OUTPUT);
  pinMode(btnPin, INPUT);
  for (int i = 0; i < BUFSIZE; i++) {
    buf[i] = 'x';
  }
  attachInterrupt(btnPin, trigger_isr, RISING);
  connectWiFi();
}

void loop() {
  // put your main code here, to run repeatedly:
  connectMQTTBroker();
  if (ext_irq_detected) {
    Serial.println("start test");
    detachInterrupt(btnPin);
    for (int i = 0; i < 1000; i++) {
      sprintf(oTopic, "esl/SN/%4d", i);
      client.publish(oTopic, buf);
      delay(100);
    }
    ext_irq_detected = false;
    delay(1000);
    attachInterrupt(btnPin, trigger_isr, RISING);
    Serial.println("finish test");
  }
//  else{
//    attachInterrupt(btnPin, trigger_isr, RISING);
//  }
}
