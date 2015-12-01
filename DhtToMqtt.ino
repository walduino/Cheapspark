#include <SoftwareSerial.h>
#include <espduino.h>
#include <mqtt.h>
#include <dht.h>

#define MYSSID "tim"
#define MYPASS "PASSWORD"
#define BROKERIP "192.168.1.121"
#define MQTTCLIENT "Cheapspark"
#define MQTTTOPIC0 "/cheapspark/humi"
#define MQTTTOPIC1 "/cheapspark/temp"
#define DHT44_PIN 12

SoftwareSerial debugPort(2, 3); // RX, TX
ESP esp(&Serial, &debugPort, 4);
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
    if(status == STATION_GOT_IP) {
      debugPort.println("WIFI CONNECTED");
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
  debugPort.println("Connected");
  mqtt.publish(MQTTTOPIC0, "data0");
  mqtt.publish(MQTTTOPIC1, "data1");
}
void mqttDisconnected(void* response)
{

}
void mqttData(void* response)
{
  RESPONSE res(response);

  debugPort.print("Received: topic=");
  String topic = res.popString();
  debugPort.println(topic);

  debugPort.print("data=");
  String data = res.popString();
  debugPort.println(data);

}
void mqttPublished(void* response)
{
  debugPort.println("Published: Succes");
}
void setup() {
  delay(5000);
  Serial.begin(19200);
  debugPort.begin(19200);
  esp.enable();
  delay(500);
  esp.reset();
  delay(500);
  while(!esp.ready());

  debugPort.println("ARDUINO: setup mqtt client");
  if(!mqtt.begin(MQTTCLIENT, "", "", 120, 1)) {
    debugPort.println("ARDUINO: fail to setup mqtt");
    while(1);
  }

  debugPort.println("ARDUINO: setup mqtt lwt");
  mqtt.lwt("/lwt", "offline", 0, 0);
  
  /*setup wifi*/
  debugPort.println("ARDUINO: setup wifi");
  esp.wifiCb.attach(&wifiCb);

  esp.wifiConnect(MYSSID,MYPASS);

  debugPort.println("ARDUINO: system started");
}

void loop() {
  esp.process();
  
  digitalWrite(ledpin, ledstate);
  
  if(wifiConnected) {
    
    now = millis();
    
    if ((digitalRead(3) == HIGH) && now >= nextPub) { //alarm sensors are NC if using a switch make this LOW
      
      nextPub = reportInterval + now;
      
      int chk = DHT.read44(12);
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
