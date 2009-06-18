/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *  Copyright (C) 2009  The Aethyra Development Team
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

#include "itemdb.h"
#include "iteminfo.h"

const std::string& ItemInfo::getSprite(const Gender &gender) const
{
    if (mView)
    {
        // Forward the request to the item defining how to view this item
        return ItemDB::get(mView).getSprite(gender);
    }
    else
    {
        static const std::string empty = "";
        std::map<int, std::string>::const_iterator i =
            mAnimationFiles.find(gender);

        return (i != mAnimationFiles.end()) ? i->second : empty;
    }
}

void ItemInfo::setType(const std::string& type)
{
    mType = type;

    // All of this will be replaced by getting a
    // single int from the server.  The only part
    // of the client that should know the data in
    // this enum is the equipment window (for
    // putting the icons in the right place).
    enum {
        // Equipment rules:
        EQUIP_LEGS_SLOT = 0,
        EQUIP_FIGHT1_SLOT,
        EQUIP_GLOVES_SLOT,
        EQUIP_RING2_SLOT,
        EQUIP_RING1_SLOT,
        EQUIP_FIGHT2_SLOT,
        EQUIP_FEET_SLOT,
        EQUIP_CAPE_SLOT,
        EQUIP_HEAD_SLOT,
        EQUIP_TORSO_SLOT,
        EQUIP_AMMO_SLOT,
        EQUIP_VECTOREND
    };

    if (type == "equip-legs")
        mEquipSlots = 1 << EQUIP_LEGS_SLOT;
    else if (type == "equip-1hand")
        mEquipSlots = 1 << EQUIP_FIGHT1_SLOT;
    else if (type == "equip-arms")
        mEquipSlots = 1 << EQUIP_GLOVES_SLOT;
    else if (type == "equip-shield")
        mEquipSlots = 1 << EQUIP_FIGHT2_SLOT;
    else if (type == "equip-feet")
        mEquipSlots = 1 << EQUIP_FEET_SLOT;
    else if (type == "equip-cape")
        mEquipSlots = 1 << EQUIP_CAPE_SLOT;
    else if (type == "equip-head")
        mEquipSlots = 1 << EQUIP_HEAD_SLOT;
    else if (type == "equip-torso")
        mEquipSlots = 1 << EQUIP_TORSO_SLOT;
    else if (type == "equip-ring")
        mEquipSlots = (1 << EQUIP_RING1_SLOT)
                    | (1 << EQUIP_RING2_SLOT);
    else if (type == "equip-2hand")
        mEquipSlots = (1 << EQUIP_FIGHT1_SLOT)
                    | (1 << EQUIP_FIGHT2_SLOT);
    else if (type == "equip-ammo")
        mEquipSlots = 1 << EQUIP_AMMO_SLOT;
    else
        mEquipSlots = 0;
}

void ItemInfo::setWeaponType(const int &type)
{
    // See server item.hpp file for type values.
    switch (type)
    {
        case 0:     // none
            mAttackType = ACTION_DEFAULT;
            break;
        case 1:     // knife
        case 2:     // sword
            mAttackType = ACTION_ATTACK_STAB;
            break;
        case 8:     // projectile
            mAttackType = ACTION_ATTACK_THROW;
            break;
        case 10:    // bow
            mAttackType = ACTION_ATTACK_BOW;
            break;
        case 11:    // sickle
            mAttackType = ACTION_ATTACK_SWING;
            break;
        default:
            mAttackType = ACTION_ATTACK;
    }
}

void ItemInfo::addSound(const EquipmentSoundEvent &event,
                        const std::string &filename)
{
    mSounds[event].push_back("sfx/" + filename);
}

const std::string& ItemInfo::getSound(const EquipmentSoundEvent &event) const
{
    static const std::string empty;
    std::map< EquipmentSoundEvent, std::vector<std::string> >::const_iterator i;
    i = mSounds.find(event);

    return i == mSounds.end() ? empty : i->second[rand() % i->second.size()];
}
