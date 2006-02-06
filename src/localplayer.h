/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#ifndef _TMW_LOCALPLAYER_H
#define _TMW_LOCALPLAYER_H

#include "player.h"

// TODO move into some sane place...
#define MAX_SLOT 2

class Equipment;
class FloorItem;
class Inventory;
class Item;
class Network;

class LocalPlayer : public Player
{
    public:
        enum Attribute {
            STR = 0, AGI, VIT, INT, DEX, LUK
        };

        LocalPlayer(Uint32 id, Uint16 job, Map *map);

        virtual ~LocalPlayer();

        void setNetwork(Network *network) { mNetwork = network; }

        virtual void logic();
        virtual void nextStep();

        /**
         * Draws the name text below the being.
         */
        virtual void
        drawName(Graphics *graphics, Sint32 offsetX, Sint32 offsetY) {};

        virtual Type getType() const;

        void clearInventory();
        void addInvItem(int id, int quantity, bool equipment);
        void addInvItem(int index, int id, int quantity, bool equipment);
        Item* getInvItem(int index);

        /**
         * Equips an item.
         */
        void equipItem(Item *item);

        /**
         * Unequips an item.
         */
        void unequipItem(Item *item);

        void useItem(Item *item);
        void dropItem(Item *item, int quantity);
        void pickUp(FloorItem *item);

        /**
         * Sents a trade request to the given being.
         */
        void trade(Being *being) const;

        /**
         * Accept or decline a trade offer
         */
        void tradeReply(bool accept);

        /**
         * Returns true when the player is ready to accept a trade offer.
         * Returns false otherwise.
         */
        bool tradeRequestOk() const;

        /**
         * Sets the trading state of the player, i.e. whether or not he is
         * currently involved into some trade.
         */
        void setTrading(bool trading) { mTrading = trading; };

        void attack(Being *target=NULL, bool keep=false);
        void stopAttack();
        Being* getTarget() const;

        void walk(Being::Direction dir);

        /**
         * Sets a new destination for this being to walk to.
         */
        virtual void setDestination(Uint16 x, Uint16 y);

        void raiseAttribute(Attribute attr);
        void raiseSkill(Uint16 skillId);

        void toggleSit();
        void emote(Uint8 emotion);

        void revive();

        Uint32 mCharId;

        Uint32 xp, jobXp;
        Uint16 lvl;
        Uint32 jobLvl;
        Uint32 xpForNextLevel, jobXpForNextLevel;
        Uint16 hp, maxHp, mp, maxMp;
        Uint32 gp;

        Uint32 totalWeight, maxWeight;

        Uint8 ATTR[6];
        Uint8 ATTR_UP[6];

        Sint16 ATK, MATK, DEF, MDEF, HIT, FLEE;
        Sint16 ATK_BONUS, MATK_BONUS, DEF_BONUS, MDEF_BONUS, FLEE_BONUS;

        Uint16 statPoint, skillPoint;
        Uint16 statsPointsToAttribute;

        float lastAttackTime; /**< Used to synchronize the charge dialog */

        Inventory *mInventory;
        Equipment *mEquipment;

    protected:
        Network *mNetwork;
        Being *mTarget;
        FloorItem *mPickUpTarget;

        bool mTrading;
};

extern LocalPlayer *player_node;

#endif
