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

#ifndef WORDTEXTWRAPHANDLER_H
#define WORDTEXTWRAPHANDLER_H

#include "textwraphandler.h"

/**
 * A text wrapping handler which takes a given string, and calculates where the
 * class which uses this handler should insert line feeds for line wraps without
 * ever splitting any of the words up. Because of this, if there is a word that
 * is longer than the supplied dimensions, the wrapping algorithm will expand
 * the maximum dimensions as needed.
 */
class WordTextWrapHandler : public TextWrapHandler
{
    public:
        /**
         * Constructor.
         */
        WordTextWrapHandler() {}

        /**
         * Wraps the supplied text based on the given maximum dimension.
         * Overflows the maximum dimension when a word within the text can't fit
         * within that dimension.
         */
        std::string wrapText(const gcn::Font *mFont, const std::string &text,
                             int &maxDimension);
};

#endif
