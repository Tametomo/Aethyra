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

#ifndef POPUP_MENU_H
#define POPUP_MENU_H

#include "../bindings/guichan/handlers/linkhandler.h"
#include "../bindings/guichan/widgets/popup.h"

class Being;
class BrowserBox;
class FloorItem;
class Item;

enum MenuType {
    FLOOR_ITEM,
    TRADE,
    INVENTORY,
    STORAGE,
    BEING,
    UNKNOWN
};

/**
 * Window showing popup menu.
 */
class PopupMenu : public Popup, public LinkHandler
{
    public:
        /**
         * Constructor.
         */
        PopupMenu(MenuType type = UNKNOWN);

        /**
         * Shows a popup menu at the specified mouse coords.
         */
        void showPopup(int x, int y);

        /**
         * Handles link action.
         */
        void handleLink(const std::string& link);

        /**
         * Changes the type of popups this PopupMenu handles.
         */
        void setType(MenuType type) { mType = type; }

        /**
         * Sets the internal floor item to use.
         */
        void setFloorItem(FloorItem *item) { mFloorItem = item; }

        /**
         * Sets the internal item to use.
         */
        void setItem(Item *item) { mItem = item; }

        /**
         * Sets the internal being to use.
         */
        void setBeing(Being *being) { mBeing = being; }

    private:
        BrowserBox* mBrowserBox;

        Being *mBeing;
        FloorItem *mFloorItem;
        Item *mItem;
        MenuType mType;
};

#endif
