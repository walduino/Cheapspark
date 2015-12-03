#include <espduino.h>
#include <mqtt.h>
#include <dht.h>

#define MYSSID "tim"
#define MYPASS "PASSWORD"
#define BROKERIP "192.168.1.121"
#define MQTTCLIENT "cheapspark1"
#define MQTTSTOPIC0 "commands"
#define DHT_PIN 5
#define REL1_PIN 9
#define REL2_PIN 10
#define REL3_PIN 11
#define REL4_PIN 12


ESP esp(&Serial, 4);
MQTT mqtt(&esp);
boolean wifiConnected = false;
int reportInterval =  15000;
unsigned long now = 0;
unsigned long nextPub = reportInterval;
int ledpin = 13;
boolean ledstate = false;
boolean switchstate = false;
dht DHT;

void wifiCb(void* response)
{
  uint32_t status;
  RESPONSE res(response);

  if(res.getArgc() == 1) {
    res.popArgs((uint8_t*)&status, 4);
    if(status == STATION_GOT_IP) {        //WIFI CONNECTED
      mqtt.connect(BROKERIP, 1883, false);
      wifiConnected = true;
    } else {
      wifiConnected = false;
      mqtt.disconnect();
    }
    
  }
}

void mqttConnected(void* response)
{
  delay(500);
  mqtt.subscribe("/cheapspark1/commands");
  mqtt.publish("/fb", MQTTCLIENT " online");
}


void mqttDisconnected(void* response)
{

}


void mqttData(void* response)
{
  RESPONSE res(response);
  char buffer[4];
  String topic = res.popString();
  String data = res.popString();
//  ledstate = !ledstate;
//  digitalWrite(ledpin, ledstate);
//  if (topic = "/Cheapspark1/commands") {
    data.toCharArray(buffer,4);
    if (strcmp(buffer,"r11") == 0) digitalWrite(REL1_PIN,HIGH);
    if (strcmp(buffer,"r10") == 0) digitalWrite(REL1_PIN,LOW);

    if (strcmp(buffer,"r21") == 0) digitalWrite(REL2_PIN,HIGH);
    if (strcmp(buffer,"r20") == 0) digitalWrite(REL2_PIN,LOW);

    if (strcmp(buffer,"r31") == 0) digitalWrite(REL3_PIN,HIGH);
    if (strcmp(buffer,"r30") == 0) digitalWrite(REL3_PIN,LOW);

    if (strcmp(buffer,"r41") == 0) digitalWrite(REL4_PIN,HIGH);
    if (strcmp(buffer,"r40") == 0) digitalWrite(REL4_PIN,LOW);

}
void mqttPublished(void* response)
{
//runs when publish is a success
}
void setup() {
  switchstate = false;
  pinMode(A0,INPUT);
  digitalWrite(A0,HIGH);
  pinMode(REL1_PIN,OUTPUT);
  pinMode(REL2_PIN,OUTPUT);
  pinMode(REL3_PIN,OUTPUT);
  pinMode(REL4_PIN,OUTPUT);
  
  
  delay(5000);
  Serial.begin(19200);
  esp.enable();
  delay(500);
  esp.reset();
  delay(500);
  while(!esp.ready());

//setup mqtt client");
  if(!mqtt.begin(MQTTCLIENT, "", "", 30, 1)) {
    while(1);
  }

//setup mqtt lwt
  mqtt.lwt("/lwt", MQTTCLIENT " offline", 0, 0);
  
/*setup mqtt events */
  mqtt.connectedCb.attach(&mqttConnected);
  mqtt.disconnectedCb.attach(&mqttDisconnected);
  mqtt.publishedCb.attach(&mqttPublished);
  mqtt.dataCb.attach(&mqttData);  
  
  
  /*setup wifi*/
  esp.wifiCb.attach(&wifiCb);
  esp.wifiConnect(MYSSID,MYPASS);

}

void loop() {
  esp.process();

  if(wifiConnected) {
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

      mqtt.publish(("/" MQTTCLIENT "/humi"),chHumid);
      mqtt.publish(("/" MQTTCLIENT "/temp"),chTempe);
    }
    if ((switchval>500) && (switchstate == false)) {
      mqtt.publish(("/" MQTTCLIENT "/" MQTTSTOPIC0),"Switch1On");
      switchstate = !switchstate;
    }
    if ((switchval<500) && (switchstate == true)) {
      mqtt.publish(("/" MQTTCLIENT "/" MQTTSTOPIC0),"Switch1Off");
      switchstate = !switchstate;
    }
  }
}
