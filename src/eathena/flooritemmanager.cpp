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

#include "flooritemmanager.h"

#include "../core/map/sprite/flooritem.h"

#include "../core/utils/dtor.h"

FloorItemManager::~FloorItemManager()
{
    clear();
}

FloorItem* FloorItemManager::create(const int id, const int itemId,
                                    const int x, const int y, Map *map)
{
    FloorItem *floorItem = new FloorItem(id, itemId, x, y, map);
    mFloorItems.push_back(floorItem);
    return floorItem;
}

void FloorItemManager::destroy(FloorItem *item)
{
    mFloorItems.remove(item);

    // TODO: Find out why this is an unsafe destroy and fix it
    delete item;
}

void FloorItemManager::clear()
{
    delete_all(mFloorItems);
    mFloorItems.clear();
}

FloorItem *FloorItemManager::findById(const int id)
{
    FloorItemIterator i;
    for (i = mFloorItems.begin(); i != mFloorItems.end(); i++)
    {
        if ((*i)->getId() == id)
            return *i;
    }

    return NULL;
}

FloorItem *FloorItemManager::findByCoordinates(const int x, const int y)
{
    FloorItemIterator i;
    for (i = mFloorItems.begin(); i != mFloorItems.end(); i++)
    {
        if ((*i)->getX() == x && (*i)->getY() == y)
            return *i;
    }

    return NULL;
}

FloorItem *FloorItemManager::findNearestItem(const int x, const int y,
                                             int maxdist)
{
    FloorItem *closestItem = NULL;
    int dist = 0;

    FloorItemIterator i;
    for (i = mFloorItems.begin(); i != mFloorItems.end(); i++)
    {
        FloorItem *item = (*i);
        int d = std::max(abs(item->getX() - x), abs(item->getY() - y));

        if (d < maxdist && (d < dist || closestItem == NULL))
        {
            dist = d;
            closestItem = item;
        }
    }

    return closestItem;
}
