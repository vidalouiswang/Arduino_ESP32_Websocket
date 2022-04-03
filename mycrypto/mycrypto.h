/*
    This library able to get sha1/256 and base64 encode/decode using in the
    "esp32-arduino" framework.
    SHA part is according to offical document of ESP32-WROOM-32(D/E/UE),
    using ESP32 hardware acceleration to get sha digest.
    If you are using ESP32-C3 or other modules of ESP32, you
    should replace those "registers address" to fit in the core,
    because of these chip may have different address.
*/

#ifndef MY_CRYPTO_H_
#define MY_CRYPTO_H_

#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "soc/dport_access.h"
#include "soc/hwcrypto_reg.h"

#if ESP_IDF_VERSION_MAJOR >= 4
#include "esp_private/periph_ctrl.h"
#else
#include "driver/periph_ctrl.h"
#endif

namespace mycrypto
{
    typedef enum
    {
        SHA1 = 1,
        SHA256 = 0
    } SHAType;

    typedef enum
    {
        LOWER_CASE,
        UPPER_CASE
    } SHAOutputCase;

    class SHA
    {
    private:
        static void sha(uint8_t *data, uint64_t length, uint32_t *output, SHAType type = SHA256);
        static String aSHA(uint8_t *data, uint64_t length, SHAType type, SHAOutputCase hexCase = LOWER_CASE);
        static void convertU32ToU8(uint8_t *data, uint64_t length, uint8_t *output, SHAType type);

    public:
        static inline void initialize()
        {
            periph_module_enable(PERIPH_SHA_MODULE);
        }

        static inline void sha1(uint8_t *data, uint64_t length, uint32_t *output)
        {
            sha(data, length, output, SHA1);
        }

        static inline void sha1(uint8_t *data, uint64_t length, uint8_t *output)
        {
            convertU32ToU8(data, length, output, SHA1);
        }

        static inline void sha256(uint8_t *data, uint64_t length, uint8_t *output)
        {
            convertU32ToU8(data, length, output, SHA256);
        }

        static inline void sha256(uint8_t *data, uint64_t length, uint32_t *output)
        {
            sha(data, length, output, SHA256);
        }

        static inline String sha1(uint8_t *data, uint64_t length, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA(data, length, SHA1, hexCase);
        }

        static inline String sha256(uint8_t *data, uint64_t length, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA(data, length, SHA256, hexCase);
        }

        static inline String sha1(String data, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA((uint8_t *)data.c_str(), data.length(), SHA1, hexCase);
        }

        static inline String sha256(String data, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA((uint8_t *)data.c_str(), data.length(), SHA256, hexCase);
        }

        static inline String sha1(String *data, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA((uint8_t *)data->c_str(), data->length(), SHA1, hexCase);
        }

        static inline String sha256(String *data, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA((uint8_t *)data->c_str(), data->length(), SHA256, hexCase);
        }

        static inline String sha1(const char *data, uint64_t length, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA((uint8_t *)data, length, SHA1, hexCase);
        }

        static inline String sha256(const char *data, uint64_t length, SHAOutputCase hexCase = LOWER_CASE)
        {
            return aSHA((uint8_t *)data, length, SHA256, hexCase);
        }
    };

    class Base64
    {
    private:
        static uint8_t getCharIndex(uint8_t c);
        static uint8_t *base64Decode(uint8_t *data, uint64_t iLen, uint64_t *oLen);

    public:
        static char *base64Encode(uint8_t *data, uint64_t length);

        static inline String base64Encode(const char *data, uint64_t length)
        {
            char *a = base64Encode((uint8_t *)data, length);
            String b = String(a); // this will make a copy in RAM
            delete a;
            return b;
        }

        static inline String base64Encode(String data)
        {
            return base64Encode((const char *)data.c_str(), data.length());
        }

        static inline uint8_t *base64Decode(std::string data, uint64_t *oLen)
        {
            return base64Decode((uint8_t *)data.c_str(), data.length() - 1, oLen);
        }

        static inline uint8_t *base64Decode(String data, uint64_t *oLen)
        {
            return base64Decode((uint8_t *)data.c_str(), data.length(), oLen);
        }

        static inline String base64Decode(String data)
        {
            uint64_t oLen = 0;
            uint8_t *output = base64Decode((uint8_t *)data.c_str(), data.length(), &oLen);
            String a = String((char *)output); // this will make a copy
            delete output;
            return a;
        }

        static inline uint8_t *base64Decode(const char *data, uint64_t iLen, uint64_t *oLen)
        {
            return base64Decode((uint8_t *)data, iLen, oLen);
        }
    };
}

#endif