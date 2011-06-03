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

#include <SDL.h>
#include <SDL_endian.h>

#include "messagein.h"

#define MAKEWORD(low,high) \
    ((unsigned short)(((unsigned char)(low)) | \
    ((unsigned short)((unsigned char)(high))) << 8))

MessageIn::MessageIn(const char *data, unsigned int length):
    mData(data),
    mLength(length),
    mPos(0)
{
    // Read the message ID
    mId = readInt16();
}

int MessageIn::readInt8()
{
    int value = -1;
    if (mPos < mLength)
        value = (unsigned char) mData[mPos];

    mPos += 1;
    return value;
}

int MessageIn::readInt16()
{
    int16_t value = -1;
    if (mPos + 2 <= mLength)
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        int16_t swap;
        memcpy(&swap, mData + mPos, sizeof(int16_t));
        value = SDL_Swap16(swap);
#else
        memcpy(&value, mData + mPos, sizeof(int16_t));
#endif
    }
    mPos += 2;
    return value;
}

int MessageIn::readInt32()
{
    int32_t value = -1;
    if (mPos + 4 <= mLength)
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        int32_t swap;
        memcpy(&swap, mData + mPos, sizeof(int32_t));
        value = SDL_Swap32(swap);
#else
        memcpy(&value, mData + mPos, sizeof(int32_t));
#endif
    }
    mPos += 4;
    return value;
}

void MessageIn::readCoordinates(uint16_t &x, uint16_t &y, uint8_t &direction)
{
    if (mPos + 3 <= mLength)
    {
        const char *data = mData + mPos;
        int16_t temp;

        temp = MAKEWORD(data[1] & 0x00c0, data[0] & 0x00ff);
        x = temp >> 6;
        temp = MAKEWORD(data[2] & 0x00f0, data[1] & 0x003f);
        y = temp >> 4;

        direction = data[2] & 0x000f;

        // Translate from eAthena format
        switch (direction)
        {
            case 0:
                direction = 1;
                break;
            case 1:
                direction = 3;
                break;
            case 2:
                direction = 2;
                break;
            case 3:
                direction = 6;
                break;
            case 4:
                direction = 4;
                break;
            case 5:
                direction = 12;
                break;
            case 6:
                direction = 8;
                break;
            case 7:
                direction = 9;
                break;
            default:
                // OOPSIE! Impossible or unknown
                direction = 0;
        }
    }
    mPos += 3;
}

void MessageIn::readCoordinatePair(uint16_t &srcX, uint16_t &srcY,
                                   uint16_t &dstX, uint16_t &dstY)
{
    if (mPos + 5 <= mLength)
    {
        const char *data = mData + mPos;
        int16_t temp;

        temp = MAKEWORD(data[3], data[2] & 0x000f);
        dstX = temp >> 2;

        dstY = MAKEWORD(data[4], data[3] & 0x0003);

        temp = MAKEWORD(data[1], data[0]);
        srcX = temp >> 6;

        temp = MAKEWORD(data[2], data[1] & 0x003f);
        srcY = temp >> 4;
    }
    mPos += 5;
}

void MessageIn::skip(unsigned int length)
{
    mPos += length;
}

std::string MessageIn::readString(int length)
{
    // Get string length
    if (length < 0)
        length = readInt16();

    // Make sure the string isn't erroneous
    if (length < 0 || mPos + length > mLength)
    {
        mPos = mLength + 1;
        return "";
    }

    // Read the string
    char const *stringBeg = mData + mPos;
    char const *stringEnd = (char const *)memchr(stringBeg, '\0', length);
    std::string readString(stringBeg,
                           stringEnd ? stringEnd - stringBeg : length);
    mPos += length;
    return readString;
}
