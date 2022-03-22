#include "mywebsocket.h"

namespace myWebSocket
{
    String generateServerKey(String clientKey)
    {
        unsigned char inputServerKey[clientKey.length()];
        for (int i = 0; i < clientKey.length(); i++)
            inputServerKey[i] = (unsigned char)clientKey.charAt(i);
        int l = 20;
        unsigned char output[l];

        esp_sha(SHA1, inputServerKey, clientKey.length(), output);
        clientKey = base64().encode(output, l);
        return clientKey;
    }

    String WebSocketClient::generateHanshake()
    {
        // copy header
        String wsHeader = String(myWebSocketHeader);

        // make host
        this->domain = String(this->host + ":" + String(this->port));

        // replace host and path
        wsHeader.replace("@HOST@", domain);
        wsHeader.replace("@PATH@", this->path);

        // generate key
        uint8_t key[16];
        for (uint8_t i = 0; i < 16; i++)
            key[i] = random(0xFF);

        // hash
        unsigned char output[20];
        esp_sha(SHA1, key, 16, output);
        this->clientKey = base64().encode(output, 16);

        wsHeader.replace("@KEY@", this->clientKey);
        return wsHeader;
    }

    bool WebSocketClient::connect(String url, bool withHeader)
    {
        if (url.startsWith("wss://"))
        {
            return false;
        }
        if (url.startsWith("ws://"))
        {
            url = url.substring(url.indexOf("ws://") + 5);
        }
        else
        {
            if (withHeader)
            {
                return false;
            }
        }

        int a = url.indexOf(":");
        int b = url.indexOf("/", (a < 0 ? 0 : a));

        String domain = "";
        uint16_t port = 80;
        String path = "/";

        if (a < 0)
        {
            if (b < 0)
            { // abc.com
                domain = url;
            }
            else
            { // abc.com/path
                domain = url.substring(0, b);
                path = url.substring(b);
            }
        }
        else
        {
            if (b < 0)
            { // abc.com:8080
                port = url.substring(a + 1).toInt();
            }
            else
            { // abc.com:8080/path
                port = url.substring(a + 1, b).toInt();
                path = url.substring(b);
            }
            domain = url.substring(0, a);
        }
        return this->connect(domain, port, path);
    }

