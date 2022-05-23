// Inlcude header files
// 引入头文件
#include <Arduino.h> // This line is optional if you are using Arduino IDE. 如果你使用Arduino IDE这行可以删掉。
#include "mywebsocket/mywebsocket.h"
#include <WiFi.h>

// Declare and initialize instance
// 声明和实例化
myWebSocket::CombinedServer server;

// Define IP and mask for AP mode
// 定义AP模式IP和子网掩码
IPAddress APIP = IPAddress(192, 168, 8, 1);
IPAddress subnet = IPAddress(255, 255, 255, 0);

void setup()
{
    // Initialize serial port(for debug)
    // 初始化串口(测试使用)
    Serial.begin(115200);
    
    
    // Uncomment following part and input your SSID and password if you want to use in STA mode
    // 如果你在STA模式使用，取消下面这部分注释，填写你的SSID和密码
    
    // WiFi.begin("yourSSID", "Your password");
    // while (WiFi.status() != WL_CONNECTED)
    // {
    //   yield();
    //   delay(100);
    //   yield();
    // }
    

    // Call this to enable ESP32 hardware acceleration
    // 启用ESP32的硬件加速(SHA)
    mycrypto::SHA::initialize();
    
    
    // For AP mode
    // Comment the following part if you'd like to use STA mode
    // AP模式
    // 注释掉下面这部分代码如果你使用STA模式
    WiFi.softAP("ESP32_WebSocketServer");
    delay(300);
    WiFi.softAPConfig(APIP, APIP, subnet);
    
    
    // Set websocket connection callback(use Lambda function here, you could use normal function if you want to)
    // 设置websocket客户端的回调函数(此处使用Lambda函数，你也可以使用普通函数)
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

    // Set http callback(for debug)
    // 设置默认的HTTP回调函数(测试使用)
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
    
    // Start server
    // 启动服务器
    server.begin(80);
}

void loop()
{
    // Loop server to accept websocket client and handle http resquest
    // 循环服务器来接受websocket客户端和处理http请求
    server.loop();
}
