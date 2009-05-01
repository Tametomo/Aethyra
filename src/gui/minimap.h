/*
 *  Aethyra
 *  Copyright (C) 2004-2005  The Mana World Development Team
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

#ifndef MINIMAP_H
#define MINIMAP_H

#include "../bindings/guichan/widgets/window.h"

class Image;
class Map;

/**
 * Minimap window. Shows a minimap image and the name of the current map.
 *
 * The name of the map is defined by the map property "name". The minimap image
 * is defined by the map property "minimap". The path to the image should be
 * given relative to the root of the client data.
 *
 * \ingroup Interface
 */
class Minimap : public Window
{
    public:
        Minimap();
        ~Minimap();

        /**
         * Sets the map image that should be displayed.
         */
        void setMap(Map *map);

        /**
         * Toggles the displaying of the minimap.
         */
        void toggle();

        /**
         * Overrridden close() method so that the user's visible state for the
         * minimap can be remembered;
         */
        void close();

        /**
         * Hides/unhides the minimap
         */
        void hide();

        /**
         * Draws the minimap.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Overrridden to allow the MiniMap to keep track of the user set width
         * and height values.
         */
        void mouseReleased(gcn::MouseEvent &event);

    private:
        Image *mMapImage;
        float mWidthProportion;
        float mHeightProportion;
        static bool mShow;
        static int mUserWidth;
        static int mUserHeight;
};

extern Minimap *minimap;

#endif
