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
Switch mqttsw2 "Relay Pulse" (all) {mqtt=">[mqtt_broker:/cheapspark1/commands:command:ON:r1p]"}
Number Temp_DHT22 "Temperature DHT22 [%.1f °C]" (grTemperature) {mqtt="<[mqtt_broker:/cheapspark1/temp:state:default]"}
Number Humidity_DHT22 "Humidity DHT22 [%.1f %%]" (grHumidity) {mqtt="<[mqtt_broker:/cheapspark1/humi:state:default]"}
```
## How to use it
The wifi settings and broker ip ,used in normal operation mode are held in the EEPROM on the arduino, so it's important to get that info in there first.  

To write the settings to the EEPROM bridge analog0(A0) to GND and power the arduino.
At this point the ESP module will connect to the setup wifi (SETUPSSID in the code) "CSSetupWifi" with password (SETUPSSIDPW in the code) "cheapspark". The setup broker can also be defined in the code  

Send a message on /setup containing JSON params for all configs :  
{"SSID":"yourssid","password":"YourPasswd","broker ip":"192.168.1.125","client name":"Test"}  

Once the settings are written to eeprom a led will light up on the arduino.  
Remove the bridge between A0 and GND and reboot. The ESP will now connect to the SSID and broker defined in EEPROM

## Setup

Add libraries to arduino : 

1. [Tuan's espduino lib](https://github.com/tuanpmt/espduino)
2. [Rob Tillaart's DHT lib](https://github.com/RobTillaart/Arduino)
3. [Benoît Blanchon's Json Lib](https://github.com/bblanchon/ArduinoJson)

