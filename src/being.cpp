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
#include "being.h"

#include <algorithm>

#include "animatedsprite.h"
#include "equipment.h"
#include "game.h"
#include "graphics.h"
#include "log.h"
#include "map.h"

#include "resources/resourcemanager.h"
#include "resources/imageset.h"

#include "gui/gui.h"

#include "utils/dtor.h"
#include "utils/tostring.h"

int Being::instances = 0;
ImageSet *Being::emotionSet = NULL;

Being::Being(Uint32 id, Uint16 job, Map *map):
    mJob(job),
    mX(0), mY(0),
    mAction(0),
    mWalkTime(0),
    mEmotion(0), mEmotionTime(0),
    mAttackSpeed(350),
    mEquipment(new Equipment()),
    mId(id),
    mWeapon(0),
    mWalkSpeed(150),
    mDirection(DOWN),
    mMap(NULL),
    mHairStyle(0), mHairColor(0),
    mSex(2),
    mSpeechTime(0),
    mDamageTime(0),
    mPx(0), mPy(0),
    mSprites(VECTOREND_SPRITE, NULL),
    mEquipmentSpriteIDs(VECTOREND_SPRITE, 0)
{
    setMap(map);

    if (instances == 0)
    {
        // Load the emotion set
        ResourceManager *rm = ResourceManager::getInstance();
        emotionSet = rm->getImageSet("graphics/sprites/emotions.png", 30, 32);
        if (!emotionSet) logger->error("Unable to load emotions!");
    }

    instances++;
}

Being::~Being()
{
    std::for_each(mSprites.begin(), mSprites.end(), make_dtor(mSprites));
    clearPath();
    setMap(NULL);

    instances--;

    if (instances == 0)
    {
        emotionSet->decRef();
        emotionSet = NULL;
    }
}

void
Being::setDestination(Uint16 destX, Uint16 destY)
{
    if (mMap)
    {
        setPath(mMap->findPath(mX, mY, destX, destY));
    }
}

void
Being::clearPath()
{
    mPath.clear();
}

void
Being::setPath(const Path &path)
{
    mPath = path;

    if (mAction != WALK && mAction != DEAD)
    {
        nextStep();
        mWalkTime = tick_time;
    }
}

void
Being::setHairColor(Uint16 color)
{
    mHairColor = color;
    if (mHairColor < 1 || mHairColor > NR_HAIR_COLORS + 1)
    {
        mHairColor = 1;
    }
}

void
Being::setHairStyle(Uint16 style)
{
    mHairStyle = style;
    if (mHairStyle < 1 || mHairStyle > NR_HAIR_STYLES)
    {
        mHairStyle = 1;
    }
}

void
Being::setVisibleEquipment(Uint8 slot, int id)
{
    mEquipmentSpriteIDs[slot] = id;
}

void
Being::setSpeech(const std::string &text, Uint32 time)
{
    mSpeech = text;
    mSpeechTime = 500;
}

void
Being::takeDamage(int amount)
{
    mDamage = amount ? toString(amount) : "miss";
    mDamageTime = 300;
}

void
Being::handleAttack(Being *victim, int damage)
{
    setAction(Being::ATTACK);
    mFrame = 0;
    mWalkTime = tick_time;
}

void
Being::setMap(Map *map)
{
    // Remove sprite from potential previous map
    if (mMap != NULL)
    {
        mMap->removeSprite(mSpriteIterator);
    }

    mMap = map;

    // Add sprite to potential new map
    if (mMap != NULL)
    {
        mSpriteIterator = mMap->addSprite(this);
    }
}

void
Being::setAction(Uint8 action)
{
    SpriteAction currentAction = ACTION_INVALID;
    switch (action)
    {
        case WALK:
            currentAction = ACTION_WALK;
            break;
        case SIT:
            currentAction = ACTION_SIT;
            break;
        case ATTACK:
            switch (getWeapon())
            {
                case 3:
                    currentAction = ACTION_ATTACK;
                    break;
                case 2:
                    currentAction = ACTION_ATTACK_BOW;
                    break;
                case 1:
                    currentAction = ACTION_ATTACK_STAB;
                    break;
                case 0:
                    currentAction = ACTION_ATTACK;
                    break;
            }
            for (int i = 0; i < VECTOREND_SPRITE; i++)
            {
                if (mSprites[i])
                {
                    mSprites[i]->reset();
                }
            }
            break;
        case HURT:
            //currentAction = ACTION_HURT;  // Buggy: makes the player stop
                                            // attacking and unable to attack
                                            // again until he moves
            break;
        case DEAD:
            currentAction = ACTION_DEAD;
            break;
        case STAND:
            currentAction = ACTION_STAND;
            break;
    }

    if (currentAction != ACTION_INVALID)
    {
        for (int i = 0; i < VECTOREND_SPRITE; i++)
        {
            if (mSprites[i])
            {
                mSprites[i]->play(currentAction);
            }
        }
        mAction = action;
    }
}


void
Being::setDirection(Uint8 direction)
{
    mDirection = direction;
    SpriteDirection dir = getSpriteDirection();

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i] != NULL)
            mSprites[i]->setDirection(dir);
    }
}

