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

#ifndef ITEM_H
#define ITEM_H

#include "../db/itemdb.h"

class Image;

/**
 * Represents one or more instances of a certain item type.
 */
class Item
{
    public:
        /**
         * Constructor.
         */
        Item(const int id = -1, const int quantity = 0,
             const bool equipment = false, const bool equipped = false);

        /**
         * Destructor.
         */
        ~Item();

        /**
         * Sets the item id, identifying the item type.
         */
        void setId(const int id);

        /**
         * Returns the item id.
         */
        const int getId() const { return mId; }

        /**
         * Returns the item image.
         */
        Image* getImage() const { return mImage; }

        /**
         * Sets the number of items.
         */
        void setQuantity(const int quantity) { mQuantity = quantity; }

        /**
         * Increases the number of items by the given amount.
         */
        void increaseQuantity(const int amount) { mQuantity += amount; }

        /**
         * Returns the number of items.
         */
        const int getQuantity() const { return mQuantity; }

        /**
         * Sets whether this item is considered equipment.
         */
        void setEquipment(const bool equipment) { mEquipment = equipment; }

        /**
         * Returns whether this item is considered equipment.
         */
        const bool isEquipment() const { return mEquipment; }

        /**
         * Sets whether this item is equipped.
         */
        void setEquipped(const bool equipped) { mEquipped = equipped; }

        /**
         * Returns whether this item is equipped.
         */
        const bool isEquipped() const { return mEquipped; }

        /**
         * Sets the inventory index of this item.
         */
        void setInvIndex(const int index) { mInvIndex = index; }

        /**
         * Returns the inventory index of this item.
         */
        const int getInvIndex() const { return mInvIndex; }

        /**
         * Returns information about this item type.
         */
        const ItemInfo& getInfo() const { return ItemDB::get(mId); }

        /**
         * Returns the item's name.
         */
        const std::string getName() const { return ItemDB::get(mId).getName(); }

    protected:
        int mId;              /**< Item type id. */
        Image *mImage;        /**< Item image. */
        int mQuantity;        /**< Number of items. */
        bool mEquipment;      /**< Item is equipment. */
        bool mEquipped;       /**< Item is equipped. */
        int mInvIndex;        /**< Inventory index. */
};

#endif
