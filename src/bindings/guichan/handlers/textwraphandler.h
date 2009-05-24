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

#ifndef TEXTWRAPHANDLER_H
#define TEXTWRAPHANDLER_H

#include <string>

#include "../guichanfwd.h"

/**
 * A text wrap handler takes a supplied string, and determines at which spots
 * within the text the widget using this handler should insert line feeds.
 */
class TextWrapHandler
{
    public:
        virtual std::string wrapText(const gcn::Font *font,
                                     const std::string &text,
                                     int &maxDimension) = 0;
};

#endif
