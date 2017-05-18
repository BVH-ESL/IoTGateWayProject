// basic forward data from pc to sensor node.
// When GW get data from pc via MQTT and send it to SN.
// if send success GW send status #OK to pc and wait data from SN.
// if send not success GW send status #ERR to pc.

// v1 GW sub topic "esl/<nodeID>/in/+"

// include WiFi and MQTT library
#include <ESP8266WiFi.h>
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

enum states {
  ST_NONE = 0, ST_RST, ST_ARMED, ST_SAMPLING, ST_READY
};
typedef enum states state_t;
state_t state = ST_RST;

enum commands {
  CMD_NONE = 0, CMD_CHECK, CMD_TEMP, CMD_SET
};
int cmd = 0;

// topic variable
char oTopic[32];

// MQTT payload data
#define PAYLOAD_BUF_SIZE 116
char payloadBuf[PAYLOAD_BUF_SIZE + 1];
char payloadDummy[PAYLOAD_BUF_SIZE+1];
uint8_t payloadBufIndex = 0;

// sensor variable
word sensorNodeAddr[] = {0x1234, 0x1235};
uint8_t sensorNodeIndex = 0;

// include mRF24 lib
#include <SPI.h>
#include <mrf24j.h>

// init mrf Module
const int pin_reset = 4;
const int pin_cs = 2;
const int pin_interrupt = 5;
Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);

// init mrf module
void initMRF() {
  SPI.setFrequency(4000000ul);
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

void interrupt_routine() {
  mrf.interrupt_handler(); // mrf24 object interrupt routine
}

void handle_rx() {
  if (cmd == CMD_CHECK) {
    sprintf(oTopic, "esl/%d/out/status", sensorNodeIndex);
  }else if(cmd == CMD_TEMP){
    sprintf(oTopic, "esl/%d/out/data/temp", sensorNodeIndex);
  }
  client.publish(oTopic, mrf.get_rxinfo()->rx_data, mrf.rx_datalength());
}

void handle_tx() {
  // sad path SN not avaliable
  if (!mrf.get_txinfo()->tx_ok) {
    sprintf(oTopic, "esl/%d/out/status", sensorNodeIndex);
    client.publish(oTopic, "#ERR");
    cmd = CMD_NONE;
  } else {
    sprintf(oTopic, "esl/%d/out/status", sensorNodeIndex);
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
    if (topicIndex == 1) {
      sensorNodeIndex = atoi(topicTmp);
    }
    if ( !strcmp(topicTmp, "cmd")) {
      // forwarding command to Sensornode
      mrf.send16(sensorNodeAddr[sensorNodeIndex - 1], payloadBuf);
      checkCMD(payloadBuf); //check command type
      strcpy(payloadBuf, payloadDummy);
    }
    topicIndex++;
    topicTmp = strtok(NULL, "/");
  }
}

void checkCMD(char * input){
  cmd = CMD_NONE;
  if(strtok(input, " ") != NULL){
    if(!strcmp(input, "CHECK")){
      cmd = CMD_CHECK;
    }else if(!strcmp(input, "GET/TEMP")){
      cmd = CMD_TEMP;
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("...");
  for(uint8_t i=0; i<PAYLOAD_BUF_SIZE; i++){
    payloadDummy[i] = ' ';
  }
  payloadDummy[PAYLOAD_BUF_SIZE] = '\0';
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
        client.subscribe("esl/+/in/+");
      }
    }
    if (client.connected())
      client.loop();
  }

  // check state
  switch (state) {
    case ST_RST:
      break;

    default:
      state = ST_RST;
      break;
  }
}
