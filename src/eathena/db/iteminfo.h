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

#ifndef ITEMINFO_H
#define ITEMINFO_H

#include <map>
#include <string>
#include <vector>

#include "../../core/image/sprite/spritedef.h"

#include "../../core/image/sprite/being.h"

enum EquipmentSoundEvent
{
    EQUIP_EVENT_STRIKE,
    EQUIP_EVENT_HIT
};

/**
 * Defines a class for storing item infos. This includes information used when
 * the item is equipped.
 */
class ItemInfo
{
    public:
        /**
         * Constructor.
         */
        ItemInfo():
            mType(""),
            mWeight(0),
            mView(0),
            mAttackType(ACTION_DEFAULT)
        {
        }

        void setId(const int &id) { mId = id; }

        const int &getId() const { return mId; }

        void setName(const std::string &name) { mName = name; }

        const std::string& getName() const { return mName; }

        void setParticleEffect(const std::string &particleEffect)
        { mParticle = particleEffect; }

        const std::string &getParticleEffect() const { return mParticle; }

        void setImageName(const std::string &imageName)
        { mImageName = imageName; }

        const std::string& getImageName() const { return mImageName; }

        void setDescription(const std::string &description)
        { mDescription = description; }

        const std::string& getDescription() const { return mDescription; }

        void setEffect(const std::string &effect) { mEffect = effect; }

        const std::string& getEffect() const { return mEffect; }

        void setType(const std::string& type);

        const std::string& getType() const { return mType; }

        void setWeight(const short &weight) { mWeight = weight; }

        const short &getWeight() const { return mWeight; }

        void setView(const int &view) { mView = view; }

        void setSprite(const std::string &animationFile, const Gender &gender)
        { mAnimationFiles[gender] = animationFile; }

        const std::string& getSprite(const Gender &gender) const;

        void setWeaponType(const int&);

        const SpriteAction &getAttackType() const { return mAttackType; }

        void addSound(const EquipmentSoundEvent &event,
                      const std::string &filename);

        const std::string &getSound(const EquipmentSoundEvent &event) const;

        /**
         * A bitmask of which equipment slots will be used by this item.
         * If the item is not equippable, it returns 0.
         */
        int getEquipSlots() const { return mEquipSlots; }

    protected:
        std::string mImageName;      /**< The filename of the icon image. */
        std::string mName;
        std::string mDescription;    /**< Short description. */
        std::string mEffect;         /**< Description of effects. */
        std::string mType;           /**< Item type. */
        std::string mParticle;       /**< Particle effect used with this item */
        short mWeight;               /**< Weight in grams. */
        int mView;                   /**< Item ID of how this item looks. */
        int mId;                     /**< Item ID */
        int mEquipSlots;             /**< Which slots this item gets equipped in (0 for non-equipment) */

        // Equipment related members
        SpriteAction mAttackType;    /**< Attack type, in case of weapon. */

        /** Maps gender to sprite filenames. */
        std::map<int, std::string> mAnimationFiles;

        /** Stores the names of sounds to be played at certain event. */
        std::map< EquipmentSoundEvent, std::vector<std::string> > mSounds;
};

#endif
