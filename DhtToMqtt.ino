#include <espduino.h>
#include <mqtt.h>
#include <dht.h>

#define MYSSID "tim"
#define MYPASS "PASSWORD"
#define BROKERIP "192.168.1.121"
#define MQTTCLIENT "Cheapspark"
#define MQTTTOPIC0 "/cheapspark/humi"
#define MQTTTOPIC1 "/cheapspark/temp"
#define DHT_PIN 5


ESP esp(&Serial, 4);
MQTT mqtt(&esp);
boolean wifiConnected = false;
int reportInterval =  15000;
unsigned long now = 0;
unsigned long nextPub = reportInterval;
int ledpin = 13;
volatile int ledstate = LOW;
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
  mqtt.publish(MQTTTOPIC0, "data0");
  mqtt.publish(MQTTTOPIC1, "data1");
}
void mqttDisconnected(void* response)
{

}
void mqttData(void* response)
{
  RESPONSE res(response);

//db  debugPort.print("Received: topic=");
  String topic = res.popString();
//db  debugPort.println(topic);

//db  debugPort.print("data=");
  String data = res.popString();
//db  debugPort.println(data);

}
void mqttPublished(void* response)
{
//runs when publish is a success
}
void setup() {
  delay(5000);
  Serial.begin(19200);
  esp.enable();
  delay(500);
  esp.reset();
  delay(500);
  while(!esp.ready());

//setup mqtt client");
  if(!mqtt.begin(MQTTCLIENT, "", "", 120, 1)) {
    while(1);
  }

//setup mqtt lwt
  mqtt.lwt("/lwt", "offline", 0, 0);
  
  /*setup wifi*/
  esp.wifiCb.attach(&wifiCb);
  esp.wifiConnect(MYSSID,MYPASS);

}

void loop() {
  esp.process();
  digitalWrite(ledpin, ledstate);
  if(wifiConnected) {
    now = millis();
    if (now >= nextPub) {
      
      nextPub = reportInterval + now;
      
      int chk = DHT.read22(DHT_PIN);
      float humid = DHT.humidity;
      float tempe = DHT.temperature;
      
      char chHumid[10];
      char chTempe[10];
      
      dtostrf(humid,1,2,chHumid);
      dtostrf(tempe,1,2,chTempe);

      mqtt.publish(MQTTTOPIC0,chHumid);
      mqtt.publish(MQTTTOPIC1,chTempe);
  
      ledstate = !ledstate;
    }
    
  }
}
