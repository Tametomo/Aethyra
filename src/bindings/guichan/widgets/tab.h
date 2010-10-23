/*
 *  Aethyra
 *  Copyright (C) 2008  The Mana World Development Team
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

#ifndef TAB_H
#define TAB_H

#include <guichan/basiccontainer.hpp>
#include <guichan/mouselistener.hpp>

#include "label.h"

class Image;
class ImageRect;
class TabbedArea;
class TabConfigListener;

/**
 * A tab, the same as the guichan tab in 0.8, but extended to allow
 * transparancy, and to use our TabbedArea
 */
class Tab : public gcn::BasicContainer, public gcn::MouseListener
{
    public:
        friend class TabConfigListener;

        /**
         * Constructor.
         */
        Tab(const std::string &caption = "", bool closeable = false);

        /**
         * Destructor.
         */
        virtual ~Tab();

        /**
         * Adjusts the size of the tab to fit the caption. If this tab was
         * added to a TabbedArea, it will also adjust the tab positions.
         */
        void adjustSize();

        /**
         * Sets the tabbed area for the tab.
         */
        void setTabbedArea(TabbedArea* tabbedArea) { mTabbedArea = tabbedArea; }

        /**
         * Gets the tabbed area the tab is a part of.
         */
        TabbedArea* getTabbedArea() const { return mTabbedArea; }

        /**
         * Sets the caption of the tab.
         */
        void setCaption(const std::string& caption);

        /**
         * Gets the caption of the tab.
         */
        const std::string& getCaption() const { return mLabel->getCaption(); }

        /**
         * Draw the tabbed area.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Changes the color the default tab text uses.
         */
        void setTabColor(const gcn::Color *color) { mTabColor = color; }

        /**
         * Set tab highlighted
         */
        void setHighlighted(bool high) { mHighlighted = high; }

        bool isHighlighted() const { return mHighlighted; }

        void setCloseable(bool close);

        bool isCloseable() const { return mCloseable; }

        void fontChanged();

        /**
         * Removes a tab from a TabbedArea
         *
         * NOTE: If the TabbedArea didn't create this tab, you are responsible
         *       for its destruction.
         */
        void close();

        // Inherited from MouseListener

        virtual void mouseEntered(gcn::MouseEvent& mouseEvent) { mHasMouse = true; }
        virtual void mouseExited(gcn::MouseEvent& mouseEvent) { mHasMouse = false; }
        virtual void mousePressed(gcn::MouseEvent& mouseEvent);

    protected:
        static float mAlpha;
        static TabConfigListener *mConfigListener;

    private:
        static ImageRect tabImg[4];    /**< Tab state graphics */
        static Image *mCloseButton;    /**< Tab close graphic */
        static int mInstances;         /**< Number of tab instances */

        const gcn::Color *mTabColor;
        bool mCloseable;
        bool mHighlighted;
        bool mHasMouse;

        gcn::Label* mLabel;
        TabbedArea* mTabbedArea;
};

#endif


