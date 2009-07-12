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

#include <guichan/actionlistener.hpp>

#include "../../bindings/guichan/widgets/popup.h"
#include "../../bindings/guichan/widgets/windowcontainer.h"

class Being;
class FloorItem;
class Item;
class LinkMappedListModel;
class MappedListBox;

enum MenuType {
    ITEM_SHORTCUT,
    INVENTORY,
    TRADE,
    STORAGE,
    FLOOR_ITEM,
    BEING,
    EMOTE,
    UNKNOWN
};

/**
 * Window showing popup menu.
 */
class PopupMenu : public Popup, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        PopupMenu(MenuType type = UNKNOWN, gcn::Container *parent = windowContainer);

        /**
         * Shows a popup menu at the specified mouse coords.
         */
        void showPopup(int x, int y);

        /**
         * Handles link action.
         */
        void action(const gcn::ActionEvent &event);

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

        /**
         * Sets the internal emote id to use.
         */
        void setEmote(int emote) { mEmote = emote; }

        /**
         * Overloaded requestFocus(), in order to get the mappedListBox to
         * take focus, as well as keeping track of what should gain focus on
         * losing focus.
         */
        void requestFocus();

    private:
        gcn::Widget *mPreviousFocus;

        MappedListBox* mMappedListBox;
        LinkMappedListModel* mModel;

        Being *mBeing;
        FloorItem *mFloorItem;
        Item *mItem;
        int mEmote;
        MenuType mType;
};

#endif
