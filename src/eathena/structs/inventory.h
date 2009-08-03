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

#ifndef INVENTORY_H
#define INVENTORY_H

class Item;

#define INVENTORY_SIZE 102
#define STORAGE_SIZE 301
#define NO_SLOT_INDEX -1 /**< Slot has no index. */

/**
 * A collection of Items.
 *
 * The main instances of this class are the character's carried items
 * and storage (accessed through LocalPlayer::getInventory() and
 * LocalPlayer::getStorage() respectively).  These are a local mirror of
 * the character's inventory on the server; the order of items is
 * important, as many of the messages between client and server identify
 * items simply by their index in the inventory.  The server can
 * rearrange the items (typically at map-change).
 *
 * Other instances are used for trading.
 *
 * In the current design, GUI items that display the inventory redraw
 * continually, and draw the inventory's current state; thus they don't
 * need notification when it changes.
 */
class Inventory
{
    public:
        /**
         * Constructor.
         */
        Inventory(const int size);

        /**
         * Destructor.
         */
        ~Inventory();

        /**
         * Returns the size that this instance is configured for
         */
        const int getSize() const { return mSize; }

        /**
         * Returns the item at the specified index.
         *
         * This bounds-checks, and also checks that (quantity > 0),
         * returning NULL in either case.
         */
        Item* getItem(const int index) const;

        /**
         * Searches for the specified item by it's id.
         *
         * @param itemId The id of the item to be searched.
         * @return Item found on success, NULL on failure.
         */
        Item* findItem(const int itemId) const;

        /**
         * Adds a new item in a free slot.
         */
        void addItem(const int id, const int quantity, const bool equipment);

        /**
         * Sets the item at the given position.
         */
        void setItem(int index, int id, int quantity, bool equipment);

        /**
         * Remove a item from the inventory.
         */
        void removeItem(const int id);

        /**
         * Remove the item at the specified index from the inventory.
         */
        void removeItemAt(const int index);

        /**
         * Checks if the given item is in the inventory
         */
        const bool contains(const Item *item) const;

        /**
         * Returns id of next free slot or -1 if all occupied.
         */
        const int getFreeSlot() const;

        /**
         * Reset all item slots.
         */
        void clear();

        /**
         * Get the number of slots filled with an item
         */
        const int getNumberOfSlotsUsed() const;

        /**
         * Returns the index of the last occupied slot or 0 if none occupied.
         */
        const int getLastUsedSlot() const;
    protected:
        Item **mItems;  /**< The holder of items */
        int mSize;      /**< The max number of inventory items */
};

#endif
