# Arduino ESP32 Websocket
This lib work with Espressif offical framework esp32-arduino, version 1.0.6.
Only had tested on esp32 dev module. I'm not sure if support ESP8266.

Before I made this lib, I used another two lib on github, they both did very good job.
But connecting speed of the first one it will take 1700 ~ 1900 ms, it should be 90 ~ 110 ms.
Another one has memory leak when act a server.

This lib doesn't support wss://. Cause I think use AES to encrypted data before send it is much better(of course you need both user have same key), that depends on your design.

# Characteristic of this this project:

### High speed
Connect and data transfer.

### Easy to use.
You could start from the examples.

### No memory leak
Client and server had been fully tested for hours, send and receive large data(text and binary) periodicly one second.

### Easily handle http request and websocket request when act a server.
One line code to start a server to handle http and websocket request. 

### Stable
Easily transfer 64KB binary data in AP and STA mode with very low time time consumption and zero error.
