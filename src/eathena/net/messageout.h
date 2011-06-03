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

#ifndef MESSAGEOUT_
#define MESSAGEOUT_

#include <iosfwd>
#include <stdint.h>

/**
 * Used for building an outgoing message.
 */
class MessageOut
{
    friend MessageOut& operator<<(MessageOut &msg, const int8_t &rhs);
    friend MessageOut& operator<<(MessageOut &msg, const int16_t &rhs);
    friend MessageOut& operator<<(MessageOut &msg, const int32_t &rhs);

    public:
        /**
         * Constructor.
         */
        MessageOut(short id);

        void writeInt8(int8_t value);          /**< Writes a byte. */
        void writeInt16(int16_t value);        /**< Writes a short. */
        void writeInt32(int32_t value);          /**< Writes a long. */

        /**
         * Encodes coordinates and direction in 3 bytes. Used by eAthena.
         */
        void writeCoordinates(unsigned short x, unsigned short y,
                              unsigned char direction);

        /**
         * Writes a string. If a fixed length is not given (-1), it is stored
         * as a short at the start of the string.
         */
        void writeString(const std::string &string, int length = -1);

    private:
        char *mData;                         /**< Data building up. */
        unsigned int mDataSize;              /**< Size of data. */
        unsigned int mPos;                   /**< Position in the data. */
};

#endif
