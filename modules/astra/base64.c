/*  Copyright (c) 2006-2007, Philip Busch <broesel@studcs.uni-sb.de>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include "base.h"
#include <stdlib.h>
#include <string.h>

static const char base64_list[] = \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define XX 100

static const int base64_index[256] =
{
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,62, XX,XX,XX,63,
    52,53,54,55, 56,57,58,59, 60,61,XX,XX, XX,XX,XX,XX,
    XX, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,XX, XX,XX,XX,XX,
    XX,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
    41,42,43,44, 45,46,47,48, 49,50,51,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
};

static void base64_encode_block(uint8_t out[4], const uint8_t in[3], int len)
{
    out[0] = base64_list[ in[0] >> 2 ];
    out[1] = base64_list[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (uint8_t)((len > 1) ? base64_list[((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)]
                                 : '=');
    out[3] = (uint8_t)((len > 2) ? base64_list[in[2] & 0x3f] : '=');
}

static int base64_decode_block(uint8_t out[3], const uint8_t in[4])
{
    int i, numbytes = 3;
    char tmp[4];

    for(i = 3; i >= 0; i--)
    {
        if(in[i] == '=')
        {
            tmp[i] = 0;
            numbytes = i - 1;
        }
        else
            tmp[i] = base64_index[(uint8_t)in[i]];

        if(tmp[i] == XX)
            return(-1);
    }

    out[0] = (uint8_t)(  tmp[0] << 2 | tmp[1] >> 4);
    out[1] = (uint8_t)(  tmp[1] << 4 | tmp[2] >> 2);
    out[2] = (uint8_t)(((tmp[2] << 6) & 0xc0) | tmp[3]);

    return numbytes;
}

static size_t base64_encoded_size(size_t len)
{
    return(((len + 2) / 3) * 4);
}

static size_t base64_decoded_size(size_t len)
{
    return((len / 4) * 3);
}

static void base64_encode_binary(char *out, const uint8_t *in, size_t len)
{
    int size;
    size_t i = 0;

    while(i < len)
    {
        size = (len-i < 4) ? len-i : 4;
        base64_encode_block((uint8_t *)out, in, size);

        out += 4;
        in  += 3;
        i   += 3;
    }

    *out = '\0';
}

static int base64_decode_binary(uint8_t *out, const char *in)
{
    size_t len = strlen(in), i = 0;
    int numbytes = 0;

    while(i < len)
    {
        if((numbytes += base64_decode_block(out, (uint8_t *)in)) < 0)
            return(-1);

        out += 3;
        in  += 4;
        i   += 4;
    }

    return numbytes;
}

char * base64_encode(const char *in, size_t size, size_t *key_size)
{
    char *out;
    size_t outlen;

    if(in == NULL)
        return(NULL);

    outlen = base64_encoded_size(size);

    if((out = (char *)malloc(sizeof(char) * (outlen + 1))) == NULL)
        return NULL;

    base64_encode_binary(out, (uint8_t *)in, size);
    if(key_size)
        *key_size = outlen;

    return out;
}

char * base64_decode(const char *in, size_t *data_size)
{
    char *out;
    size_t outlen;
    int numbytes;

    outlen = base64_decoded_size(strlen(in));

    if((out = (char *)malloc(sizeof(char) * (outlen + 1))) == NULL)
        return NULL;

    if((numbytes = base64_decode_binary((uint8_t *)out, in)) < 0)
    {
            free(out);
            return NULL;
    }

    *(out + numbytes) = '\0';
    if(data_size)
        *data_size = outlen;

    return out;
}
