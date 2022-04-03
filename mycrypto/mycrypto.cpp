#include "mycrypto.h"

namespace mycrypto
{
    // universal sha digest function
    // use esp32 built-in hardware acceleration module
    // all procedures is from Espressif offical technical document
    // befor using this function, you need call "periph_module_enable(PERIPH_SHA_MODULE);" at first
    // this is included in this class
    // call SHA::initialize();
    // or you will got all zero result
    void SHA::sha(uint8_t *data, uint64_t length, uint32_t *output, SHAType type)
    {
        // type 1 for sha1
        // 0 for sha256
        if (length <= 0)
        {
            bzero(output, (type & 1 ? 5 : 8));
            return;
        }

        // original length
        uint64_t ori = length;

        // calc padding length
        length = ((length * 8) % 512);
        uint64_t zeroLength = ((length < 448) ? (448 - length) : (448 + 512 - length)) / 8;

        // add length
        length = ori + zeroLength + 8;

        // allocate buffer
        uint8_t *buf = new (std::nothrow) uint8_t[length];

        if (!buf)
        {
            output[0] = 0;
            return;
        }

        // padding zero
        bzero(buf, length);

        // copy original data
        memcpy(buf, data, ori);

        // padding the "1" after data
        buf[ori] = (uint8_t)0x80;

        // add data length(bits) into the tail
        uint64_t bits = ori * 8;
        for (int i = 0; i < 8; i++)
        {
            buf[ori + zeroLength + i] = (bits >> ((7 - i) * 8)) & 0xff;
        }

        uint64_t i = 0;

        // fill 512 bits(1 block) to start
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 0, (uint32_t)((buf[i + 0] << 24) + (buf[i + 1] << 16) + (buf[i + 2] << 8) + (buf[i + 3])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 4, (uint32_t)((buf[i + 4] << 24) + (buf[i + 5] << 16) + (buf[i + 6] << 8) + (buf[i + 7])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 8, (uint32_t)((buf[i + 8] << 24) + (buf[i + 9] << 16) + (buf[i + 10] << 8) + (buf[i + 11])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 12, (uint32_t)((buf[i + 12] << 24) + (buf[i + 13] << 16) + (buf[i + 14] << 8) + (buf[i + 15])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 16, (uint32_t)((buf[i + 16] << 24) + (buf[i + 17] << 16) + (buf[i + 18] << 8) + (buf[i + 19])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 20, (uint32_t)((buf[i + 20] << 24) + (buf[i + 21] << 16) + (buf[i + 22] << 8) + (buf[i + 23])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 24, (uint32_t)((buf[i + 24] << 24) + (buf[i + 25] << 16) + (buf[i + 26] << 8) + (buf[i + 27])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 28, (uint32_t)((buf[i + 28] << 24) + (buf[i + 29] << 16) + (buf[i + 30] << 8) + (buf[i + 31])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 32, (uint32_t)((buf[i + 32] << 24) + (buf[i + 33] << 16) + (buf[i + 34] << 8) + (buf[i + 35])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 36, (uint32_t)((buf[i + 36] << 24) + (buf[i + 37] << 16) + (buf[i + 38] << 8) + (buf[i + 39])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 40, (uint32_t)((buf[i + 40] << 24) + (buf[i + 41] << 16) + (buf[i + 42] << 8) + (buf[i + 43])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 44, (uint32_t)((buf[i + 44] << 24) + (buf[i + 45] << 16) + (buf[i + 46] << 8) + (buf[i + 47])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 48, (uint32_t)((buf[i + 48] << 24) + (buf[i + 49] << 16) + (buf[i + 50] << 8) + (buf[i + 51])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 52, (uint32_t)((buf[i + 52] << 24) + (buf[i + 53] << 16) + (buf[i + 54] << 8) + (buf[i + 55])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 56, (uint32_t)((buf[i + 56] << 24) + (buf[i + 57] << 16) + (buf[i + 58] << 8) + (buf[i + 59])));
        DPORT_REG_WRITE(DR_REG_SHA_BASE + 60, (uint32_t)((buf[i + 60] << 24) + (buf[i + 61] << 16) + (buf[i + 62] << 8) + (buf[i + 63])));
        i += 64;

        // start
        if (type & 1)
        {
            DPORT_REG_WRITE(SHA_1_START_REG, (uint32_t)(1));
            while (DPORT_REG_READ(SHA_1_BUSY_REG))
            {
                // yield();
                // because of the hardware acceleration is very fast
                // for 8KB data only needs less than 300us(ESPRESSIF YYDS)
                // so yield() is no need to call
            }
        }
        else
        {
            DPORT_REG_WRITE(SHA_256_START_REG, (uint32_t)(1));
            while (DPORT_REG_READ(SHA_256_BUSY_REG))
            {
            }
        }

        // to process other blocks
        // always fill 512bits(a block) at one time
        for (; i < length; i += 64)
        {
            // fill 512 bits into registers to continue
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 0, (uint32_t)((buf[i + 0] << 24) + (buf[i + 1] << 16) + (buf[i + 2] << 8) + (buf[i + 3])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 4, (uint32_t)((buf[i + 4] << 24) + (buf[i + 5] << 16) + (buf[i + 6] << 8) + (buf[i + 7])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 8, (uint32_t)((buf[i + 8] << 24) + (buf[i + 9] << 16) + (buf[i + 10] << 8) + (buf[i + 11])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 12, (uint32_t)((buf[i + 12] << 24) + (buf[i + 13] << 16) + (buf[i + 14] << 8) + (buf[i + 15])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 16, (uint32_t)((buf[i + 16] << 24) + (buf[i + 17] << 16) + (buf[i + 18] << 8) + (buf[i + 19])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 20, (uint32_t)((buf[i + 20] << 24) + (buf[i + 21] << 16) + (buf[i + 22] << 8) + (buf[i + 23])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 24, (uint32_t)((buf[i + 24] << 24) + (buf[i + 25] << 16) + (buf[i + 26] << 8) + (buf[i + 27])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 28, (uint32_t)((buf[i + 28] << 24) + (buf[i + 29] << 16) + (buf[i + 30] << 8) + (buf[i + 31])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 32, (uint32_t)((buf[i + 32] << 24) + (buf[i + 33] << 16) + (buf[i + 34] << 8) + (buf[i + 35])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 36, (uint32_t)((buf[i + 36] << 24) + (buf[i + 37] << 16) + (buf[i + 38] << 8) + (buf[i + 39])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 40, (uint32_t)((buf[i + 40] << 24) + (buf[i + 41] << 16) + (buf[i + 42] << 8) + (buf[i + 43])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 44, (uint32_t)((buf[i + 44] << 24) + (buf[i + 45] << 16) + (buf[i + 46] << 8) + (buf[i + 47])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 48, (uint32_t)((buf[i + 48] << 24) + (buf[i + 49] << 16) + (buf[i + 50] << 8) + (buf[i + 51])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 52, (uint32_t)((buf[i + 52] << 24) + (buf[i + 53] << 16) + (buf[i + 54] << 8) + (buf[i + 55])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 56, (uint32_t)((buf[i + 56] << 24) + (buf[i + 57] << 16) + (buf[i + 58] << 8) + (buf[i + 59])));
            DPORT_REG_WRITE(DR_REG_SHA_BASE + 60, (uint32_t)((buf[i + 60] << 24) + (buf[i + 61] << 16) + (buf[i + 62] << 8) + (buf[i + 63])));

            // continue
            if (type & 1)
            {
                DPORT_REG_WRITE(SHA_1_CONTINUE_REG, (uint32_t)(1));

                while (DPORT_REG_READ(SHA_1_BUSY_REG))
                {
                }
            }
            else
            {
                DPORT_REG_WRITE(SHA_256_CONTINUE_REG, (uint32_t)(1));

                while (DPORT_REG_READ(SHA_256_BUSY_REG))
                {
                }
            }
        }
        free(buf);

        // get sha result
        if (type & 1)
        {
            DPORT_REG_WRITE(SHA_1_LOAD_REG, (uint32_t)(1));
            while (DPORT_REG_READ(SHA_1_BUSY_REG))
            {
            }
        }
        else
        {
            DPORT_REG_WRITE(SHA_256_LOAD_REG, (uint32_t)(1));
            while (DPORT_REG_READ(SHA_256_BUSY_REG))
            {
            }
        }

        uint8_t shaLen = type & 1 ? 5 : 8;

        // read result
        for (int i = 0; i < shaLen; i++)
        {
            output[i] = (uint32_t)DPORT_REG_READ(DR_REG_SHA_BASE + (i * 4));
        }
    }

    // this is for arduino framework
    String SHA::aSHA(uint8_t *data, uint64_t length, SHAType type, SHAOutputCase hexCase)
    {
        // for sha1 is 160 bits which is 5x32 bits
        // for sha256 is 256 bits which is 8x32 bits
        uint8_t shaLen = type & 1 ? 5 : 8;
        uint32_t output[shaLen];

        // call sha
        sha(data, length, output, type);

        // to store formated hex string
        char hex[9];
        bzero(hex, 9);

        // return value
        String res = "";

        // format
        char format[] = "%08x";

        // case
        if (hexCase == UPPER_CASE)
        {
            format[3] = 'X';
        }

        // convert result into hex string
        for (int i = 0; i < shaLen; i++)
        {
            sprintf(hex, format, output[i]);
            res += hex;
            bzero(hex, 9);
        }
        return res;
    }

    void SHA::convertU32ToU8(uint8_t *data, uint64_t length, uint8_t *output, SHAType type)
    {
        int len = type & 1 ? 5 : 8;
        uint32_t o[len];
        sha(data, length, o, type);
        int k = 0;
        for (int i = 0; i < len; i++)
        {
            output[k++] = (uint8_t)((o[i] & (uint32_t)(0xff000000)) >> 24);
            output[k++] = (uint8_t)((o[i] & (uint32_t)(0x00ff0000)) >> 16);
            output[k++] = (uint8_t)((o[i] & (uint32_t)(0x0000ff00)) >> 8);
            output[k++] = (uint8_t)(o[i] & (uint32_t)(0x000000ff));
        }
    }

    // a very simple base64 encode method
    char *Base64::base64Encode(uint8_t *data, uint64_t length)
    {
        const char *base64Table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        uint32_t group = 0;

        uint8_t extra = length % 3;

        uint64_t len = length - extra;

        char *res = new (std::nothrow) char[extra == 0 ? (len / 3 * 4 + 1) : (len / 3 * 4 + 5)]{0};

        uint64_t location = 0;
        for (uint64_t i = 0; i < len; i += 3, location += 4)
        {
            group = (data[i + 0] << 24) + (data[i + 1] << 16) + (data[i + 2] << 8);

            res[location] = base64Table[((uint8_t)(((uint32_t)(4227858432U) & group) >> 26))];
            res[location + 1] = base64Table[((uint8_t)(((uint32_t)(66060288U) & group) >> 20))];
            res[location + 2] = base64Table[((uint8_t)(((uint32_t)(1032192U) & group) >> 14))];
            res[location + 3] = base64Table[((uint8_t)(((uint32_t)(16128U) & group) >> 8))];
            group = 0;
        }

        if (extra == 1)
        {
            res[location] = base64Table[((uint8_t)(data[len] >> 2))];
            res[location + 1] = base64Table[((uint8_t)((data[len] & 3U) << 4))];
            res[location + 2] = '=';
            res[location + 3] = '=';
        }
        else if (extra == 2)
        {
            uint16_t t = (data[len] << 8) + (data[len + 1]);
            res[location] = base64Table[((uint8_t)((uint32_t)(t & 64512U) >> 10))];
            res[location + 1] = base64Table[((uint8_t)((uint32_t)(t & 1008U) >> 4))];
            res[location + 2] = base64Table[((uint8_t)((uint32_t)(t & 15U) << 2))];
            res[location + 3] = '=';
        }

        return res;
    }

    uint8_t Base64::getCharIndex(uint8_t c)
    {
        if (c > 96 && c < 123)
        {
            return c - 97 + 26;
        }
        else if (c > 64 && c < 91)
        {
            return c - 65;
        }
        else if (c > 47 && c < 58)
        {
            return c - 48 + 52;
        }
        else if (c == 43)
        {
            return 62;
        }
        else if (c == 47)
        {
            return 63;
        }
        else
        {
            return 0;
        }
    }

    uint8_t *Base64::base64Decode(uint8_t *data, uint64_t iLen, uint64_t *oLen)
    {
        *oLen = 0;

        if (iLen % 4)
        {
            return nullptr;
        }

        uint64_t eIndex = 0;

        for (int i = 1; i < 3; i++)
        {
            if (data[iLen - i] == '=')
            {
                eIndex = i;
            }
        }

        iLen -= eIndex;

        uint8_t *output = new uint8_t[iLen / 4 * 3 + 3];
        // bzero(output, iLen / 4 * 3 + 2);

        uint8_t tLen = 0;
        uint8_t arr[4] = {0};

        for (uint64_t i = 0; i < iLen; i++)
        {
            arr[tLen++] = data[i];
            if (tLen >= 4)
            {
                output[(*oLen)++] = getCharIndex(arr[0]) << 2 | (getCharIndex(arr[1]) & 48U) >> 4;
                output[(*oLen)++] = getCharIndex(arr[1]) << 4 | (getCharIndex(arr[2]) & 60U) >> 2;
                output[(*oLen)++] = getCharIndex(arr[2]) << 6 | (getCharIndex(arr[3]) & 63U);
                tLen = 0;
            }
        }
        if (tLen == 2)
        {
            output[(*oLen)++] = (getCharIndex(arr[0])) << 2 | (getCharIndex(arr[1])) >> 4;
        }
        else if (tLen == 3)
        {
            output[(*oLen)++] = (getCharIndex(arr[0])) << 2 | (getCharIndex(arr[1])) >> 4;
            output[(*oLen)++] = (getCharIndex(arr[1])) << 4 | (getCharIndex(arr[2])) >> 2;
        }

        output[(*oLen)] = 0;

        return output;
    }
}