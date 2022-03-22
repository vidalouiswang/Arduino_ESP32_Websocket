# Arduino ESP32 Websocket

[中文](https://github.com/vidalouiswang/Arduino_ESP32_Websocket/blob/main/README_CN.md)

This lib work with Espressif offical framework esp32-arduino, version 1.0.6.
Only had tested on esp32 dev module. I'm not sure it support ESP8266 or not.
In my opinion, ESP32-WROOM-32(module) and ESP8266-12F(module) have almost same price but ESP32 is much powerful.

Before I made this lib, I had used another two libs you could find on github(most stars) for days, they both did very good job.

But connecting speed of the first one it will take 1700 ~ 1900 ms(handshake), it should be 90 ~ 110 ms(test at same time to same server).

Another one has memory leak when act a server(I try to fix it but couldn't find reason).

This lib doesn't support wss://. Because I think use AES(or other encryption) to encrypt data before send it is much better(of course you need both user have same key), that depends on your design.

# Characteristic of this this project

### High speed
Connecting and data transfering.

### Easy to use
You could start from the examples.

### No memory leak
Client and server had been fully tested for hours, send and receive large data(16 KB text and binary, random bytes every time, use sha256 to test data) periodicly one second, echo to each other(not included in examples, example only have normal light-weight test).

### Easily handle http request and websocket request when act a server
One line code to start a server to handle http and websocket request. 

### Stable
Easily transfer 64KB binary data in AP and STA mode with very low time time consumption and zero error(complex code environment, test more than 10000 times, use sha256 to test data).

Currently work with 1.0.6 of the [offical framework](https://github.com/espressif/arduino-esp32/releases/tag/1.0.6/).

### Note
When you test the client, every time after connection lost and reconnected again, you will find heap memory will reduce a little bit, that's not a memory leak, for deeper reason, you should search "lwip memory leak" in google.

For more config, look into "mywebsocket.h".

# License
(GNU General Public License v3.0 License)

Copyright 2022, Vida Wang  <github@vida.wang>


Children are the future of mankind, but there are still many children who are enduring hunger all over the world at this moment. If you are a good person and you like this lib, please donate to UNICEF.
https://unicef.org
