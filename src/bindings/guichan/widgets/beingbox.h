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

#ifndef BEINGBOX_H
#define BEINGBOX_H

#include <guichan/widget.hpp>

class Being;
class BeingBoxConfigListener;
class ImageRect;

/**
 * A box showing a player character.
 *
 * \ingroup GUI
 */
class BeingBox : public gcn::Widget
{
    public:
        friend class BeingBoxConfigListener;

        /**
         * Constructor. Takes the initial player character that this box should
         * display, which defaults to <code>NULL</code>.
         */
        BeingBox(const Being *being = NULL);

        /**
         * Destructor.
         */
        ~BeingBox();

        /**
         * Sets the being to be displayed by this box. Setting the
         * being to <code>NULL</code> causes the box not to draw any
         * being.
         */
        void setBeing(const Being *being) { mBeing = being; }

        /**
         * Draws the scroll area.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Draws the background and border of the scroll area.
         */
        void drawFrame(gcn::Graphics *graphics);

    protected:
        static BeingBoxConfigListener *mConfigListener;
        static float mAlpha;

    private:
        const Being *mBeing;       /**< The being used for display */

        static int instances;
        static ImageRect background;
};

#endif
