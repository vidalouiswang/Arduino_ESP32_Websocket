#include <Arduino.h>
#include "mywebsocket/mywebsocket.h"
#include <WiFi.h>

myWebSocket::CombinedServer server;

IPAddress APIP = IPAddress(192, 168, 8, 1);
IPAddress subnet = IPAddress(255, 255, 255, 0);

void setup()
{
    Serial.begin(115200);
    // STA mode
    // WiFi.begin("yourSSID", "Your password");
    // while (WiFi.status() != WL_CONNECTED)
    // {
    //   yield();
    //   delay(100);
    //   yield();
    // }

    //call this to enable ESP32 hardware acceleration
    mycrypto::SHA::initialize();
    
    //AP mode
    WiFi.softAP("ESP32_WebSocketServer");
    delay(300);
    WiFi.softAPConfig(APIP, APIP, subnet);
    
    //set websocket connection callback
    server.setCallback(
        [](myWebSocket::WebSocketClient *client, myWebSocket::WebSocketEvents type, uint8_t *payload, uint64_t length)
        {
            if (length)
            {
                if (type == myWebSocket::TYPE_TEXT)
                {
                    Serial.println("Got text data:");
                    Serial.println(String((char *)payload));
                    client->send(String("Hello from ESP32 WebSocket: ") + String(ESP.getFreeHeap()));
                }
                else if (type == myWebSocket::TYPE_BIN)
                {
                    Serial.println("Got binary data, length: " + String((long)length));
                    Serial.println("First byte: " + String(payload[0]));
                    Serial.println("Last byte: " + String(payload[length - 1]));
                }
            }
        });

    //set http callback
    server.on(
        "/",
        [](myWebSocket::ExtendedWiFiClient *client, myWebSocket::HttpMethod method, uint8_t *data, uint64_t len)
        {
            client->send(R"(<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Combined Server</title>
</head>
<body>
    <h1>Hello World!</h1>
    <input value="Stop timer" type="button" onclick="clearInterval(webSocket.t);" />
    <div id='main'></div>
    <script>
        (e => {
            let w = window;
            let container = document.getElementById('main');
            w.webSocket = 0;
            let create = txt => {
                let p = document.createElement("p");
                p.innerText = txt;
                return p;
            };
            let connectWebSocket = url => {
                url = url || 'ws://' + w.location.host;
                let connectingTime = new Date().getTime();
                w.webSocket = new WebSocket(url);
                w.webSocket.binaryType = "arraybuffer";
                w.webSocket.onopen = function() {
                    container.appendChild(create("Connected, time: " + (new Date().getTime() - connectingTime) + "ms"));
                    setTimeout(() => {
                        let arr = [];
                        for (let i = 0; i < 65536; i++) {
                            arr.push(parseInt(Math.random() * 0xff));
                        }
                        container.appendChild(create("First byte: " + arr[0]));
                        container.appendChild(create("Last byte: " + arr[arr.length - 1]));
                        arr = new Uint8Array(arr);
                        this.send(arr.buffer);
                        this.binaryType = "nodebuffer";
                        this.t = setInterval(() => {
                            this.send("Hello from client: " + new Date().getTime());
                        }, 1e3);
                    }, 1e3);
                };
                w.webSocket.onerror = function() {
                    clearInterval(this.t);
                    console.log('websocket error');
                };
                w.webSocket.onclose = function() {
                    clearInterval(this.t);
                    console.log('websocket closed');
                };
                w.webSocket.onmessage = function(msg) {
                    msg.data = msg.data.toString();
                    let p = create(msg.data);
                    container.children.length ? container.insertBefore(p, container.children[0]) : container.appendChild(p);
                }
            };
            connectWebSocket();
        })();
    </script>
</body>
</html>)");
            client->close();
        });
    
    //start server
    server.begin(80);
}

void loop()
{
    //loop server
    server.loop();
}
