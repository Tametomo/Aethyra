/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra.
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

#include <sstream>

#include <guichan/font.hpp>

#include "lettertextwraphandler.h"

std::string LetterTextWrapHandler::wrapText(const gcn::Font *font, 
                                            const std::string &text,
                                            int &maxDimension)
{
    std::stringstream wrappedStream;
    std::string::size_type pos = 0;
    const int hyphenWidth = font->getWidth(mHyphen);
    int minWidth = 0, xpos = 0;

    while (pos < text.size())
    {
        xpos = 0;

        while (isspace(text[pos]) && pos < text.size())
            pos++;

        while (xpos < (maxDimension - hyphenWidth) && pos < text.size())
        {
            if (text[pos] == '\n')
                xpos = 0;

            xpos += font->getWidth(text.substr(pos, 1));
            wrappedStream << text[pos];
            pos++;
        }

        if (xpos > minWidth)
            minWidth = xpos;

        if (pos == text.size())
            break;

        // This would work if we only dealt with the basic ASCII characters, but
        // we don't. Left as a reminder to not simplify to this.
        //if (!(isalpha(text[pos]) || isdigit(text[pos])) || isspace(text[pos - 1]))
        if (isspace(text[pos]) || isspace(text[pos - 1]) || text[pos] == '\n' ||
            ispunct(text[pos]))
            wrappedStream << "\n";
        else
            wrappedStream << mHyphen << "\n";
    }

    maxDimension = minWidth;

    return wrappedStream.str();
}
