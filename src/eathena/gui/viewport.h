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

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <guichan/mouselistener.hpp>

#include "../../bindings/guichan/widgets/container.h"

#include "../../core/map/position.h"

class Being;
class FloorItem;
class Graphics;
class ImageSet;
class Item;
class Map;
class PopupMenu;

/**
 * The viewport on the map. Displays the current map and handles mouse input
 * and the popup menu.
 *
 * TODO: This class is planned to be extended to allow floating widgets on top
 * of it such as NPC messages, which are positioned using map pixel
 * coordinates.
 */
class Viewport : public Container, public gcn::MouseListener
{
    public:
        /**
         * Constructor.
         */
        Viewport();

        /**
         * Destructor.
         */
        ~Viewport();

        /**
         * Gets the currently displayed map by the viewport.
         */
        Map *getMap() { return mCurrentMap; }

        /**
         * Returns the map name of the current loaded map.
         */
        std::string getMapName() { return mMapName; }

        /**
         * Returns the full map path for the current map.
         */
        std::string getMapPath();

        /**
         * Draws the viewport.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Implements player to keep following mouse.
         */
        void logic();

        /**
         * Toggles whether the path debug graphics are shown
         */
        void toggleDebugPath() { mShowDebugPath = !mShowDebugPath; }

        /**
         * Handles mouse press on map.
         */
        void mousePressed(gcn::MouseEvent &event);

        /**
         * Handles mouse move on map
         */
        void mouseDragged(gcn::MouseEvent &event);

        /**
         * Handles mouse button release on map.
         */
        void mouseReleased(gcn::MouseEvent &event);

        /**
         * Shows a popup for an item.
         * TODO Find some way to get rid of Item here
         */
        void showPopup(int x, int y, Item *item);

        /**
         * Shows a popup for a being.
         * TODO Find some way to combine this with the function above
         */
        void showPopup(int x, int y, Being *being);

        /**
         * Closes the popup menu. Needed for when the player dies or switching
         * maps.
         */
        void closePopupMenu();

        /**
         * Returns camera x offset in pixels.
         */
        int getCameraX() const { return (int) mPixelViewX; }

        /**
         * Returns camera y offset in pixels.
         */
        int getCameraY() const { return (int) mPixelViewY; }

        /**
         * Changes viewpoint by relative pixel coordinates.
         */
        void scrollBy(float x, float y) { mPixelViewX += x; mPixelViewY += y; }

        /**
         * Sets the currently active map.
         */
        bool changeMap(const std::string &mapName);

    private:
        /**
         * Sets the map displayed by the viewport.
         */
        void setMap(Map *map);

        Map *mCurrentMap;            /**< The current map. */
        std::string mMapName;

        int mLastTick;
        float mScrollRadius;
        float mScrollLaziness;
        float mScrollWidthOffset;    /**< In # of tiles */
        float mScrollHeightOffset;   /**< In # of tiles */
        float mPixelViewX;           /**< Current viewpoint in pixels. */
        float mPixelViewY;           /**< Current viewpoint in pixels. */
        int mTileViewX;              /**< Current viewpoint in tiles. */
        int mTileViewY;              /**< Current viewpoint in tiles. */
        bool mShowDebugPath;         /**< Show a path from player to pointer. */

        bool mPlayerFollowMouse;
        int mWalkTime;

        PopupMenu *mPopupMenu;       /**< Popup menu. */
};

extern Viewport *viewport;           /**< The viewport */

#endif
