#include <Arduino.h>
#include "mywebsocket/mywebsocket.h"
#include <WiFi.h>

myWebSocket::WebSocketClient client;

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

                Serial.println("Websocket connected.");
                Serial.println("long msg will be send to server in 1000 second.");
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
            }
            else
            {
                //...
            }
        });

    Serial.println("Connecting to websocket server...");

    client.connect("ws://abc.com", 8080, "/");

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