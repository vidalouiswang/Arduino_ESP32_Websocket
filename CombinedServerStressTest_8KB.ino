#include <Arduino.h>
#include "mywebsocket/mywebsocket.h"
#include <WiFi.h>

myWebSocket::CombinedServer server;

IPAddress APIP = IPAddress(192, 168, 8, 1);
IPAddress subnet = IPAddress(255, 255, 255, 0);

String globalHash = "";

void sendData(myWebSocket::WebSocketClient *client)
{
    uint16_t length = random(1, 8192);
    uint8_t binaryOrText = random(10);
    uint8_t t = 0;
    uint8_t *data = new uint8_t[length];
    if (binaryOrText > 5)
    {
        data[length - 1] = 0;
        length -= 1;
        // send text
        for (int i = 0; i < length; i++)
        {
            while (t < 32)
            {
                t = random(32, 127);
            }
            data[i] = t;
            t = 0;
        }
        String *a = new String((char *)data);
        delete data;
        globalHash = mycrypto::SHA::sha256(a);
        client->send(a);
        delete a;
    }
    else
    {
        // send binary
        for (int i = 0; i < length; i++)
        {
            data[i] = random(0, 255);
        }
        globalHash = mycrypto::SHA::sha256(data, length);
        client->send(data, length);
        delete data;
    }
}

