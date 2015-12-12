# Cheapspark

## What it does

Send MQTT messages with data from sensors (currently dht22)  
Send MQTT messages when a switch is closed (currently pulling A0 to GND is a switch)  
Receive MQTT messages containing commands for relays  

## Why you may need it

My primary use is as a hardware interface for openhab (home automation)  
For this purpose add some lines to your items config looking something like this:

```
Switch mqttsw1 "Toggle Relay with button" (all) {mqtt=">[mqtt_broker:/cheapspark1/commands:command:ON:r11],>[mqtt_broker:/cheapspark1/commands:command:OFF:r10],<[mqtt_broker:/cheapspark1/commands:command:OFF:.*s10.*],<[mqtt_broker:/cheapspark1/commands:command:ON:.*s11.*"]"}
Switch mqttsw2 "Relay Pulse" (all) {mqtt=">[mybroker:/cheapspark0/relays:command:ON:r1p]"}
Number Temp_DHT22 "Temperature DHT22 [%.1f °C]" (grTemperature) {mqtt="<[mybroker:/cheapspark1//temp:state:default]"}
Number Humidity_DHT22 "Humidity DHT22 [%.1f %%]" (grHumidity) {mqtt="<[mybroker:/cheapspark1/humi:state:default]"}
```

## Setup

Add libraries to arduino : 

1. [Tuan's espduino lib](https://github.com/tuanpmt/espduino)
2. [Rob Tillaart's DHT lib](https://github.com/RobTillaart/Arduino)
