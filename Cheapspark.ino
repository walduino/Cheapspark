#include <espduino.h>
#include <mqtt.h>
#include <dht.h>
#include <EEPROM.h>
#include <ArduinoJson.h>


#define SETUPSSID "CSsetupwifi"
#define SETUPSSIDPW "cheapspark"
#define SETUPBROKERIP "192.168.43.1"
#define SETUPMQTTCLIENT "cheapspark666"
#define MQTTTOPIC0 "commands"
#define SETUPMQTTTOPIC "setup"
#define DHT_PIN 5
#define REL1_PIN 9
#define REL2_PIN 10
#define REL3_PIN 11
#define REL4_PIN 12


ESP esp(&Serial, 4);
MQTT mqtt(&esp);

const int EEPROM_MIN_ADDR = 0;
const int EEPROM_MAX_ADDR = 511;
char eepromMqttClientName[20];
boolean wifiConnected = false;
int reportInterval =  15000;
int switchInterval = 100;
int pulseInterval = 50;
unsigned long now = 0;
unsigned long nextPub = 30000;
unsigned long nextSwitch = switchInterval;
unsigned long nextPulse = pulseInterval;
int ledpin = 13;
boolean setupmode = false;
boolean switchstate = false;
boolean rel1_pulse = false;
boolean rel2_pulse = false;
boolean rel3_pulse = false;
boolean rel4_pulse = false;
dht DHT;

void wifiCb(void* response){
  uint32_t status;
  RESPONSE res(response);
  if(res.getArgc() == 1) {
    res.popArgs((uint8_t*)&status, 4);
    if(status == STATION_GOT_IP) {        //WIFI CONNECTED
      if (setupmode == true){
        mqtt.connect(SETUPBROKERIP, 1883, false);
        wifiConnected = true;
      } else {
        char eepromBroker[20];
        eeprom_read_string(356, eepromBroker, 20); //broker adress
        mqtt.connect(eepromBroker, 1883, false);
        wifiConnected = true;
      }
    } else {
      wifiConnected = false;
      mqtt.disconnect();
    }
  }
}

void mqttConnected(void* response){
  delay(500);
  if (setupmode == true){
    mqtt.subscribe("/setup");
//    mqtt.subscribe("/" SETUPMQTTCLIENT "/" SETUPMQTTTOPIC);
    mqtt.publish("/fb","ready4setup");

  } else {
    char topic[40];
    strcat(strcat(strcpy(topic, "/"), eepromMqttClientName), "/" MQTTTOPIC0);
    mqtt.subscribe(topic);

    char fbmsg[40];
    strcat(strcpy(fbmsg,eepromMqttClientName), " online in normal mode");
    mqtt.publish("/fb",fbmsg);
  }
}

void mqttDisconnected(void* response){}








void mqttData(void* response){
  RESPONSE res(response);
  char buffer[128];
  String topic = res.popString();
  String data = res.popString();
  if (setupmode == true){
    data.toCharArray(buffer,128);
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(buffer);

    const char* json_ssid = root["SSID"];
    const char* json_password = root["password"];
    const char* json_brokerip = root["broker ip"];
    const char* json_clientname = root["client name"];
    eeprom_write_string(100, (char*)json_ssid);
    eeprom_write_string(228, (char*)json_password);
    eeprom_write_string(356, (char*)json_brokerip);
    eeprom_write_string(484, (char*)json_clientname);
    digitalWrite(ledpin,  !digitalRead(ledpin));
  } else {
    data.toCharArray(buffer,4);
    if (strcmp(buffer,"r11") == 0) digitalWrite(REL1_PIN,HIGH);
    else if (strcmp(buffer,"r10") == 0) digitalWrite(REL1_PIN,LOW);
    else if (strcmp(buffer,"r1p") == 0) rel1_pulse = true;
    else if (strcmp(buffer,"r21") == 0) digitalWrite(REL2_PIN,HIGH);
    else if (strcmp(buffer,"r20") == 0) digitalWrite(REL2_PIN,LOW);
    else if (strcmp(buffer,"r2p") == 0) rel2_pulse = true;
    else if (strcmp(buffer,"r31") == 0) digitalWrite(REL3_PIN,HIGH);
    else if (strcmp(buffer,"r30") == 0) digitalWrite(REL3_PIN,LOW);
    else if (strcmp(buffer,"r3p") == 0) rel3_pulse = true;
    else if (strcmp(buffer,"r41") == 0) digitalWrite(REL4_PIN,HIGH);
    else if (strcmp(buffer,"r40") == 0) digitalWrite(REL4_PIN,LOW);
    else if (strcmp(buffer,"r4p") == 0) rel4_pulse = true;
  }
}

void mqttPublished(void* response){}   //runs when publish is a success

void setup(){
  pinMode(A0,INPUT);
  digitalWrite(A0,HIGH);
  pinMode(REL1_PIN,OUTPUT);
  pinMode(REL2_PIN,OUTPUT);
  pinMode(REL3_PIN,OUTPUT);
  pinMode(REL4_PIN,OUTPUT);
  if (analogRead(0)<500) setupmode = true;
  eeprom_read_string(484, eepromMqttClientName, 20); // read client name intro global var
  //setup ESP
  delay(5000);
  Serial.begin(19200);
  esp.enable();
  delay(500);
  esp.reset();
  delay(500);
  while(!esp.ready());
  //setup mqtt client");
  if (setupmode == true){
    if(!mqtt.begin(SETUPMQTTCLIENT, "", "", 30, 1)) {
      while(1);
    }
  } else {
    if(!mqtt.begin(eepromMqttClientName, "", "", 30, 1)) {
      while(1);
    }
  }
  //setup mqtt lwt
  char ch_lwt[40];
  strcat(strcpy(ch_lwt,eepromMqttClientName), " offline");
  mqtt.lwt("/lwt", ch_lwt, 0, 0);
  //setup mqtt events
  mqtt.connectedCb.attach(&mqttConnected);
  mqtt.disconnectedCb.attach(&mqttDisconnected);
  mqtt.publishedCb.attach(&mqttPublished);
  mqtt.dataCb.attach(&mqttData);
  //setup wifi
  esp.wifiCb.attach(&wifiCb);
  if (setupmode == true) esp.wifiConnect(SETUPSSID,SETUPSSIDPW);
  else {
    char eepromSsid[20];
    char eepromWifiPw[20];
    eeprom_read_string(100, eepromSsid, 20); //ssid
    eeprom_read_string(228, eepromWifiPw, 20); // wifipasswd
    esp.wifiConnect(eepromSsid,eepromWifiPw);
  }
}

