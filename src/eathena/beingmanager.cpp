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

#include "beingmanager.h"

#include "net/messageout.h"
#include "net/protocol.h"

#include "../core/map/sprite/localplayer.h"
#include "../core/map/sprite/monster.h"
#include "../core/map/sprite/npc.h"
#include "../core/map/sprite/player.h"

#include "../core/utils/dtor.h"

class FindBeingFunctor
{
    public:
        bool operator() (Being *being)
        {
            uint16_t other_y = y + ((being->getType() == Being::NPC) ? 1 : 0);
            return (being->mX == x &&
                    (being->mY == y || being->mY == other_y) &&
                    being->mAction != Being::DEAD &&
                    (type == Being::UNKNOWN || being->getType() == type));
        }

        uint16_t x, y;
        Being::Type type;
} beingFinder;

BeingManager::BeingManager()
{
}

BeingManager::~BeingManager()
{
    clear();
}

void BeingManager::refreshParticleEffects() const
{
    for (Beings::const_iterator i = mBeings.begin(), i_end = mBeings.end();
         i != i_end; ++i)
    {
        (*i)->refreshParticleEffects();
    }
}

void BeingManager::setMap(Map *map)
{
    mMap = map;
    if (player_node)
        player_node->setMap(map);
}

void BeingManager::setPlayer(LocalPlayer *player)
{
    player_node = player;
    mBeings.push_back(player);
}

Being *BeingManager::createBeing(int id, uint16_t job)
{
    Being *being;

    if (job <= 25 || (job >= 4001 && job <= 4049))
        being = new Player(id, job, mMap);
    else if (job >= 46 && job <= 1000)
        being = new NPC(id, job, mMap);
    else if (job > 1000 && job <= 2000)
        being = new Monster(id, job, mMap);
    else
        being = new Being(id, job, mMap);

    // Player or NPC
    if (job <= 1000 || (job >= 4001 && job <= 4049))
    {
        MessageOut outMsg(0x0094);
        outMsg.writeInt32(id);//readLong(2));
    }

    mBeings.push_back(being);

    return being;
}

void BeingManager::destroyBeing(Being *being)
{
    mBeings.remove(being);
    destroy(being);
}

Being *BeingManager::findBeing(int id) const
{
    for (Beings::const_iterator i = mBeings.begin(), i_end = mBeings.end();
         i != i_end; ++i)
    {
        Being *being = (*i);

        if (being->getId() == id)
            return being;
    }
    return NULL;
}

Being *BeingManager::findBeing(int x, int y, Being::Type type) const
{
    beingFinder.x = x;
    beingFinder.y = y;
    beingFinder.type = type;

    Beings::const_iterator i = find_if(mBeings.begin(), mBeings.end(),
                                       beingFinder);

    return (i == mBeings.end()) ? NULL : *i;
}

Being *BeingManager::findBeingByPixel(int x, int y) const
{
    Beings::const_iterator itr = mBeings.begin();
    Beings::const_iterator itr_end = mBeings.end();

    for (; itr != itr_end; ++itr)
    {
        Being *being = (*itr);

        int xtol = being->getWidth();
        int uptol = being->getHeight() / 2;

        if ((being->mAction != Being::DEAD) &&
            (being != player_node) &&
            (being->getPixelX() <= x) &&
            (being->getPixelX() + xtol >= x) &&
            (being->getPixelY() - uptol <= y) &&
            (being->getPixelY() + uptol >= y))
        {
            return being;
        }
    }

    return NULL;
}

Being *BeingManager::findBeingByName(const std::string &name,
                                     Being::Type type) const
{
    for (Beings::const_iterator i = mBeings.begin(), i_end = mBeings.end();
         i != i_end; ++i)
    {
        Being *being = (*i);
        if (being->getName() == name &&
           (type == Being::UNKNOWN || type == being->getType()))
            return being;
    }
    return NULL;
}

const Beings &BeingManager::getAll() const
{
    return mBeings;
}

void BeingManager::logic()
{
    Beings::iterator i = mBeings.begin();
    while (i != mBeings.end())
    {
        Being *being = (*i);

        being->logic();

        if (being->mAction == Being::DEAD && being->mFrame >= 20)
        {
            destroy(being);
            i = mBeings.erase(i);
        }
        else
            ++i;
    }
}

void BeingManager::clear()
{
    if (player_node)
        mBeings.remove(player_node);

    delete_all(mBeings);
    mBeings.clear();

    if (player_node)
        mBeings.push_back(player_node);
}

Being *BeingManager::findNearestLivingBeing(int x, int y, int maxdist,
                                            Being::Type type) const
{
    Being *closestBeing = NULL;
    int dist = 0;

    Beings::const_iterator itr = mBeings.begin();
    Beings::const_iterator itr_end = mBeings.end();

    for (; itr != itr_end; ++itr)
    {
        Being *being = (*itr);
        int d = std::max(abs(being->mX - x), abs(being->mY - y));

        if ((being->getType() == type || (type == Being::UNKNOWN &&
             being->getType() != Being::WARP))
                && (d < dist || closestBeing == NULL)   // it is closer
                && being->mAction != Being::DEAD        // no dead beings
                && being != player_node)                // it is not you
        {
            dist = d;
            closestBeing = being;
        }
    }

    return (maxdist >= dist) ? closestBeing : NULL;
}

Being *BeingManager::findNearestLivingBeing(Being *aroundBeing, int maxdist,
                                            Being::Type type) const
{
    Being *closestBeing = NULL;
    int dist = 0;
    int x = aroundBeing->mX;
    int y = aroundBeing->mY;

    for (Beings::const_iterator i = mBeings.begin(), i_end = mBeings.end();
         i != i_end; ++i)
    {
        Being *being = (*i);
        int d = std::max(abs(being->mX - x), abs(being->mY - y));

        if ((being->getType() == type || type == Being::UNKNOWN)
                && (d < dist || closestBeing == NULL)   // it is closer
                && being->mAction != Being::DEAD        // no dead beings
                && being != aroundBeing)
        {
            dist = d;
            closestBeing = being;
        }
    }

    return (maxdist >= dist) ? closestBeing : NULL;
}

bool BeingManager::hasBeing(Being *being)
{
    for (Beings::const_iterator i = mBeings.begin(), i_end = mBeings.end();
         i != i_end; ++i)
    {
        if (being == *i)
            return true;
    }

    return false;
}
