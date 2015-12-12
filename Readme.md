# Cheapspark

## What it does

Send MQTT messages with data from sensors (currently dht22)  
Send MQTT messages when a switch is closed (currently pulling A0 to GND is a switch)  
Receive MQTT messages containing commands for relays  

## Why you may need it

My primary use is as a hardware interface for openhab (home automation)  
For this purpose add a line to your items config looking something like this:

```
Switch mqttsw4 "CSP" (all) {mqtt=">[mqtt_broker:/cheapspark1/commands:command:ON:r11],>[mqtt_broker:/cheapspark1/commands:command:OFF:r10],<[mqtt_broker:/cheapspark1/commands:command:OFF:.*s10.*],<[mqtt_broker:/cheapspark1/commands:command:ON:.*s11.*"]"}
```

## Setup

Add libraries to arduino : 

1. [Tuan's espduino lib](https://github.com/tuanpmt/espduino)
2. [Rob Tillaart's DHT lib](https://github.com/RobTillaart/Arduino)