boolean eeprom_write_string(int addr, const char* string) {
  // Writes a string starting at the specified address.
  // Returns true if the whole string is successfully written.
  int numBytes;
  numBytes = strlen(string) + 1;
  return eeprom_write_bytes(addr, (const byte*)string, numBytes);
}

boolean eeprom_read_string(int addr, char* buffer, int bufSize) {
  // Reads a string starting from the specified address.
  // Returns true if at least one byte (even only the
  // string terminator one) is read.
  byte ch;  // byte read from eeprom
  int bytesRead;  // number of bytes read so far
  if (!eeprom_is_addr_ok(addr)) return false;   // check start address
  if (bufSize == 0) return false;   // how can we store bytes in an empty buffer ?
  if (bufSize == 1) {   // is there is room for the string terminator only,no reason to go further
    buffer[0] = 0;
    return true;
  }
  bytesRead = 0;   // initialize byte counter
  ch = EEPROM.read(addr + bytesRead);   // read next byte from eeprom
  buffer[bytesRead] = ch;   // store it into the user buffer
  bytesRead++;   // increment byte counter

  // stop conditions:
  // - the character just read is the string terminator one (0x00)
  // - we have filled the user buffer
  // - we have reached the last eeprom address
  while ( (ch != 0x00) && (bytesRead < bufSize) && ((addr + bytesRead) <= EEPROM_MAX_ADDR) ) {
    ch = EEPROM.read(addr + bytesRead);     // if no stop condition is met, read the next byte from eeprom
    buffer[bytesRead] = ch;     // store it into the user buffer
    bytesRead++;     // increment byte counter

  }
  if ((ch != 0x00) && (bytesRead >= 1)) buffer[bytesRead - 1] = 0;  // make sure the user buffer has a string terminator (0x00) as its last byte
  return true;
}

boolean eeprom_write_bytes(int startAddr, const byte* array, int numBytes) {
  int i;  // counter
  if (!eeprom_is_addr_ok(startAddr) || !eeprom_is_addr_ok(startAddr + numBytes)) return false;   // both first byte and last byte addresses must fall within the allowed range
  for (i = 0; i < numBytes; i++) {
    EEPROM.write(startAddr + i, array[i]);
  }
  return true;
}

boolean eeprom_is_addr_ok(int addr) {
  return ((addr >= EEPROM_MIN_ADDR) && (addr <= EEPROM_MAX_ADDR));
}

void loop() {
  esp.process();

 if((wifiConnected) && (setupmode == false)) {
    char topic[40];
    now = millis();
    int switchval = analogRead(0);

    if (now >= nextPub) {
      nextPub = reportInterval + now;
      int chk = DHT.read22(DHT_PIN);
      float humid = DHT.humidity;
      float tempe = DHT.temperature;
      char chHumid[10];
      char chTempe[10];
      dtostrf(humid,1,2,chHumid);
      dtostrf(tempe,1,2,chTempe);
      strcat(strcat(strcpy(topic, "/"), eepromMqttClientName), "/humi");
      mqtt.publish(topic,chHumid);
      strcat(strcat(strcpy(topic, "/"), eepromMqttClientName), "/temp");
      mqtt.publish(topic,chTempe);
    }
    if (now >= nextSwitch) {
      nextSwitch = switchInterval + now;
      strcat(strcat(strcpy(topic, "/"), eepromMqttClientName), "/" MQTTTOPIC0);
      if ((switchval>500) && (switchstate == false)) {
        mqtt.publish(topic,"s11");
        switchstate = !switchstate;
      }
      if ((switchval<500) && (switchstate == true)) {
        mqtt.publish(topic,"s10");
        switchstate = !switchstate;
      }
    }
    if (now >= nextPulse) {
      nextPulse = pulseInterval +  now;
      if ((rel1_pulse == true) && (digitalRead(REL1_PIN) == LOW)) digitalWrite(REL1_PIN,HIGH);
      else if ((rel1_pulse == true) && (digitalRead(REL1_PIN) == HIGH)) {
        rel1_pulse = false;
        digitalWrite(REL1_PIN,LOW);
      }
    }
  }

  if((wifiConnected) && (setupmode == true)) {
    now = millis();
    if (now >= nextPub) {
      char tester[80];
      // char topic[40];
      nextPub = reportInterval + now;
      // strcat(strcat(strcpy(topic, "/"), SETUPMQTTCLIENT "/config");
      eeprom_read_string(100, tester, 20);
      mqtt.publish("/" SETUPMQTTCLIENT "/echo",tester);
      eeprom_read_string(228, tester, 20);
      mqtt.publish("/" SETUPMQTTCLIENT "/echo",tester);
      eeprom_read_string(356, tester, 20);
      mqtt.publish("/" SETUPMQTTCLIENT "/echo",tester);
      eeprom_read_string(484, tester, 20);
      mqtt.publish("/" SETUPMQTTCLIENT "/echo",tester);
    }
  }
}
