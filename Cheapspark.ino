#include <ELClient.h>
#include <ELClientMqtt.h>

#include <dht.h>

#define MQTTTOPIC0 "incoming"
#define MQTTTOPIC1 "outgoing"
#define DHT_PIN 8
#define REL1_PIN 9
#define REL2_PIN 10
#define REL3_PIN 11
#define REL4_PIN 12

ELClient esp(&Serial, &Serial); // Initialize a connection to esp-link using the normal hardware serial port both for SLIP and for debug messages.
ELClientMqtt mqtt(&esp);

char HostName[20];
boolean wifiConnected = false;
int reportInterval =  15000;
int switchInterval = 100;
int pulseInterval = 50;
unsigned long now = 0;
unsigned long nextPub = 30000;
unsigned long nextSwitch = switchInterval;
unsigned long nextPulse = pulseInterval;
int ledpin = 13;
boolean switchstate0 = false;
boolean switchstate1 = false;
boolean switchstate2 = false;
boolean switchstate3 = false;
boolean switchstate4 = false;
boolean rel1_pulse = false;
boolean rel2_pulse = false;
boolean rel3_pulse = false;
boolean rel4_pulse = false;
dht DHT;

// Callback made from esp-link to notify of wifi status changes
// Here we just print something out for grins
void wifiCb(void* response) {
  ELClientResponse *res = (ELClientResponse*)response;
  if (res->argc() == 1) {
    uint8_t status;
    res->popArg(&status, 1);

    if(status == STATION_GOT_IP) {
      Serial.println("WIFI CONNECTED");
    } else {
      Serial.print("WIFI NOT READY: ");
      Serial.println(status);
    }
  }
}

// Callback when MQTT is connected
void mqttConnected(void* response) {
  Serial.println("MQTT connected!");

  char topic[40];
  strcat(strcat(strcpy(topic, "/"), HostName), "/" MQTTTOPIC0);
  mqtt.subscribe(topic,1);

  wifiConnected = true;
}

// Callback when MQTT is disconnected
void mqttDisconnected(void* response) {
  Serial.println("MQTT disconnected");
  wifiConnected = false;
}

void mqttData(void* response){
  ELClientResponse *res = (ELClientResponse *)response;

  String topic = res->popString();
  String data = res->popString();
  char buffer[4];

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

void mqttPublished(void* response) {
  Serial.println("MQTT published");
}



void setup(){
  delay(2000);
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
  pinMode(A4,INPUT);
  digitalWrite(A0,HIGH);
  digitalWrite(A1,HIGH);
  digitalWrite(A2,HIGH);
  digitalWrite(A3,HIGH);
  digitalWrite(A4,HIGH);

  pinMode(REL1_PIN,OUTPUT);
  pinMode(REL2_PIN,OUTPUT);
  pinMode(REL3_PIN,OUTPUT);
  pinMode(REL4_PIN,OUTPUT);

  //setup EL-Link
    Serial.begin(115200);
  Serial.println("EL-Client starting!");

  // Sync-up with esp-link, this is required at the start of any sketch and initializes the
  // callbacks to the wifi status change callback. The callback gets called with the initial
  // status right after Sync() below completes.
  esp.wifiCb.attach(wifiCb); // wifi status change callback, optional (delete if not desired)
  bool ok;
  do {
    ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
    if (!ok) Serial.println("EL-Client sync failed!");
  } while(!ok);
  Serial.println("EL-Client synced!");

  // Look hostname up
  lookupHostname(HostName);
  Serial.print("Setup - Hostname: ");
  Serial.println(HostName);

  // Set-up callbacks for events and initialize with es-link.
  mqtt.connectedCb.attach(mqttConnected);
  mqtt.disconnectedCb.attach(mqttDisconnected);
  mqtt.publishedCb.attach(mqttPublished);
  mqtt.dataCb.attach(mqttData);
  mqtt.setup();


  //setup mqtt lwt
  char ch_lwt[40];
  strcat(strcpy(ch_lwt,HostName), " offline");
  mqtt.lwt("/lwt", ch_lwt, 0, 0);

  Serial.println("EL-MQTT ready");
}

void loop() {
  esp.Process();

 if(wifiConnected) {
    char topic[40];
    now = millis();
    int switchval0 = analogRead(0);
    int switchval1 = analogRead(1);
    int switchval2 = analogRead(2);
    int switchval3 = analogRead(3);
    int switchval4 = analogRead(4);


    if (now >= nextPub) {
      nextPub = reportInterval + now;
      int chk = DHT.read22(DHT_PIN);
      float humid = DHT.humidity;
      float tempe = DHT.temperature;
      char chHumid[10];
      char chTempe[10];
      dtostrf(humid,1,2,chHumid);
      dtostrf(tempe,1,2,chTempe);
      strcat(strcat(strcpy(topic, "/"), HostName), "/humi");
      mqtt.publish(topic,chHumid);
      strcat(strcat(strcpy(topic, "/"), HostName), "/temp");
      mqtt.publish(topic,chTempe);
    }

    //reading switch
    strcat(strcat(strcpy(topic, "/"), HostName), "/" MQTTTOPIC1);
    if ((switchval0>500) && (switchstate0 == false)) {
      mqtt.publish(topic,"s00",1,0);
      switchstate0 = !switchstate0;
    }
    if ((switchval0<500) && (switchstate0 == true)) {
      mqtt.publish(topic,"s01",1,0);
      switchstate0 = !switchstate0;
    }

    if ((switchval1>500) && (switchstate1 == false)) {
      mqtt.publish(topic,"s10",1,0);
      switchstate1 = !switchstate1;
    }
    if ((switchval1<500) && (switchstate1 == true)) {
      mqtt.publish(topic,"s11",1,0);
      switchstate1 = !switchstate1;
    }

    if ((switchval2>500) && (switchstate2 == false)) {
      mqtt.publish(topic,"s30",1,0);
      switchstate2 = !switchstate2;
    }
    if ((switchval2<500) && (switchstate2 == true)) {
      mqtt.publish(topic,"s31",1,0);
      switchstate2 = !switchstate2;
    }

    if ((switchval3>500) && (switchstate3 == false)) {
      mqtt.publish(topic,"s20",1,0);
      switchstate3 = !switchstate3;
    }
    if ((switchval3<500) && (switchstate3 == true)) {
      mqtt.publish(topic,"s21",1,0);
      switchstate3 = !switchstate3;
    }

    if ((switchval4>500) && (switchstate4 == false)) {
      mqtt.publish(topic,"PIR1",1,0); //inverted pir sensors are NC
      switchstate4 = !switchstate4;
    }
    if ((switchval4<500) && (switchstate4 == true)) {
      mqtt.publish(topic,"PIR0",1,0);
      switchstate4 = !switchstate4;
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
}

// Requests hostname and puts it result in theHostname
void lookupHostname (char * aHostname)
{
  esp.GetHostname();
  ELClientPacket *packet;

  if ((packet=esp.WaitReturn()) != NULL) {
    // Create Response from packet
    ELClientResponse resp = (ELClientResponse)packet;
    
    resp.popArg(aHostname,packet->value);
  }
}
