/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *  Copyright (C) 2008  Aethyra Development Team
 *
 *  This file is part of Aethyra derived from original code
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

#include <sstream>

#include <guichan/font.hpp>

#include "wordtextwraphandler.h"

std::string WordTextWrapHandler::wrapText(const gcn::Font *font, 
                                          const std::string &text,
                                          int &maxDimension)
{
    // Take the supplied maximum dimension as a starting point and try to beat it
    std::stringstream wrappedStream;
    std::string::size_type spacePos, newlinePos, lastNewlinePos = 0;
    int minWidth = 0, xpos = 0;

    spacePos = text.rfind(" ", text.size());

    if (spacePos != std::string::npos)
    {
        const std::string word = text.substr(spacePos + 1);
        const int length = font->getWidth(word);

        if (length > maxDimension)
            maxDimension = length;
    }

    do
    {
        // Determine next piece of string to wrap
        newlinePos = text.find("\n", lastNewlinePos);

        if (newlinePos == std::string::npos)
            newlinePos = text.size();

        std::string::size_type lastSpacePos = 0;
        std::string line = text.substr(lastNewlinePos, newlinePos -
                                       lastNewlinePos);

        xpos = 0;

        do
        {
            spacePos = line.find(" ", lastSpacePos);

            if (spacePos == std::string::npos)
                spacePos = line.size();

            std::string word = line.substr(lastSpacePos, spacePos -
                                           lastSpacePos);

            int width = font->getWidth(word);

            if (xpos == 0 && width > maxDimension)
            {
                maxDimension = width;
                xpos = width;
                wrappedStream << word;
            }
            else if (xpos != 0 && xpos + font->getWidth(" ") + width <= 
                     maxDimension)
            {
                xpos += font->getWidth(" ") + width;
                wrappedStream << " " << word;
            }
            else if (lastSpacePos == 0)
            {
                xpos += width;
                wrappedStream << word;
            }
            else
            {
                if (xpos > minWidth)
                    minWidth = xpos;

                // The maximum width wasn't big enough. Resize it and try again.
                if (minWidth > maxDimension)
                {
                    maxDimension = minWidth;
                    wrappedStream.clear();
                    wrappedStream.str("");
                    spacePos = 0;
                    lastNewlinePos = 0;
                    newlinePos = text.find("\n", lastNewlinePos);

                    if (newlinePos == std::string::npos)
                        newlinePos = text.size();

                    line = text.substr(lastNewlinePos, newlinePos -
                                       lastNewlinePos);
                    width = 0;
                    break;
                }
                else
                    wrappedStream << "\n" << word;

                xpos = width;
            }
            lastSpacePos = spacePos + 1;
        }
        while (spacePos != line.size());

        if (text.find("\n", lastNewlinePos) != std::string::npos)
            wrappedStream << "\n";

        lastNewlinePos = newlinePos + 1;
    }
    while (newlinePos != text.size());

    if (xpos > minWidth)
        minWidth = xpos;

    maxDimension = minWidth;

    return wrappedStream.str();
}