void setup()
{
    Serial.begin(115200);
    // WiFi.begin("yourSSID", "Your password");
    // while (WiFi.status() != WL_CONNECTED)
    // {
    //   yield();
    //   delay(100);
    //   yield();
    // }

    // call this to enable ESP32 hardware acceleration
    mycrypto::SHA::initialize();

    WiFi.softAP("ESP32_WebSocketServer");
    delay(300);
    WiFi.softAPConfig(APIP, APIP, subnet);
    server.setCallback(
        [](myWebSocket::WebSocketClient *client, myWebSocket::WebSocketEvents type, uint8_t *payload, uint64_t length)
        {
            if (length)
            {
                if (type == myWebSocket::TYPE_TEXT)
                {
                    if (globalHash == "")
                    {
                        Serial.println("Got text");
                        String *a = new String((char *)payload);
                        delete payload;
                        client->setRecvBufferDeleted();
                        String hash = mycrypto::SHA::sha256(a);
                        delete a;
                        client->send(hash);
                        sendData(client);
                    }
                    else
                    {
                        String remoteHash = String((char *)payload);
                        if (remoteHash == globalHash)
                        {
                            Serial.println("OK");
                        }
                        else
                        {
                            Serial.println("failed");
                        }
                        globalHash = "";
                    }
                }
                else if (type == myWebSocket::TYPE_BIN)
                {
                    Serial.println("Got binary");
                    String hash = mycrypto::SHA::sha256(payload, length);
                    client->send(hash);
                    sendData(client);
                }
            }
        });

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
    <input value="Stop timer" type="button" onclick="stopSend = 1;" />
    <div id='main'></div>
    <script>
        (function() {
            let e, r, t, l = (e, r) => r >>> e | r << 32 - e,
                o = (e, r, t) => e & r ^ ~e & t,
                n = (e, r, t) => e & r ^ e & t ^ r & t,
                f = e => l(2, e) ^ l(13, e) ^ l(22, e),
                a = e => l(6, e) ^ l(11, e) ^ l(25, e),
                w = e => l(7, e) ^ l(18, e) ^ e >>> 3,
                A = e => l(17, e) ^ l(19, e) ^ e >>> 10,
                y = (e, r) => e[15 & r] += A(e[r + 14 & 15]) + e[r + 9 & 15] + w(e[r + 1 & 15]),
                c = new Array(1116352408, 1899447441, 3049323471, 3921009573, 961987163, 1508970993, 2453635748, 2870763221, 3624381080, 310598401, 607225278, 1426881987, 1925078388, 2162078206, 2614888103, 3248222580, 3835390401, 4022224774, 264347078, 604807628, 770255983, 1249150122, 1555081692, 1996064986, 2554220882, 2821834349, 2952996808, 3210313671, 3336571891, 3584528711, 113926993, 338241895, 666307205, 773529912, 1294757372, 1396182291, 1695183700, 1986661051, 2177026350, 2456956037, 2730485921, 2820302411, 3259730800, 3345764771, 3516065817, 3600352804, 4094571909, 275423344, 430227734, 506948616, 659060556, 883997877, 958139571, 1322822218, 1537002063, 1747873779, 1955562222, 2024104815, 2227730452, 2361852424, 2428436474, 2756734187, 3204031479, 3329325298),
                d = "0123456789abcdef",
                u = (e, r) => {
                    let t = (65535 & e) + (65535 & r),
                        l = (e >> 16) + (r >> 16) + (t >> 16);
                    return l << 16 | 65535 & t
                },
                h = () => {
                    e = new Array(8);
                    r = new Array(2);
                    t = new Array(64);
                    r[0] = r[1] = 0;
                    e[0] = 1779033703;
                    e[1] = 3144134277;
                    e[2] = 1013904242;
                    e[3] = 2773480762;
                    e[4] = 1359893119;
                    e[5] = 2600822924;
                    e[6] = 528734635;
                    e[7] = 1541459225
                },
                i = () => {
                    let r, l, w, A, d, h, i, g, s, b, p = new Array(16);
                    r = e[0];
                    l = e[1];
                    w = e[2];
                    A = e[3];
                    d = e[4];
                    h = e[5];
                    i = e[6];
                    g = e[7];
                    for (let e = 0; e < 16; e++) p[e] = t[3 + (e << 2)] | t[2 + (e << 2)] << 8 | t[1 + (e << 2)] << 16 | t[e << 2] << 24;
                    for (let e = 0; e < 64; e++) {
                        s = g + a(d) + o(d, h, i) + c[e];
                        s += e < 16 ? p[e] : y(p, e);
                        b = f(r) + n(r, l, w);
                        g = i;
                        i = h;
                        h = d;
                        d = u(A, s);
                        A = w;
                        w = l;
                        l = r;
                        r = u(s, b)
                    }
                    e[0] += r;
                    e[1] += l;
                    e[2] += w;
                    e[3] += A;
                    e[4] += d;
                    e[5] += h;
                    e[6] += i;
                    e[7] += g
                },
                g = (e, l) => {
                    let o, n, f = 0;
                    n = r[0] >> 3 & 63;
                    let a = 63 & l;
                    (r[0] += l << 3) < l << 3 && r[1]++;
                    r[1] += l >> 29;
                    for (o = 0; o + 63 < l; o += 64) {
                        for (let r = n; r < 64; r++) t[r] = e.charCodeAt(f++);
                        i();
                        n = 0
                    }
                    for (let r = 0; r < a; r++) t[r] = e.charCodeAt(f++)
                },
                s = () => {
                    let e = r[0] >> 3 & 63;
                    t[e++] = 128;
                    if (e <= 56)
                        for (let r = e; r < 56; r++) t[r] = 0;
                    else {
                        for (let r = e; r < 64; r++) t[r] = 0;
                        i();
                        for (let e = 0; e < 56; e++) t[e] = 0
                    }
                    t[56] = r[1] >>> 24 & 255;
                    t[57] = r[1] >>> 16 & 255;
                    t[58] = r[1] >>> 8 & 255;
                    t[59] = 255 & r[1];
                    t[60] = r[0] >>> 24 & 255;
                    t[61] = r[0] >>> 16 & 255;
                    t[62] = r[0] >>> 8 & 255;
                    t[63] = 255 & r[0];
                    i()
                },
                b = () => {
                    let r = 0,
                        t = new Array(32);
                    for (let l = 0; l < 8; l++) {
                        t[r++] = e[l] >>> 24 & 255;
                        t[r++] = e[l] >>> 16 & 255;
                        t[r++] = e[l] >>> 8 & 255;
                        t[r++] = 255 & e[l]
                    }
                    return t
                },
                p = () => {
                    let r = new String;
                    for (let t = 0; t < 8; t++)
                        for (let l = 28; l >= 0; l -= 4) r += d.charAt(e[t] >>> l & 15);
                    return r
                },
                C = (e, r, t) => {
                    h();
                    g(e, e.length);
                    s();
                    return r ? b() : p()
                };
            "object" == typeof window ? window.getHash = C : module.exports = C
        })();
    </script>
    <script>
        (e => {
            let w = window;
            let container = document.getElementById('main');
            w.webSocket = 0;
            w.lastType = 0;
            w.stopSend = 0;
            w.globalHash = "";
            let create = txt => {
                let p = document.createElement("p");
                p.innerText = txt;
                return p;
            };
            let randomChar = e => {
                let t = 0;
                while (t < 32) {
                    t = parseInt(Math.random() * 127);
                }
                return String.fromCharCode(t);
            };
            let connectWebSocket = url => {
                url = url || 'ws://' + w.location.host;
                let connectingTime = new Date().getTime();
                w.webSocket = new WebSocket(url);
                w.webSocket.binaryType = "arraybuffer";
                w.webSocket.onopen = function() {
                    this.binaryType = "arraybuffer";
                    container.appendChild(create("Connected, time: " + (new Date().getTime() - connectingTime) + "ms"));
                    this.sendNext = e => {
                        let binaryOrText = Math.random();
                        let length = parseInt(Math.random() * 8192);
                        while (!length) {
                            length = parseInt(Math.random() * 8192);
                        }
                        if (binaryOrText < 0.5) {
                            //binary
                            w.lastType = 0;

                            let u8a = new Uint8Array(length);
                            let h = "";
                            for (let i = 0; i < length; i++) {
                                u8a[i] = parseInt(Math.random() * 255);
                                h += String.fromCharCode(u8a[i]);
                            }
                            globalHash = getHash(h);
                            this.send(u8a.buffer);
                        } else {
                            //text
                            w.lastType = 1;
                            let h = "";
                            let t = 0;
                            for (let i = 0; i < length; i++) {
                                h += randomChar();
                            }
                            globalHash = getHash(h);
                            this.send(h);
                        }
                    };

                    setTimeout(() => {
                        this.sendNext();
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
                    let p;
                    if (globalHash != "") {
                        let hash = msg.data.toString();
                        if (globalHash == hash) {
                            p = create("OK, " + "type: " + (lastType ? "text" : "binary") + ", local: " + globalHash + ", remote: " + hash + ", " + new Date().getTime());
                        } else {
                            p = create("failed, " + "type: " + (lastType ? "text" : "binary") + ", local: " + globalHash + ", remote: " + hash + ", " + new Date().getTime());
                        }
                        globalHash = "";
                    } else {
                        let h = "";
                        if (Object.prototype.toString.call(msg.data).toLowerCase().indexOf("arraybuffer") >= 0) {
                            let dv = new DataView(msg.data);
                            p = create("Got binary");
                            for (let i = 0; i < dv.byteLength; i++) {
                                h += String.fromCharCode(dv.getUint8(i));
                            }
                        } else {
                            p = create("Got text");
                            h = msg.data.toString();
                        }
                        let hash = getHash(h);
                        this.send(hash);

                        if (!w.stopSend) {
                            this.sendNext();
                        }
                    }


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
    server.begin(80);

    Serial.println("Connect to AP, and open \"192.168.8.1\"");
}

void loop()
{
    server.loop();
}