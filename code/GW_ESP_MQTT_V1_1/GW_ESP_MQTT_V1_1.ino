// basic forward data from pc to sensor node.
// When GW get data from pc via MQTT and send it to SN.
// if send success GW send status #OK to pc and wait data from SN.
// if send not success GW send status #ERR to pc.

// v1.1 change topic for sub
// "esl/<sensorType>/<sensorAddr>/<sensorPAN>/in/cmd"
// EX topic
// mRF select SN "esl/mrf/0x1234/0x16/in/cmd"
// mRF broadcast "esl/mrf/0xFFFF/0x16/in/cmd"


// include WiFi and MQTT libra//ry
#
//#include <ESP8266WiFi.h>///
#include <WiFi.h>//
#include <PubSubClient.h>

// init WiFi AP
const char *ssid =  "ESL_Lab1";
const char *pass =  "wifi@esl";
IPAddress server(192, 168, 1, 9);

//const char *ssid =  "294/40";
//const char *pass =  "1qaz2wsx";
//IPAddress server(192, 168, 1, 113);

// init wifi client and MQTT client
WiFiClient wclient;
PubSubClient client(wclient);

//enum states {
//  ST_NONE = 0, ST_IDLE, ST_ARMED, ST_SAMPLING, ST_READY
//};
//typedef enum states state_t;
//state_t state = ST_IDLE;

enum links {
  LT_NONE = 0, LT_MRF, LT_NRF, LT_BLE, LT_SGB
};
uint8_t link = 0;

enum commands {
  CMD_NONE = 0, CMD_CHECK, CMD_TEMP, CMD_SET
};
uint8_t cmd = 0;

// MQTT payload data
#define PAYLOAD_BUF_SIZE 115
char payloadBuf[PAYLOAD_BUF_SIZE + 1];
char payloadDummy[PAYLOAD_BUF_SIZE + 1];

// sensor node vairable
char sensorNodeLink[8];
uint16_t sensorNodeAddr;
uint8_t sensorNodePAN;

char oTopic[64];

// include mRF24 lib
#include <SPI.h>
#include <mrf24j.h>

// init mrf Module
uint8_t pin_reset = 4;
uint8_t pin_cs = 2;
uint8_t pin_interrupt = 5;
Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);

// init mrf module

void interrupt_routine() {
  mrf.interrupt_handler(); // mrf24 object interrupt routine
}

void initMRF() {
  SPI.setFrequency(8000000ul);
  mrf.reset();
  mrf.init();
  mrf.write_short(MRF_RFCON3, 0x00);

  mrf.set_pan(0x16);
  Serial.println(mrf.get_pan());

  // This is _our_ address
  mrf.address16_write(0x1292);

  // uncomment if you want to enable PA/LNA external control
  mrf.set_palna(true);

  // set interrupt
  attachInterrupt(5, interrupt_routine, FALLING);
  interrupts();
}

void handle_rx() {
  char * payloadTmp;
  payloadTmp = strtok((char *)mrf.get_rxinfo()->rx_data, " ");//
    Serial.println(payloadTmp);
  if (payloadTmp != NULL) {
    if (cmd == CMD_CHECK) {
      sprintf(oTopic, "esl/%s/0x%x/0x%x/out/status", sensorNodeLink, sensorNodeAddr, sensorNodePAN);
    }
    else if (cmd == CMD_TEMP) {
      sprintf(oTopic, "esl/%s/0x%x/0x%x/out/data", sensorNodeLink, sensorNodeAddr, sensorNodePAN);
    }
    client.publish(oTopic, payloadTmp);
  }
//  strncpy(payloadTmp, (char *)mrf.get_rxinfo()->rx_data, mrf.rx_datalength());
//  /Serial.println(strlen(payloadTmp));
//  while (payloadTmp != NULL) {/
//    /Serial.println(payloadTmp);
    

//    Serial.println(strlen(oTopic));
//    Serial.println(oTopic);
//    Serial.println("clear aTopic at RX");
//    //    client.publish(oTopic, "abc");
//  }/
}

void handle_tx() {
  // sad path SN not avaliable
  sprintf(oTopic, "esl/%s/0x%x/0x%x/out/status", sensorNodeLink, sensorNodeAddr, sensorNodePAN);
  if (!mrf.get_txinfo()->tx_ok) {
    client.publish(oTopic, "#ERR");
    cmd = CMD_NONE;
  } else {
    client.publish(oTopic, "#OK");
  }
}

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

void checkCMD(char * input, int length) {
  cmd = CMD_NONE;
  if (!strncmp(input, "CHECK", length)) {
    cmd = CMD_CHECK;
  } else if (!strncmp(input, "GET/TEMP", length)) {
    cmd = CMD_TEMP;
  }
}

void sendToSN() {
  switch (link) {
    case LT_MRF:
      mrf.set_pan(sensorNodePAN);
      mrf.send16(sensorNodeAddr, payloadBuf);
      strcpy(payloadBuf, payloadDummy);
      break;
  }
}

// mqtt callback when msg arrive
void callback(char* topic, byte* payload, unsigned int length) {
  // copy payload to char array
  strncpy(payloadBuf, (char *)payload, length);
  payloadBuf[PAYLOAD_BUF_SIZE] = '\0';

  // split MQTT topic
  char * topicTmp;
  uint8_t topicIndex = 0;
  topicTmp = strtok(topic, "/");
  while (topicTmp != NULL) {
    if ( !strcmp(topicTmp, "mrf")) {
      link = LT_MRF;
      strncpy(sensorNodeLink, topicTmp, strlen(topicTmp));
      sensorNodeLink[strlen(topicTmp)] = '\0';
    } else if (topicIndex == 2) {
      sensorNodeAddr = strtol(topicTmp, NULL, 16);
    } else if (topicIndex == 3) {
      sensorNodePAN = strtol(topicTmp, NULL, 16);
    } else if ( !strcmp(topicTmp, "cmd")) {
      checkCMD((char *)payload, length);
      sendToSN();
    }
    topicIndex++;
    topicTmp = strtok(NULL, "/");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("...");
  for (uint8_t i = 0; i < PAYLOAD_BUF_SIZE; i++) {
    payloadDummy[i] = ' ';
  }
  payloadDummy[PAYLOAD_BUF_SIZE] = '\0';
  strcpy(payloadBuf, payloadDummy);
  connectWiFi();
  initMRF();
}

void loop() {
  // put your main code here, to run repeatedly:
  // check mrf flags
  mrf.check_flags(&handle_rx, &handle_tx);

  // check MQTT connection to broker
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      client.setServer(server, 1883);
      if (client.connect("arduinoClient")) {
        client.setCallback(callback);
        client.subscribe("esl/+/+/+/in/+");
      }
    }
    if (client.connected())
      client.loop();
  }

  // check state
  //  switch (state) {
  //    case ST_IDLE:
  //      break;
  //
  //    default:
  //      state = ST_IDLE;
  //      break;
  //  }
}
