/*
    This is a small websocket server/client and http server component.
    And is not finished yet.
    But currently, it could use for project.
    Fast speed, no memory leak.
    Designed for ESP32-WROOM-32 work with offical framework arduino-esp32 version 1.0.6.
    Because of the default stack size of offical framework is tiny, so code use free store memory as could as possible.
    Otherwise your should modify default stack size in sdk config and do some changes with this component.
    Websocket client/server don't support wss, because https is not absolute safe either.
    With ESP lOT env, I prefer use AES to encrypt data before send it, that much safer than https.
    Author: Vida Wang.
*/

#ifndef MY_WEBSOCKET_H_
#define MY_WEBSOCKET_H_

#include <Arduino.h>
#include <WifiClient.h>
#include <base64.h>
#include <vector>
#include <functional>
#include <hwcrypto/sha.h> //this is only for esp32-arudino 1.0.6
#include <WiFiServer.h>

#define myWebSocketHeader "GET @PATH@ HTTP/1.1\r\nHost: @HOST@\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nCache-Control: no-cache\r\nUpgrade: websocket\r\nSec-WebSocket-Key: @KEY@\r\n\r\n"

#define READ_MAX_TIMES 100

// payload length
#define MY_WEBSOCKET_CLIENT_MAX_PAYLOAD_LENGTH 102400

// buffer length for websocket handshake
#define MY_WEBSOCKET_BUFFER_LENGTH 1024

// http post body length
#define MY_WEBSOCKET_HTTP_POST_LENGTH 1024

// http header length
#define MY_WEBSOCKET_HTTP_MAX_HEADER_LENGTH 1024

// client length
#define MAX_CLIENTS 10

namespace myWebSocket
{

    // to calc server key
    String generateServerKey(String clientKey);

    typedef enum
    {
        TCP_CONNECTED = 0xfe,
        TCP_TIMEOUT = 0xfd,
        TCP_FAILED = 0xfc,
        WS_CONNECTED = 0xfb,
        WS_DISCONNECTED = 0xf9,
        TCP_ERROR = 0xf8,
        HANDSHAKE_UNKNOWN_ERROR = 0xf7,
        MAX_PAYLOAD_EXCEED = 0xf6,
        MEMORY_FULL = 0xf5,
        REACH_MAX_READ_TIMES = 0xae,
        TYPE_CON = 0x00,
        TYPE_TEXT = 0x01,
        TYPE_BIN = 0x02,
        TYPE_CLOSE = 0x08,
        TYPE_PING = 0x09,
        TYPE_PONG = 0x0a,
        TYPE_UNKNOWN = 0xff
    } WebSocketEvents;

    // websocket client message callback
    typedef std::function<void(WebSocketEvents type, uint8_t *payload, uint64_t length)> WebSocketMessageCallback;

    class WebSocketClient
    {
    private:
        String host;
        uint16_t port;
        String path; // currently only support "/"
        bool handShake();
        String domain;
        WebSocketMessageCallback fn;
        WiFiClient *client = nullptr;

        uint8_t *accBuffer = nullptr;
        uint64_t accBufferOffset = 0;

        // this will be true if this client is transfer from local websocket server
        // the client will not reconnect automaticlly if this will be true
        bool isFromServer = false;

        bool autoReconnect = true;

        // to store last reconnect time
        uint64_t lastConnectTime = 0;

        // reconnect timeout in ms
        uint64_t connectTimeout = 5000;

        // for handshake
        uint8_t *buffer = new uint8_t[MY_WEBSOCKET_BUFFER_LENGTH];

        // the local key to send to server
        String clientKey;

        String generateHanshake();

        uint64_t _send(WebSocketEvents type, uint8_t *data, uint64_t len);

    public:
        WebSocketEvents status; // to indicate current status of client
        inline WebSocketClient() {}

        // only CombinedServer will call this function
        // to transfer the client in
        inline WebSocketClient(WiFiClient *client)
        {
            this->client = client;
            this->isFromServer = true;
            this->autoReconnect = false;
        }
        inline ~WebSocketClient()
        {
            delete this->buffer;
            if (this->client != nullptr)
            {
                delete this->client;
            }
        }

        inline bool available()
        {
            return this->client->available();
        }

        inline uint8_t connected()
        {
            return this->client->connected();
        }

        inline bool connect(String host, uint16_t port = 80, String path = "/")
        {
            this->host = host;
            this->port = port;
            this->path = path;
            return this->handShake();
        }

