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

#include <algorithm>

#include "inventory.h"
#include "item.h"

#include "../../core/log.h"

struct SlotUsed : public std::unary_function<Item*, bool>
{
    bool operator()(const Item *item) const
    {
        return item && item->getId() != NO_SLOT_INDEX && item->getQuantity() > 0;
    }
};

Inventory::Inventory(const int size):
    mSize(size)
{
    mItems = new Item*[mSize];
    std::fill_n(mItems, mSize, (Item*) NULL);
}

Inventory::~Inventory()
{
    for (int i = 0; i < mSize; i++)
        delete mItems[i];

    delete [] mItems;
}

Item* Inventory::getItem(const int index) const
{
    if (index < 0 || index >= mSize || !mItems[index] ||
        mItems[index]->getQuantity() <= 0)
        return 0;

    return mItems[index];
}

Item* Inventory::findItem(const int itemId) const
{
    for (int i = 0; i < mSize; i++)
        if (mItems[i] && mItems[i]->getId() == itemId)
            return mItems[i];

    return NULL;
}

void Inventory::addItem(const int id, const int quantity, const bool equipment)
{
    setItem(getFreeSlot(), id, quantity, equipment);
}

void Inventory::setItem(const int index, const int id, const int quantity,
                        const bool equipment)
{
    if (index < 0 || index >= mSize)
    {
        logger->log("Warning: invalid inventory index: %d", index);
        return;
    }

    if (!mItems[index] && id > 0)
    {
        Item *item = new Item(id, quantity, equipment);
        item->setInvIndex(index);
        mItems[index] = item;
    }
    else if (id > 0)
    {
        mItems[index]->setId(id);
        mItems[index]->setQuantity(quantity);
        mItems[index]->setEquipment(equipment);
    }
    else if (mItems[index])
    {
        removeItemAt(index);
    }
}

void Inventory::clear()
{
    for (int i = 0; i < mSize; i++)
        removeItemAt(i);
}

void Inventory::removeItem(const int id)
{
    for (int i = 0; i < mSize; i++)
        if (mItems[i] && mItems[i]->getId() == id)
            removeItemAt(i);
}

void Inventory::removeItemAt(const int index)
{
    delete mItems[index];
    mItems[index] = 0;
}

const bool Inventory::contains(const Item *item) const
{
    for (int i = 0; i < mSize; i++)
        if (mItems[i] && mItems[i]->getId() == item->getId())
            return true;

    return false;
}

const int Inventory::getFreeSlot() const
{
    Item **i = std::find_if(mItems, mItems + mSize, std::not1(SlotUsed()));
    return (i == mItems + mSize) ? NO_SLOT_INDEX : (i - mItems);
}

const int Inventory::getNumberOfSlotsUsed() const
{
    return count_if(mItems, mItems + mSize, SlotUsed());
}

const int Inventory::getLastUsedSlot() const
{
    for (int i = mSize - 1; i >= 0; i--)
        if (SlotUsed()(mItems[i]))
            return i;

    return NO_SLOT_INDEX;
}
