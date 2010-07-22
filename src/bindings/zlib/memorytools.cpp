/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <cassert>
#include <cstdlib>
#include <zlib.h>

#include "memorytools.h"

#include "../../core/log.h"

unsigned int inflateMemory(unsigned char *in, const unsigned int &inLength,
                           unsigned char *&out)
{
    int bufferSize = 256 * 1024;
    int ret;
    unsigned int outLength = 0u;
    z_stream strm;

    out = (unsigned char*) malloc(bufferSize);

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = in;
    strm.avail_in = inLength;
    strm.next_out = out;
    strm.avail_out = bufferSize;

    ret = inflateInit2(&strm, 15 + 32);

    if (ret != Z_OK)
        return ret;

    do
    {
        if (strm.next_out == NULL)
        {
            inflateEnd(&strm);
            return Z_MEM_ERROR;
        }

        ret = inflate(&strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);

        switch (ret)
        {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void) inflateEnd(&strm);
                return ret;
        }

        if (ret != Z_STREAM_END)
        {
            out = (unsigned char*) realloc(out, bufferSize * 2);

            if (out == NULL)
            {
                inflateEnd(&strm);
                return Z_MEM_ERROR;
            }

            strm.next_out = out + bufferSize;
            strm.avail_out = bufferSize;
            bufferSize *= 2;
        }
    }
    while (ret != Z_STREAM_END);
    assert(strm.avail_in == 0);

    if (ret != Z_STREAM_END)
    {
        reportError(ret, out);
        outLength = 0u;
    }
    else
        outLength = bufferSize - strm.avail_out;

    (void) inflateEnd(&strm);
    return outLength;
}

void reportError(const int &errorCode, unsigned char *&out)
{
    if (errorCode == Z_MEM_ERROR)
       logger->log("Error: Out of memory while decompressing data!");
    else if (errorCode == Z_VERSION_ERROR)
       logger->log("Error: Incompatible zlib version!");
    else if (errorCode == Z_DATA_ERROR)
       logger->log("Error: Incorrect zlib compressed data!");
    else
       logger->log("Error: Unknown error while decompressing data!");

    free(out);
    out = NULL;
}
