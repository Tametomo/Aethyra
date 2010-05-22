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

#ifndef _SDL_GRAPHICS_H
#define _SDL_GRAPHICS_H

#include "../graphics.h"

class Image;
class ImageRect;

struct SDL_Surface;

/**
 * A central point of control for SDLGraphics.
 */
class SDLGraphics : public Graphics
{
    public:
        /**
         * Constructor.
         */
        SDLGraphics();

        /**
         * Destructor.
         */
        virtual ~SDLGraphics();

        virtual bool pushClipArea(gcn::Rectangle area);

        virtual void popClipArea();

        virtual void drawPoint(int x, int y);

        virtual void drawLine(int x1, int y1, int x2, int y2);

        virtual void drawRectangle(const gcn::Rectangle& rectangle);

        virtual void fillRectangle(const gcn::Rectangle& rectangle);

        virtual void setColor(const gcn::Color& color);

        /**
         * Try to create a window with the given settings.
         */
        virtual bool setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel);

        /**
         * Blits an image onto the screen.
         *
         * @return <code>true</code> if the image was blitted properly
         *         <code>false</code> otherwise.
         */
        virtual bool drawImage(Image *image, int srcX, int srcY,
                               int dstX, int dstY, int width, int height,
                               bool useColor = false);

        virtual void drawImagePattern(Image *image, int x, int y, int w, int h);

        /**
         * Draws a rectangle using images. 4 corner images, 4 side images and 1
         * image for the inside.
         */
        void drawImageRect(int x, int y, int w, int h, Image *topLeft,
                           Image *topRight, Image *bottomLeft,
                           Image *bottomRight, Image *top, Image *right,
                           Image *bottom, Image *left, Image *center);

        /**
         * Draws a rectangle using images. 4 corner images, 4 side images and 1
         * image for the inside.
         */
        void drawImageRect(int x, int y, int w, int h, const ImageRect &imgRect);

        /**
         * Updates the screen. This is done by either copying the buffer to the
         * screen or swapping pages.
         */
        virtual void updateScreen();

        /**
         * Takes a screenshot and returns it as SDL surface.
         */
        virtual SDL_Surface* getScreenshot();
        // Inherited from Graphics

        virtual void _beginDraw();

        virtual void _endDraw();

};

#endif
