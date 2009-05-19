/*
 *  Aethyra
 *  Copyright (C) 2008,  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2009,  Aethyra Development Team
 *
 *  This file is part of Aethyra derived from original code
 *  from The Legend of Mazzeroth.
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

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <string>

#include "container.h"

class TextBox;

class ToolTip : public Container
{
    public:
        /**
         * Constructor.
         */
        ToolTip();

        /**
         * Draws the tooltip.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Sets the text to be displayed.
         */
        void setText(std::string text);

        /**
         * Adjusts the size of the speech bubble.
         */
        void adjustSize();

        /**
         * Sets the location to display the tooltip.
         */
        void view(int x, int y);

    private:
        std::string mText;
        TextBox *mToolTipBox;
};

#endif