        bool connect(String url, bool withHeader = false);

        inline bool connect(const char *url, bool withHeader = false)
        {
            return this->connect(String(url), withHeader);
        }

        inline bool connect(const char *host, uint16_t port, const char *path)
        {
            return this->connect(String(host), port, String(path));
        }

        inline void setCallBack(WebSocketMessageCallback fn)
        {
            this->fn = fn;
        }

        void loop();

        uint64_t send(String *data);

        uint64_t send(String data);

        uint64_t send(const char *data);

        uint64_t send(uint8_t *data, uint64_t length);

        inline void stop()
        {
            this->client->stop();
        }

        inline void setAutoReconnect(bool autoReconnect = true, uint64_t timeout = 5000)
        {
            this->autoReconnect = autoReconnect;
            this->connectTimeout = timeout;
        }
    };

    typedef enum
    {
        GET,
        POST,
        NO_METHOD,
        OTHERS
    } HttpMethod;

    class ExtendedWiFiClient : public WiFiClient
    {
    public:
        inline ExtendedWiFiClient() {}
        inline ExtendedWiFiClient(const WiFiClient &externalClient) : WiFiClient(externalClient) {}
        inline ~ExtendedWiFiClient() {}

        // this is for default method with "Transfer-Encoding: chunked"
        inline uint64_t send(String *content)
        {
            String *res = new String(content->c_str());
            *res = String(res->length(), HEX) + "\r\n" + *res + "\r\n";
            size_t len = 0;
            try
            {
                len = this->print(*res);
            }
            catch (std::exception &e)
            {
            }

            delete res;
            return len;
        }

        // same as above
        inline uint64_t send(const char *content)
        {
            String *res = new String(content);
            auto len = this->send(res);
            delete res;
            return len;
        }

        // same as above
        inline void close()
        {
            this->print("0\r\n\r\n");
            this->flush();
            this->stop();
        }
    };

    // for universal websocket server msg callback
    typedef std::function<void(WebSocketClient *client, WebSocketEvents type, uint8_t *payload, uint64_t length)> WebSocketServerCallback;

    // for http handler
    typedef std::function<void(ExtendedWiFiClient *client, HttpMethod method, uint8_t *data, uint64_t length)> NonWebScoketCallback;

    typedef struct
    {
        String path;
        String mimeType;
        NonWebScoketCallback fn;
    } HttpCallback;

    class CombinedServer
    {
    private:
        // http clients
        ExtendedWiFiClient *clients[MAX_CLIENTS] = {nullptr};

        // websocket clients
        WebSocketClient *webSocketClients[MAX_CLIENTS] = {nullptr};

        WebSocketServerCallback fn;

        // router
        std::vector<HttpCallback *> nonWebSocketRequests;

        WiFiServer *server;

        // for public post
        HttpCallback *publicPostHandler = nullptr;

        // if this set to true you should only provide main content
        // of html/js/css...
        // otherwise you could process raw data in callback
        // edit your own response header or something
        bool autoFillHttpResponseHeader = true;

        uint8_t *headerBuffer = new uint8_t[MY_WEBSOCKET_HTTP_MAX_HEADER_LENGTH];
        int isWebSocketClientArrayHasFreeSapce();
        int isHttpClientArrayHasFreeSpace();
        int findHttpCallback(String path);

    public:
        inline CombinedServer() {}
        ~CombinedServer();
        inline void setCallback(WebSocketServerCallback fn)
        {
            this->fn = fn;
        }

        // if you'd like process raw data then you
        // should call this function at first and set autoFill to false
        inline void setAutoFillHttpResponseHeader(bool autoFill)
        {
            this->autoFillHttpResponseHeader = autoFill;
        }

        // for route http requests
        void on(const char *path, NonWebScoketCallback fn, String mimeType = "text/html;charset=utf-8", bool cover = true);

        void newWebSocketClientHandShanke(WiFiClient *client, String request, int index);

        void httpHandler(ExtendedWiFiClient *client, String *request);

        inline void setPublicPostHandler(NonWebScoketCallback fn)
        {
            if (this->publicPostHandler != nullptr)
            {
                delete this->publicPostHandler;
            }
            this->publicPostHandler = new HttpCallback();
            this->publicPostHandler->path = "";
            this->publicPostHandler->fn = fn;
        }
        bool begin(uint16_t port = 80);
        void loop();
    };
} // namespace myWebSocket

#endif
