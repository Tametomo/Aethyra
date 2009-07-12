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

#include "shopitem.h"

#include "../../core/utils/stringutils.h"

ShopItem::ShopItem (const int &inventoryIndex, const int &id,
                    const int &quantity, const int &price) :
    Item (id, 0),
    mPrice(price)
{
    mDisplayName = getInfo().getName() + " (" + toString(mPrice) + " GP)";
    setInvIndex(inventoryIndex);
    addDuplicate(inventoryIndex, quantity);
}

ShopItem::ShopItem (const int &id, const int &price):
    Item (id, 0),
    mPrice(price)
{
    mDisplayName = getInfo().getName() + " (" + toString(mPrice) + " GP)";
    setInvIndex(-1);
    addDuplicate(-1, 0);
}

ShopItem::~ShopItem()
{
    /** Clear all remaining duplicates on Object destruction. */
    while (!mDuplicates.empty())
    {
        delete mDuplicates.top();
        mDuplicates.pop();
    }
}

void ShopItem::addDuplicate(const int &inventoryIndex, const int &quantity)
{
    DuplicateItem* di = new DuplicateItem;
    di->inventoryIndex = inventoryIndex;
    di->quantity = quantity;
    mDuplicates.push(di);
    mQuantity += quantity;
}


void ShopItem::addDuplicate()
{
    DuplicateItem* di = new DuplicateItem;
    di->inventoryIndex = -1;
    di->quantity = 0;
    mDuplicates.push(di);
}

int ShopItem::sellCurrentDuplicate(const int &quantity)
{
    DuplicateItem* dupl = mDuplicates.top();
    int sellCount = quantity <= dupl->quantity ? quantity : dupl->quantity;
    dupl->quantity -= sellCount;
    mQuantity -= sellCount;
    if (dupl->quantity == 0)
    {
        delete dupl;
        mDuplicates.pop();
    }
    return sellCount;
}

