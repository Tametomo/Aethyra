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

#ifndef IMAGE_BUTTON_H
#define IMAGE_BUTTON_H

#include "button.h"

class Image;

/**
 * ImageButton widget. This is rather similar to the GUIChan image button
 * class, except that it uses both the internal button class for our client, as
 * well as the internal image class as well. As such, they serve similar
 * functions, but are not the same.
 *
 * \ingroup GUI
 */
class ImageButton : public Button
{
    public:
        /**
         * Constructor. Loads the image for the button from the given path.
         */
        ImageButton(const std::string &image, const std::string &actionEventId,
                    gcn::ActionListener *listener, unsigned int padding = 0);

        /**
         * Constructor. Loads the image for the button from an existing image.
         */
        ImageButton(Image* image, const std::string &actionEventId,
                    gcn::ActionListener *listener, unsigned int padding = 0);

        /**
         * Destructor.
         */
        ~ImageButton();

        /**
         * Draws the ImageButton.
         */
        void draw(gcn::Graphics *graphics);

    private:
        Image* mImage;         /**< Image to display on the button */
        static float mAlpha;
};

#endif
