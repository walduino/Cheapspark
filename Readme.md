# Cheapspark

## What it does

Send MQTT messages with data from sensors (currently dht22)  
Send MQTT messages when a switch is closed (currently pulling A0 to GND is a switch)  
Receive MQTT messages containing commands for relays  

## Setup

Add libraries to arduino : 

1. [Tuan's espduino lib](https://github.com/tuanpmt/espduino)
2. [Rob Tillaart's DHT lib](https://github.com/RobTillaart/Arduino)