    bool WebSocketClient::handShake()
    {
        this->client = new WiFiClient();
        String header = generateHanshake();
        String serverKey = this->clientKey + String("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

        serverKey = generateServerKey(serverKey);

        if (!this->client->connect(this->host.c_str(), this->port))
        {
            this->status = TCP_FAILED;
            this->fn(WS_DISCONNECTED, nullptr, 0);
            return false;
        }
        else
        {
            this->client->setNoDelay(1);
            this->status = TCP_CONNECTED;

            // while (!this->client->available())
            // {
            //     yield();
            // }

            uint64_t wroteLen = this->client->print(header);
            this->client->flush();

            if (wroteLen != header.length())
            {
                this->status = TCP_ERROR;
                this->fn(WS_DISCONNECTED, nullptr, 0);
                return false;
            }

            while (!this->client->available())
            {
                yield();
            }

            uint64_t len = this->client->read(this->buffer, MY_WEBSOCKET_BUFFER_LENGTH);
            if (len > 0)
            {

                this->buffer[len] = 0;
                String handShakeStr = String((char *)this->buffer);

                if (handShakeStr.indexOf("\r\n\r\n") >= 0)
                {
                    int keyStart = handShakeStr.indexOf("Sec-WebSocket-Accept: ") + 22;
                    int keyEnd = handShakeStr.indexOf("\r\n", keyStart);
                    String key = handShakeStr.substring(keyStart, keyEnd);
                    if (key == serverKey &&
                        handShakeStr.indexOf("Upgrade: websocket") >= 0 &&
                        handShakeStr.indexOf("Connection: Upgrade") >= 0 &&
                        handShakeStr.indexOf("HTTP/1.1 101") >= 0)
                    {
                        bzero(this->buffer, MY_WEBSOCKET_BUFFER_LENGTH);
                        this->status = WS_CONNECTED;
                        this->fn(WS_CONNECTED, nullptr, 0);
                        return true;
                    }
                    else
                    {
                        this->client->stop();
                        this->status = HANDSHAKE_UNKNOWN_ERROR;
                        this->fn(WS_DISCONNECTED, nullptr, 0);
                        return false;
                    }
                }
                else
                {
                    this->status = HANDSHAKE_UNKNOWN_ERROR;
                    return false;
                }
            }
            else
            {

                this->status = HANDSHAKE_UNKNOWN_ERROR;
                this->fn(WS_DISCONNECTED, nullptr, 0);
                return false;
            }
        }
    }

    uint64_t WebSocketClient::send(String *data)
    {
        WebSocketEvents type = TYPE_TEXT;
        return this->_send(type, (uint8_t *)data->c_str(), data->length());
    }

    uint64_t WebSocketClient::send(String data)
    {
        WebSocketEvents type = TYPE_TEXT;
        return this->_send(type, (uint8_t *)data.c_str(), data.length());
    }

    uint64_t WebSocketClient::send(const char *data)
    {
        WebSocketEvents type = TYPE_TEXT;
        return this->_send(type, (uint8_t *)data, strlen(data));
    }

    uint64_t WebSocketClient::send(uint8_t *data, uint64_t len)
    {
        WebSocketEvents type = TYPE_BIN;
        return this->_send(type, data, len);
    }

    uint64_t WebSocketClient::_send(WebSocketEvents type, uint8_t *data, uint64_t len)
    {
        uint8_t mask[4];
        for (int i = 0; i < 4; i++)
            mask[i] = random(0xff);

        uint8_t header[10];
        bzero(header, 10);
        header[0] = header[0] | (uint8_t)128;
        if (this->isFromServer)
        {
            header[1] = 0;
        }
        else
        {
            header[1] = header[1] | (uint8_t)0b10000000;
        }

        switch (type)
        {
        case TYPE_TEXT:
            header[0] = header[0] | (uint8_t)1;
            break;
        case TYPE_BIN:
            header[0] = header[0] | (uint8_t)2;
            break;
        case TYPE_CLOSE:
            header[0] = header[0] | (uint8_t)8;
            break;
        case TYPE_PING:
            header[0] = header[0] | (uint8_t)9;
            break;
        case TYPE_PONG:
            header[0] = header[0] | (uint8_t)10;
            break;
        default:
            return 0;
        }

        // calc length
        // 1000 1010 0000 0000
        if (len < 126)
        {
            header[1] = header[1] | (uint8_t)len;
            this->client->write((const char *)header, 2);
        }
        else if (len > 125 && len < 65535)
        {
            header[1] = header[1] | (uint8_t)126;
            uint16_t msgLen = (uint16_t)len;
            header[2] = (uint8_t)(msgLen >> 8);
            header[3] = (uint8_t)((msgLen << 8) >> 8);
            this->client->write((const char *)header, 4);
        }
        else
        {
            // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
            header[1] = header[1] | (uint8_t)127;
            uint64_t msgLen = len;
            header[2] = (uint8_t)(msgLen >> 56);
            header[3] = (uint8_t)((msgLen << 8) >> 56);
            header[4] = (uint8_t)((msgLen << 16) >> 56);
            header[5] = (uint8_t)((msgLen << 24) >> 56);
            header[6] = (uint8_t)((msgLen << 32) >> 56);
            header[7] = (uint8_t)((msgLen << 40) >> 56);
            header[8] = (uint8_t)((msgLen << 48) >> 56);
            header[9] = (uint8_t)((msgLen << 56) >> 56);
            this->client->write((const char *)header, 10);
        }
        // masking
        if (!this->isFromServer)
        {
            for (uint64_t i = 0; i < len; i++)
            {
                yield();
                data[i] = data[i] ^ mask[i & 3];
                yield();
            }
        }

        // send mask
        if (!this->isFromServer)
            this->client->write((const char *)mask, 4);

        // send data
        return this->client->write((const char *)data, len);
    }

    void WebSocketClient::loop()
    {
        if (this->status == WS_CONNECTED)
        {
            if (!this->client->connected())
            {
                this->client->stop();
                this->status = WS_DISCONNECTED;
                this->fn(WS_DISCONNECTED, nullptr, 0);
                return;
            }
            uint64_t len = this->client->read(this->buffer, 2);

            if (!len) // no msg
            {
                return;
            }

            if (len < 0) // socket error
            {
                this->client->stop();
                this->status = TCP_ERROR;
                this->fn(TCP_ERROR, nullptr, 0);

                return;
            }

            // msg arived
            bool isThisFrameisFin = this->buffer[0] & 0b10000000;

            // last frame
            WebSocketEvents type;
            uint8_t opcode = this->buffer[0] & 0b00001111;
            switch (opcode)
            {
            case 0:
                type = TYPE_CON;
                break;
            case 1:
                type = TYPE_TEXT;
                break;
            case 2:
                type = TYPE_BIN;
                break;
            case 9:
                type = TYPE_PING;
                break;
            case 10:
                type = TYPE_PONG;
                break;
            case 8:
                type = TYPE_CLOSE;
            default:
                type = TYPE_UNKNOWN;
                this->client->stop();
                this->status = WS_DISCONNECTED;
                this->fn(WS_DISCONNECTED, nullptr, 0);
                return;
            }
            uint64_t length = (uint64_t)(this->buffer[1] & (uint8_t)(127));
            uint8_t *buf = nullptr;
            uint8_t maskBytes[4];

            // read real length
            if (length > 125)
            {
                // read real length
                bzero(this->buffer, MY_WEBSOCKET_BUFFER_LENGTH);

                // 126: payload length > 125 && payload length < 65536
                // 127: >65535
                uint8_t extraPayloadBytes = length == 126 ? 2 : 8;
                length = 0;

                // read real length bytes
                this->client->read(this->buffer, extraPayloadBytes);

                // convert to uint64_t
                for (uint8_t i = 0; i < extraPayloadBytes; i++)
                {
                    length = length << 8;
                    length += this->buffer[i];
                }

                // see if beyond max length
                if (extraPayloadBytes > MY_WEBSOCKET_CLIENT_MAX_PAYLOAD_LENGTH)
                {
                    // this->client->stop();
                    this->status = MAX_PAYLOAD_EXCEED;
                    this->fn(MAX_PAYLOAD_EXCEED, nullptr, 0);
                    ESP_LOGI("websocket client", "MAX_PAYLOAD_EXCEED");
                    return;
                }
            }

            uint64_t bufferLength = length;
            // length +1 for text type end of string
            if (type == TYPE_TEXT && isThisFrameisFin)
            {
                bufferLength += 1;
            }
            ESP_LOGI("mainloop", "original length:%d", bufferLength);

            // for optimize unmask
            while (bufferLength % 4)
            {
                bufferLength++;
            }
            ESP_LOGI("mainloop", "optimized length:%d", bufferLength);

            // if length overflow it will be 0(though it won't happen forever on esp32)
            if (!bufferLength || this->accBufferOffset + length > MY_WEBSOCKET_CLIENT_MAX_PAYLOAD_LENGTH)
            {
                this->status = MAX_PAYLOAD_EXCEED;
                this->fn(MAX_PAYLOAD_EXCEED, nullptr, 0);
                ESP_LOGI("websocket client", "MAX_PAYLOAD_EXCEED");
                return;
            }

            buf = new uint8_t[bufferLength];

            // if this client is transfer from server
            // that means 4 bytes mask key should read at first
            if (this->isFromServer)
            {
                this->client->read(maskBytes, 4);
            }

            // otherwise this client is directly connected to remote
            // no mask key should read

            uint64_t readLength = this->client->read(buf, length);
            int times = 0;
            while (readLength < length)
            {
                while (!this->client->available())
                {
                    yield();
                }
                yield();
                int segmentLength = this->client->read(buf + readLength, length);
                readLength += segmentLength;
                if (++times > READ_MAX_TIMES)
                {
                    delete buf;
                    this->status = REACH_MAX_READ_TIMES;
                    client->stop();
                    this->fn(REACH_MAX_READ_TIMES, nullptr, 0);
                    return;
                }
            }
            if (readLength == length)
            {
                if (this->isFromServer)
                {
                    // unmask
                    uint64_t t = micros();

                    // this will consume 2200us if length type is uint64_t with 240MHz cpu config
                    // data size 64KB, AP mode
                    // this is for transfer large binary data from client to server
                    for (uint64_t i = 0; i ^ length; i += 4)
                    {
                        buf[i] = buf[i] ^ maskBytes[i & 3];
                        buf[(i + 1)] = buf[(i + 1)] ^ maskBytes[(i + 1) & 3];
                        buf[(i + 2)] = buf[(i + 2)] ^ maskBytes[(i + 2) & 3];
                        buf[(i + 3)] = buf[(i + 3)] ^ maskBytes[(i + 3) & 3];
                    }

                    memset(buf + length, 0, bufferLength - length);

                    ESP_LOGI("mainloop", "unmask time:%d us", (long)(micros() - t));
                }

                if (type == TYPE_TEXT && isThisFrameisFin)
                {
                    buf[length] = 0;
                }

                // copy data to buffer if this frame isn't last frame
                // otherwise call callback
                if (isThisFrameisFin)
                {

                    // call handler
                    if (this->accBufferOffset)
                    {
                        memcpy(this->accBuffer + this->accBufferOffset, buf, length);
                        this->accBufferOffset += length;
                        this->fn(type, this->accBuffer, this->accBufferOffset);
                        delete this->accBuffer;
                        this->accBufferOffset = 0;
                    }
                    else
                    {
                        this->fn(type, buf, length);
                    }
                }
                else
                {
                    if (!this->accBufferOffset)
                    {
                        this->accBuffer = new uint8_t[MY_WEBSOCKET_CLIENT_MAX_PAYLOAD_LENGTH];
                    }
                    memcpy(this->accBuffer + this->accBufferOffset, buf, length);
                    this->accBufferOffset += length;
                }
            }
            else
            {
                this->client->stop();
                this->status = WebSocketEvents::TCP_ERROR;
                this->fn(TCP_ERROR, nullptr, 0);
            }
            delete buf;
        }
        else
        { // disconnected
            if (this->autoReconnect)
            {
                if (millis() - this->lastConnectTime > this->connectTimeout)
                {
                    this->lastConnectTime = millis();
                    if (this->client != nullptr)
                    {
                        this->client->stop();
                        delete this->client;
                        this->client = nullptr;
                    }
                    this->handShake();
                }
            }
        }
    }

    bool CombinedServer::begin(uint16_t port)
    {
        this->server = new WiFiServer(port, 100);
        this->server->setNoDelay(true);
        this->server->begin();
        return true;
    }

    int CombinedServer::isWebSocketClientArrayHasFreeSapce()
    {
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (nullptr == this->webSocketClients[i])
            {
                return i;
                break;
            }
        }
        return -1;
    }