SpriteDirection
Being::getSpriteDirection() const
{
    SpriteDirection dir;

    if (mDirection & UP)
    {
        dir = DIRECTION_UP;
    }
    else if (mDirection & RIGHT)
    {
        dir = DIRECTION_RIGHT;
    }
    else if (mDirection & DOWN)
    {
        dir = DIRECTION_DOWN;
    }
    else {
        dir = DIRECTION_LEFT;
    }

    return dir;
}

void
Being::nextStep()
{
    if (mPath.empty())
    {
        setAction(STAND);
        return;
    }

    PATH_NODE node = mPath.front();
    mPath.pop_front();

    int dir = 0;
    if (node.x > mX)
        dir |= RIGHT;
    else if (node.x < mX)
        dir |= LEFT;
    if (node.y > mY)
        dir |= DOWN;
    else if (node.y < mY)
        dir |= UP;

    setDirection(dir);

    mX = node.x;
    mY = node.y;
    setAction(WALK);
    mWalkTime += mWalkSpeed / 10;
}

void
Being::logic()
{
    // Reduce the time that speech is still displayed
    if (mSpeechTime > 0)
        mSpeechTime--;

    // Reduce the time that damage is still displayed
    if (mDamageTime > 0)
        mDamageTime--;

    // Update pixel coordinates
    mPx = mX * 32 + getXOffset();
    mPy = mY * 32 + getYOffset();

    if (mEmotion != 0)
    {
        mEmotionTime--;
        if (mEmotionTime == 0) {
            mEmotion = 0;
        }
    }

    // Update sprite animations
    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i] != NULL)
        {
            mSprites[i]->update(tick_time * 10);
        }
    }
}

void
Being::draw(Graphics *graphics, int offsetX, int offsetY) const
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    for (int i = 0; i < VECTOREND_SPRITE; i++)
    {
        if (mSprites[i] != NULL)
        {
            mSprites[i]->draw(graphics, px, py);
        }
    }
}

void
Being::drawEmotion(Graphics *graphics, Sint32 offsetX, Sint32 offsetY)
{
    if (!mEmotion)
        return;

    int px = mPx + offsetX + 3;
    int py = mPy + offsetY - 60;

    graphics->drawImage(emotionSet->get(mEmotion - 1), px, py);
}

void
Being::drawSpeech(Graphics *graphics, Sint32 offsetX, Sint32 offsetY)
{
    int px = mPx + offsetX;
    int py = mPy + offsetY;

    // Draw speech above this being
    if (mSpeechTime > 0)
    {
        graphics->setFont(speechFont);
        graphics->setColor(gcn::Color(255, 255, 255));
        graphics->drawText(mSpeech, px + 18, py - 60, gcn::Graphics::CENTER);
    }

    // Draw damage above this being
    if (mDamageTime > 0 && mDamageTime < 275)
    {
        // Selecting the right color
        if (mDamage == "miss")
        {
            graphics->setFont(hitYellowFont);
        }
        else if (getType() == MONSTER)
        {
            graphics->setFont(hitBlueFont);
        }
        else
        {
            graphics->setFont(hitRedFont);
        }

        int textY = (getType() == MONSTER) ? 32 : 70;
        int ft = 150 - mDamageTime;
        float a = (ft > 0) ? 1.0 - ft / 150.0 : 1.0;

        graphics->setColor(gcn::Color(255, 255, 255, (int)(255 * a)));
        graphics->drawText(mDamage,
                           px + 16,
                           py - textY - (300 - mDamageTime) / 10,
                           gcn::Graphics::CENTER);

        // Reset alpha value
        graphics->setColor(gcn::Color(255, 255, 255));
    }
}

Being::Type
Being::getType() const
{
    return UNKNOWN;
}

void
Being::setWeaponById(Uint16 weapon)
{
    //TODO: Use an external file to map weapon IDs to weapon types
    switch (weapon)
    {
    case 529: // iron arrows
    case 1199: // arrows
        break;

    case 623: //scythe
        setWeapon(3);
        break;

    case 1200: // bow
    case 530: // short bow
    case 545: // forest bow
        setWeapon(2);
        break;

    case 521: // sharp knife
        /*  UNCOMMENT TO TEST SHARP KNIFE AS SCYTHE
         *  setWeapon(3)
         *  break;
         */
    case 522: // dagger
    case 536: // short sword
    case 1201: // knife
        setWeapon(1);
        break;

    case 0: // unequip
        setWeapon(0);
        break;

    default:
        logger->log("Not a weapon: %d", weapon);
    }
}

int
Being::getOffset(char pos, char neg) const
{
    // Check whether we're walking in the requested direction
    if (mAction != WALK || !(mDirection & (pos | neg))) {
        return 0;
    }

    int offset = (get_elapsed_time(mWalkTime) * 32) / mWalkSpeed;

    // We calculate the offset _from_ the _target_ location
    offset -= 32;
    if (offset > 0) {
        offset = 0;
    }

    // Going into negative direction? Invert the offset.
    if (mDirection & pos) {
        offset = -offset;
    }

    return offset;
}
