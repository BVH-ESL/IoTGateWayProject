// check connect AP
#include <ESP8266WiFi.h>

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

// define wifi ssid and password
// const char* SSID = "ESL_Lab1";
// const char* PASS = "wifi@esl";

const char* SSID = "khanchuea";
const char* PASS = "1qaz2wsx";

#define CPUFREQ 80
#define power 20
#define mainProcessPin 2       // D6 pin

void incrementState(){
  digitalWrite(mainProcessPin, LOW);
  delayMicroseconds(10);
  digitalWrite(mainProcessPin, HIGH);
}

void setup() {
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  system_update_cpu_freq(CPUFREQ);
  pinMode(mainProcessPin, OUTPUT);
}

void loop() {
  incrementState();
  delay(1000);
  incrementState();
  delay(3000);
}
