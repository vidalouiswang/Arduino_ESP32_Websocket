#include <Arduino.h>
#include "mywebsocket/mywebsocket.h"
#include <WiFi.h>

myWebSocket::WebSocketClient client;

u32_t t = 0;

void setup()
{
    Serial.begin(115200);
    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED)
    {
        yield();
        delay(100);
        yield();
    }

    Serial.println("WiFi connected.");

    client.setCallBack(
        [](myWebSocket::WebSocketEvents type, uint8_t *payload, uint64_t length)
        {
            if (type == myWebSocket::WS_CONNECTED)
            {

                Serial.println("Websocket connected. Time: " + String(millis() - t));
                Serial.println("long msg will be send to server in 1000 ms.");
                yield();
                delay(1000);
                yield();
                String *b = new String("");
                for (int i = 0; i < 8192; i++)
                {
                    yield();
                    b->concat((char)random(96, 127));
                    yield();
                }
                client.send(b);
                Serial.println(*b);
                delete b;
            }
            else if (type == myWebSocket::WS_DISCONNECTED)
            {
                Serial.println("Websocket disconnected.");
            }
            else if (type == myWebSocket::TYPE_TEXT)
            {
                Serial.println("Got text: " + String((char *)payload));
            }
            else if (type == myWebSocket::TYPE_BIN)
            {
                Serial.println("Got binary, length:" + String((long)length));
                 /*
                    if you'd like to malloc or new a heap space to send data
                    but current heap isn't big enough for that
                    you could do this:

                    delete payload;
                    client.setRecvBufferDeleted();

                    that will clear received buffer immediately

                    actually, this process could use ** to instead
                    but that may confused new hand
                    so class use a bool and a function to did this

                    note:
                    no recommended to use this method, unless it's necessary
                    otherwise you may forget to delete buffer but 
                    client.setRecvBufferDeleted() was call
                    that will cause memory leak
                */
            }
            else
            {
                //...
            }
        });

    Serial.println("Connecting to websocket server...");

    //record connect timestamp
    t = millis();

    //connect
    client.connect("abc.com", 80, "/");

    //you could set interval after connection lost
    //auto connect is true by default
    //timeout is 5000ms
    client.setAutoReconnect(true, 5000);

    // you have different ways to connect to server
    // client.connect("abc.com", 8080, "/");
    // client.connect("ws://abc.com"); == port = 80, path = "/"
    // client.connect("ws://abc.com:8080"); == port = 8080, path = "/"
    // client.connect("ws://abc.com:8080/yourpath"); == port = 8080, path = "/yourpath"
    // client.connect("ws://abc.com/yourpath"); == port = 80, path = "/yourpath"
}

void loop()
{
    client.loop();
}
