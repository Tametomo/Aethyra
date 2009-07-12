/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra derived from original code from The Mana World.
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

#ifndef ITEMSHORTCUT_H
#define ITEMSHORTCUT_H

#include "../structs/inventory.h"
#include "../structs/item.h"

#include "../../bindings/guichan/handlers/shortcuthandler.h"

#include "../../core/image/sprite/localplayer.h"

/**
 * A handler which keeps track of item shortcuts.
 */
class ItemShortcut : public ShortcutHandler
{
    public:
        /**
         * Constructor.
         */
        ItemShortcut() : ShortcutHandler("shortcut") {}

        /**
         * Try to use the item specified by the index.
         *
         * @param index Index of the item.
         */
        void useShortcut(int index)
        {
            if (index > -1 && index <= SHORTCUTS)
            {
                Item *item = player_node->getInventory()->findItem(mIndex[index]);
                if (item && item->getQuantity())
                {
                    if (item->isEquipment())
                    {
                        if (item->isEquipped())
                            player_node->unequipItem(item);
                        else
                            player_node->equipItem(item);
                    }
                    else
                        player_node->useItem(item);
                }
            }
        }
};

extern ItemShortcut *itemShortcut;

#endif