    int CombinedServer::isHttpClientArrayHasFreeSpace()
    {
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (nullptr == this->clients[i])
            {
                return i;
                break;
            }
        }
        return -1;
    }

    void CombinedServer::newWebSocketClientHandShanke(WiFiClient *client, String request, int index)
    {
        // websocket
        int keyStart = request.indexOf("Sec-WebSocket-Key: ") + 19;
        if (keyStart < 0)
        {
            // client->print("HTTP/1.1 403\r\n\r\n");
            // client->flush();
            client->stop();
            return;
        }

        // to generate server key
        String clientKey = request.substring(keyStart, request.indexOf("\r\n", keyStart));

        String serverKey = clientKey + String("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
        serverKey = generateServerKey(serverKey);

        String response = "HTTP/1.1 101 Switching Protocols\r\n";
        response += "Connection: upgrade\r\n";
        response += "Upgrade: websocket\r\n";
        response += "Content-Length: 0\r\n";
        response += "Sec-WebSocket-Accept: " + serverKey + "\r\n\r\n";

        // give response to client
        client->print(response);
        client->flush();

        // transfer to a object of WebSocketClient
        WebSocketClient *webSocketClient = new WebSocketClient(client);

        // set status and callback
        webSocketClient->status = WS_CONNECTED;
        webSocketClient->setCallBack(
            [this, webSocketClient](WebSocketEvents type, uint8_t *payload, uint64_t length)
            {
                // here don't process any events
                // all events will push to callback
                this->fn(webSocketClient, type, payload, length);
            });

        // push it into queue
        this->webSocketClients[index] = webSocketClient;
    }

    int CombinedServer::findHttpCallback(String path)
    {
        if (this->nonWebSocketRequests.size() <= 0)
        {
            return -1;
        }

        for (int i = 0; i < this->nonWebSocketRequests.size(); i++)
        {
            if (path == this->nonWebSocketRequests.at(i)->path)
            {
                return i;
            }
        }
        return -1;
    }

    void CombinedServer::httpHandler(ExtendedWiFiClient *client, String *request)
    {
        // from http handshake
        if (request != nullptr)
        {
            if (request->length() > 0)
            {
                HttpMethod method;
                uint32_t pathStart = request->indexOf("GET ") + 4;
                if (pathStart < 0)
                {
                    pathStart = request->indexOf("POST ") + 5;
                }
                uint32_t pathEnd = request->indexOf(" HTTP", pathStart);
                if (pathStart >= pathEnd)
                {
                    ESP_LOGI("mywebsocket", "can not find path");
                    return;
                }
                String path = request->substring(pathStart, pathEnd);
                if (request->indexOf("GET ") >= 0)
                {
                    method = GET;
                }
                else if (request->indexOf("POST ") >= 0)
                {
                    method = POST;
                }
                else
                {
                    // not support yet
                    method = OTHERS;
                }

                if (path.length())
                {

                    int index = this->findHttpCallback(path);

                    if (index >= 0)
                    {
                        if (this->autoFillHttpResponseHeader)
                        {
                            String res = "HTTP/1.1 200 OK\r\n";
                            res += "Content-Type: " + this->nonWebSocketRequests.at(index)->mimeType + "\r\n";
                            res += "Connection: keep-alive\r\n";
                            res += "Transfer-Encoding: chunked\r\n\r\n";
                            client->print(res);
                        }

                        this->nonWebSocketRequests.at(index)->fn(client, method, (uint8_t *)request, request->length());
                    }
                    else
                    {
                        ESP_LOGI("mywebsocket", "no http handler mathced");
                        client->print("HTTP/1.1 404\r\n");
                        client->print("Connection: close\r\n");
                        client->print("Content-Type: text/plain\r\n");
                        client->print("Content-Length: 3\r\n\r\n404");
                        client->flush();
                        client->stop();
                    }
                }
                else
                {
                    client->stop();
                    ESP_LOGI("empty path");
                }
                return;
            }
            else
            {
                client->stop();
                ESP_LOGI("request is empty", "unknown error");
            }
        }

        // from loop
        if (this->publicPostHandler == nullptr)
        {
            return;
        }

        uint8_t *data = new uint8_t[MY_WEBSOCKET_HTTP_POST_LENGTH];

        long len = client->read(data, MY_WEBSOCKET_HTTP_POST_LENGTH);

        try
        {
            this->publicPostHandler->fn(client, NO_METHOD, data, len);
        }
        catch (std::exception &e)
        {
            delete data;
            data = nullptr;
        }
        if (data != nullptr)
        {
            delete data;
        }
    }

    void CombinedServer::loop()
    {
        if (this->server->hasClient())
        {
            ESP_LOGI("WebServerMainLoop", "Free Heap: %d", ESP.getFreeHeap());
            WiFiClient newClient = this->server->available();
            ExtendedWiFiClient *client = new ExtendedWiFiClient(newClient);
            client->setNoDelay(true);

            String request = "";
            if (client->available())
            {
                int len = client->read(this->headerBuffer, MY_WEBSOCKET_HTTP_MAX_HEADER_LENGTH);
                if (len < 0)
                {
                    delete client;
                    return;
                }
                if (len >= MY_WEBSOCKET_HTTP_MAX_HEADER_LENGTH)
                {
                    delete client;
                    return;
                }
                this->headerBuffer[len] = 0;
                request = String((char *)this->headerBuffer);
                memset(this->headerBuffer, 0xff, MY_WEBSOCKET_HTTP_MAX_HEADER_LENGTH);
            }
            else
            {
                return;
            }

            if (request.length() > 0)
            {
                if (request.indexOf("\r\n\r\n") >= 0)
                {
                    if (
                        request.indexOf("Connection: Upgrade") >= 0 &&
                        request.indexOf("Upgrade: websocket") >= 0 &&
                        request.indexOf("Sec-WebSocket-Key") >= 0)
                    {

                        int index = this->isWebSocketClientArrayHasFreeSapce();
                        if (index < 0)
                        {
                            client->print("HTTP/1.1 404\r\n\r\n");
                            client->flush();
                            client->stop();
                            delete client;
                        }
                        else
                        {
                            this->newWebSocketClientHandShanke(client, request, index);
                        }
                    }
                    else
                    {
                        // http

                        int index = this->isHttpClientArrayHasFreeSpace();
                        if (index >= 0)
                        {
                            this->clients[index] = client;
                            this->httpHandler(client, &request);
                        }
                        else
                        {
                            client->stop();
                            delete client;
                        }
                    }
                }
            }
        }

        // loop websocket clients
        WebSocketClient *loopClient;

        // loop http clients
        ExtendedWiFiClient *extendedclient;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            loopClient = this->webSocketClients[i];
            if (nullptr != loopClient)
            {
                if (loopClient->connected())
                {
                    if (loopClient->available())
                    {
                        loopClient->loop();
                    }
                }
                else
                {
                    delete loopClient;
                    this->webSocketClients[i] = nullptr;
                    ESP_LOGI("WebSocketClientLoop", "A websocket client was disconnected");
                }
            }
            extendedclient = this->clients[i];
            if (nullptr != extendedclient)
            {
                if (extendedclient->connected())
                {
                    if (extendedclient->available())
                    {
                        this->httpHandler(extendedclient, nullptr);
                    }
                }
                else
                {
                    delete extendedclient;
                    this->clients[i] = nullptr;
                    ESP_LOGI("HttpLoop", "A client was disconnected");
                }
            }
        }
    }

    void CombinedServer::on(const char *path, NonWebScoketCallback fn, String mimeType, bool cover)
    {
        HttpCallback *cb = new HttpCallback();
        cb->path = String(path);
        cb->mimeType = mimeType;
        cb->fn = fn;
        bool stored = false;
        for (
            std::vector<HttpCallback *>::iterator it = this->nonWebSocketRequests.begin();
            it != this->nonWebSocketRequests.end();
            it++)
        {
            if ((*it)->path == cb->path)
            {
                if (cover)
                {
                    (*it)->fn = fn;
                    delete cb;
                }
                else
                {
                    delete (*it);
                    this->nonWebSocketRequests.push_back(cb);
                }
                stored = true;
            }
        }
        if (!stored)
        {
            this->nonWebSocketRequests.push_back(cb);
        }
    }

    CombinedServer::~CombinedServer()
    {
        // do some clean process
        if (this->headerBuffer != nullptr)
        {
            delete this->headerBuffer;
        }
        if (this->publicPostHandler != nullptr)
        {
            delete this->publicPostHandler;
        }
        if (this->nonWebSocketRequests.size() > 0)
        {
            for (int i = 0; i < this->nonWebSocketRequests.size(); i++)
            {
                delete this->nonWebSocketRequests.at(i);
            }
        }
        if (this->nonWebSocketRequests.size() > 0)
        {
            for (int i = 0; i < this->nonWebSocketRequests.size(); i++)
            {
                delete this->nonWebSocketRequests.at(i);
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (nullptr != this->clients[i])
            {
                this->clients[i]->stop();
                delete this->clients[i];
            }
            if (nullptr != this->webSocketClients[i])
            {
                this->webSocketClients[i]->stop();
                delete this->webSocketClients[i];
            }
        }
        std::vector<HttpCallback *>().swap(this->nonWebSocketRequests);
    }
};
