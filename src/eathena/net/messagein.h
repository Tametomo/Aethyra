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

#ifndef MESSAGEIN_
#define MESSAGEIN_

#include <stdint.h>
#include <string>

/**
 * Used for parsing an incoming message.
 */
class MessageIn
{
    public:
        /**
         * Constructor.
         */
        MessageIn(const char *data, unsigned int length);

        /**
         * Returns the message ID.
         */
        short getId() { return mId; }

        /**
         * Returns the message length.
         */
        unsigned int getLength() { return mLength; }

        int readInt8();               /**< Reads a byte. */
        int readInt16();              /**< Reads a short. */
        int readInt32();              /**< Reads a long. */

        /**
         * Reads a special 3 byte block used by eAthena, containing x and y
         * coordinates and direction.
         */
        void readCoordinates(uint16_t &x, uint16_t &y, uint8_t &direction);

        /**
         * Reads a special 5 byte block used by eAthena, containing a source
         * and destination coordinate pair.
         */
        void readCoordinatePair(uint16_t &srcX, uint16_t &srcY,
                                uint16_t &dstX, uint16_t &dstY);

        /**
         * Skips a given number of bytes.
         */
        void skip(unsigned int length);

        /**
         * Reads a string. If a length is not given (-1), it is assumed
         * that the length of the string is stored in a short at the
         * start of the string.
         */
        std::string readString(int length = -1);

    private:
        const char *mData;             /**< The message data. */
        unsigned int mLength;          /**< The length of the data. */
        unsigned int mPos;             /**< The position in the data. */
        short mId;                     /**< The message ID. */
};

#endif
