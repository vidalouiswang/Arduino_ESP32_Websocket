#include <Arduino.h>
#include "mywebsocket/mywebsocket.h"
#include <WiFi.h>

myWebSocket::WebSocketClients clients;

void setup()
{

    // call this to enable ESP32 hardware acceleration
    mycrypto::SHA::initialize();

    Serial.begin(115200);

    Serial.println("Connecting to WiFi...");

    WiFi.begin("SSID", "password");

    while (WiFi.status() != WL_CONNECTED)
    {
        yield();
        delay(100);
        yield();
    }

    Serial.println("WiFi connected.");

    clients.setCallBack(
        [](myWebSocket::WebSocketClient *client, myWebSocket::WebSocketEvents type, uint8_t *payload, uint64_t length)
        {
            if (type == myWebSocket::WS_CONNECTED)
            {
                Serial.println("Websocket connected.");
            }
            else if (type == myWebSocket::WS_DISCONNECTED)
            {
                Serial.println("Websocket disconnected.");
            }
            else if (type == myWebSocket::TYPE_TEXT)
            {
                Serial.println("Got text: " + String((char *)payload));
                client->send("Hello world!");
            }
            else if (type == myWebSocket::TYPE_BIN)
            {
                Serial.println("Got binary, length:" + String((long)length));
                /*
                    if you need to send large data
                    but current free heap is not enough
                    use bellow:

                    delete payload;
                    client->setRecvBufferDeleted();

                    that will clear received buffer immediately
                    actually, this process could use transfer **payload to instead
                    but that may confused new hand
                    so class use a bool and a function to did this

                    note:
                    no recommended to use this method, unless it's necessary
                    otherwise you may forget to delete buffer but
                    client.setRecvBufferDeleted() was called
                    that will cause memory leak
                */
            }
            else
            {
                // more events see header file
                //...
            }
        });

    Serial.println("Connecting to websocket server...");

    // the pointer is optional, your call
    // connect will give back the pointer
    // if queue full connect will return false and the pointer is nullptr
    myWebSocket::WebSocketClient *client1 = nullptr;
    myWebSocket::WebSocketClient *client2 = nullptr;
    myWebSocket::WebSocketClient *client3 = nullptr;

    // connect 5 different websocket server
    clients.connect("abc.com", 80, "/", client1);
    client1->setAutoReconnect(true, 5000);

    clients.connect(String("def.com"), 8080, String("/"), client2);
    client2->setAutoReconnect(true, 10000);

    clients.connect("ws://hhh.com", false, client3); // default port = 80
    client3->setID(98);

    clients.connect("ws://hhh.com:9999");
    clients.connect("ws://xyz.com:9999/yourpath");
}

void loop()
{
    clients.loop();
}
