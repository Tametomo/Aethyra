/*
 *  Aethyra
 *  Copyright 2007 The Mana World Development Team
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

#ifndef SHORTCUTCONTAINER_H__
#define SHORTCUTCONTAINER_H__

#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>

#include "../handlers/shortcuthandler.h"

class Image;

/**
 * A generic shortcut container.
 *
 * \ingroup GUI
 */
class ShortcutContainer : public gcn::Widget,
                          public gcn::WidgetListener,
                          public gcn::MouseListener
{
    public:
        /**
         * Constructor. Initializes the shortcut container.
         */
        ShortcutContainer(ShortcutHandler *shortcut);

        /**
         * Destructor.
         */
        ~ShortcutContainer();

        /**
         * Draws the shortcuts
         */
        virtual void draw(gcn::Graphics *graphics) = 0;

        /**
         * Invoked when a widget changes its size. This is used to determine
         * the new height of the container.
         */
        virtual void widgetResized(const gcn::Event &event);

        /**
         * Handles mouse when dragged.
         */
        virtual void mouseDragged(gcn::MouseEvent &event);

        /**
         * Handles mouse when pressed.
         */
        virtual void mousePressed(gcn::MouseEvent &event) = 0;

        /**
         * Handles mouse release.
         */
        virtual void mouseReleased(gcn::MouseEvent &event);

        int getMaxShortcuts() const { return SHORTCUTS; }

        int getBoxWidth() const { return mBoxWidth; }

        int getBoxHeight() const { return mBoxHeight; }

        ShortcutHandler *getShortcutHandler() { return mShortcutHandler; }

    protected:
        /**
         * Gets the index from the grid provided the point is in an item box.
         *
         * @param pointX X coordinate of the point.
         * @param pointY Y coordinate of the point.
         * @return index on success, -1 on failure.
         */
        int getIndexFromGrid(int pointX, int pointY) const;

        static Image *mBackgroundImg;

        static float mAlpha;

        static int mBoxWidth;
        static int mBoxHeight;

        ShortcutHandler *mShortcutHandler;

        int mCursorPosX, mCursorPosY;
        int mGridWidth, mGridHeight;

        bool mShortcutClicked;
        bool mShortcutDragged;

    private:
        static int mInstances;
};

#endif
