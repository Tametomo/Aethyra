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

#include <cstring>
#include <SDL.h>
#include <SDL_endian.h>
#include <string>

#include "messageout.h"
#include "network.h"

MessageOut::MessageOut(short id):
    mData(0),
    mDataSize(0),
    mPos(0)
{
    mData = network->mOutBuffer + network->mOutSize;
    writeInt16(id);
}

void MessageOut::writeInt8(int8_t value)
{
    mData[mPos] = value;
    mPos += sizeof(int8_t);
    network->mOutSize += sizeof(int8_t);
}

void MessageOut::writeInt16(int16_t value)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int16_t swap = SDL_Swap16(value);
    memcpy(mData + mPos, &swap, sizeof(int16_t));
#else
    memcpy(mData + mPos, &value, sizeof(int16_t));
#endif
    mPos += sizeof(int16_t);
    network->mOutSize += sizeof(int16_t);
}

void MessageOut::writeInt32(int32_t value)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int32_t swap = SDL_Swap32(value);
    memcpy(mData + mPos, &swap, sizeof(int32_t));
#else
    memcpy(mData + mPos, &value, sizeof(int32_t));
#endif
    mPos += sizeof(int32_t);
    network->mOutSize += sizeof(int32_t);
}

#define LOBYTE(w)  ((unsigned char)(w))
#define HIBYTE(w)  ((unsigned char)(((unsigned short)(w)) >> 8))

void MessageOut::writeCoordinates(unsigned short x, unsigned short y,
                                  unsigned char direction)
{
    char *data = mData + mPos;
    network->mOutSize += 3;
    mPos += 3;

    short temp;
    temp = x;
    temp <<= 6;
    data[0] = 0;
    data[1] = 1;
    data[2] = 2;
    data[0] = HIBYTE(temp);
    data[1] = (unsigned char) temp;
    temp = y;
    temp <<= 4;
    data[1] |= HIBYTE(temp);
    data[2] = LOBYTE(temp);

    // Translate direction to eAthena format
    switch (direction)
    {
        case 1:
            direction = 0;
            break;
        case 3:
            direction = 1;
            break;
        case 2:
            direction = 2;
            break;
        case 6:
            direction = 3;
            break;
        case 4:
            direction = 4;
            break;
        case 12:
            direction = 5;
            break;
        case 8:
            direction = 6;
            break;
        case 9:
            direction = 7;
            break;
        default:
            // OOPSIE! Impossible or unknown
            direction = (unsigned char) -1;
    }
    data[2] |= direction;
}

void MessageOut::writeString(const std::string &string, int length)
{
    std::string toWrite = string;

    if (length < 0)
    {
        // Write the length at the start if not fixed
        writeInt16(string.length());
    }
    else
    {
        // Make sure the length of the string is no longer than specified
        toWrite = string.substr(0, length);
    }

    // Write the actual string
    memcpy(&mData[mPos], (void*)toWrite.c_str(), toWrite.length());
    mPos += toWrite.length();
    network->mOutSize += toWrite.length();

    // Pad remaining space with zeros
    if (length > (int)toWrite.length())
    {
        memset(&mData[mPos], '\0', length - toWrite.length());
        mPos += length - toWrite.length();
        network->mOutSize += length - toWrite.length();
    }
}

MessageOut& operator<<(MessageOut &msg, const int8_t &rhs)
{
    msg.writeInt8(rhs);
    return msg;
}

MessageOut& operator<<(MessageOut &msg, const int16_t &rhs)
{
    msg.writeInt16(rhs);
    return msg;
}

MessageOut& operator<<(MessageOut &msg, const int32_t &rhs)
{
    msg.writeInt32(rhs);
    return msg;
}
