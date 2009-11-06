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

#include <guichan/actionevent.hpp>

#include "popupmenu.h"
#include "viewport.h"

#include "../beingmanager.h"
#include "../flooritemmanager.h"

#include "../../bindings/guichan/graphics.h"
#include "../../bindings/guichan/gui.h"
#include "../../bindings/guichan/textmanager.h"

#include "../../bindings/sdl/keyboardconfig.h"

#include "../../core/configuration.h"

#include "../../core/map/map.h"

#include "../../core/map/sprite/localplayer.h"
#include "../../core/map/sprite/npc.h"

#include "../../core/utils/stringutils.h"

extern volatile int tick_time;

Viewport::Viewport():
    mMap(0),
    mPixelViewX(0.0f),
    mPixelViewY(0.0f),
    mTileViewX(0),
    mTileViewY(0),
    mShowDebugPath(false),
    mPlayerFollowMouse(false),
    mWalkTime(0)
{
    setOpaque(false);
    addMouseListener(this);

    mScrollLaziness = config.getValue("ScrollLaziness", 16);
    mScrollRadius = config.getValue("ScrollRadius", 0);
    mScrollCenterOffsetX = config.getValue("ScrollCenterOffsetX", 0);
    mScrollCenterOffsetY = config.getValue("ScrollCenterOffsetY", 0);

    config.addListener("ScrollLaziness", this);
    config.addListener("ScrollRadius", this);

    mPopupMenu = new PopupMenu(UNKNOWN, this);

    setDimension(gcn::Rectangle(0, 0, graphics->getWidth(),
                                graphics->getHeight()));
}

Viewport::~Viewport()
{
    delete mPopupMenu;
}

void Viewport::setMap(Map *map)
{
    mMap = map;
}

void Viewport::draw(gcn::Graphics *graphics)
{
    static int lastTick = tick_time;

    if (!mMap || !player_node)
    {
        graphics->setColor(gcn::Color(64, 64, 64));
        graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
        return;
    }

    Graphics *g = static_cast<Graphics*>(graphics);

    // Avoid freaking out when tick_time overflows
    if (tick_time < lastTick)
        lastTick = tick_time;

    // Calculate viewpoint
    int midTileX = (g->getWidth() + mScrollCenterOffsetX) / 32 / 2;
    int midTileY = (g->getHeight() + mScrollCenterOffsetY) / 32 / 2;

    int player_x = (player_node->mX - midTileX) * 32 +
                    player_node->getXOffset();
    int player_y = (player_node->mY - midTileY) * 32 +
                    player_node->getYOffset();

    if (mScrollLaziness < 1)
        mScrollLaziness = 1; // Avoids division by zero

    // Apply lazy scrolling
    while (lastTick < tick_time)
    {
        if (player_x > mPixelViewX + mScrollRadius)
        {
            mPixelViewX += (player_x - mPixelViewX - mScrollRadius) /
                            mScrollLaziness;
        }
        if (player_x < mPixelViewX - mScrollRadius)
        {
            mPixelViewX += (player_x - mPixelViewX + mScrollRadius) /
                            mScrollLaziness;
        }
        if (player_y > mPixelViewY + mScrollRadius)
        {
            mPixelViewY += (player_y - mPixelViewY - mScrollRadius) /
                            mScrollLaziness;
        }
        if (player_y < mPixelViewY - mScrollRadius)
        {
            mPixelViewY += (player_y - mPixelViewY + mScrollRadius) /
                            mScrollLaziness;
        }
        lastTick++;
    }

    // Auto center when player is off screen
    if (player_x - mPixelViewX > g->getWidth() / 2 ||
        mPixelViewX - player_x > g->getWidth() / 2 ||
        mPixelViewY - player_y > g->getHeight() / 2 ||
        player_y - mPixelViewY > g->getHeight() / 2)
    {
        mPixelViewX = player_x;
        mPixelViewY = player_y;
    };

    // Don't move camera so that the end of the map is on screen
    const int viewXmax = (mMap->getWidth() * 32) - g->getWidth();
    const int viewYmax = (mMap->getHeight() * 32) - g->getHeight();

    if (mMap)
    {
        if (mPixelViewX < 0)
            mPixelViewX = 0;
        if (mPixelViewY < 0)
            mPixelViewY = 0;
        if (mPixelViewX > viewXmax)
            mPixelViewX = viewXmax;
        if (mPixelViewY > viewYmax)
            mPixelViewY = viewYmax;
    }

    mTileViewX = (int) (mPixelViewX + 16) / 32;
    mTileViewY = (int) (mPixelViewY + 16) / 32;

    // Draw tiles and sprites
    if (mMap)
    {
        mMap->draw(g, (int) mPixelViewX, (int) mPixelViewY);

        // Find a path from the player to the mouse, and draw it. This is for
        // debug purposes.
        if (mShowDebugPath)
        {
            // Get the current mouse position
            const int mouseX = gui->getMouseX();
            const int mouseY = gui->getMouseY();
            const int mouseTileX = mouseX / 32 + mTileViewX;
            const int mouseTileY = mouseY / 32 + mTileViewY;

            Path debugPath = mMap->findPath(player_node->mX, player_node->mY,
                                            mouseTileX, mouseTileY);

            g->setColor(gcn::Color(255, 0, 0));
            for (PathIterator i = debugPath.begin(); i != debugPath.end(); i++)
            {
                const int squareX = i->x * 32 - (int) mPixelViewX + 12;
                const int squareY = i->y * 32 - (int) mPixelViewY + 12;

                g->fillRectangle(gcn::Rectangle(squareX, squareY, 8, 8));
                g->drawText(toString(mMap->getMetaTile(i->x, i->y)->Gcost),
                                   squareX + 4, squareY + 12,
                                   gcn::Graphics::CENTER);
            }
        }
    }

    if (player_node->mUpdateName)
    {
        player_node->mUpdateName = false;
        player_node->setName(player_node->getName());
    }

    // Draw text
    if (textManager)
        textManager->draw(g, (int) mPixelViewX, (int) mPixelViewY);

    // Draw player names, speech, and emotion sprite as needed
    const Beings &beings = beingManager->getAll();
    for (Beings::const_iterator i = beings.begin(), i_end = beings.end();
         i != i_end; ++i)
    {
        (*i)->drawSpeech((int) mPixelViewX, (int) mPixelViewY);
        (*i)->drawEmotion(g, (int) mPixelViewX, (int) mPixelViewY);
    }

    drawChildren(g);
}

void Viewport::logic()
{
    WindowContainer::logic();

    const int mouseX = gui->getMouseX();
    const int mouseY = gui->getMouseY();
    const Uint8 button = gui->getButtonState();

    if (!mMap || !player_node)
        return;

    if (mPlayerFollowMouse && button & SDL_BUTTON(1) &&
        mWalkTime != player_node->mWalkTime)
    {
        player_node->setDestination(mouseX / 32 + mTileViewX,
                                    mouseY / 32 + mTileViewY);
        mWalkTime = player_node->mWalkTime;
    }
}

void Viewport::mousePressed(gcn::MouseEvent &event)
{
    if (event.getSource() != this)
        return;

    // Check if we are alive and kickin'
    if (!mMap || !player_node || player_node->mAction == Being::DEAD)
        return;

    mPlayerFollowMouse = false;

    const int tilex = event.getX() / 32 + mTileViewX;
    const int tiley = event.getY() / 32 + mTileViewY;
    const int x = (int)((float) event.getX() + mPixelViewX);
    const int y = (int)((float) event.getY() + mPixelViewY);

    // Right click might open a popup
    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Being *being;
        FloorItem *floorItem;

        if ((being = beingManager->findBeingByPixel(x, y)) &&
             being != player_node)
        {
            mPopupMenu->setType(BEING);
            showPopup(event.getX(), event.getY(), being);
            return;
        }
        else if ((floorItem = floorItemManager->findByCoordinates(tilex,
                                                                  tiley)))
        {
            mPopupMenu->setType(FLOOR_ITEM);
            mPopupMenu->setFloorItem(floorItem);
            mPopupMenu->showPopup(event.getX(), event.getY());
            return;
        }
    }

    // If a popup is active, just remove it
    if (mPopupMenu->isVisible())
    {
        mPopupMenu->setVisible(false);
        return;
    }

    // Left click can cause different actions
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        Being *being;
        FloorItem *item;

        // Interact with some being
//        if ((being = beingManager->findBeing(tilex, tiley)))
        if ((being = beingManager->findBeingByPixel(x, y)) &&
             being->mAction != Being::DEAD)
        {
            if (being->getType() == Being::NPC && NPC::mTalking == false)
                static_cast<NPC*>(being)->talk();
            else if (player_node->withinAttackRange(being) &&
                     being->getType() != Being::NPC)
            {
                player_node->setGotoTarget(being);
                player_node->attack(being, true);
            }
            else
                player_node->setDestination(tilex, tiley);
        }
        // Pick up some item
        else if ((item = floorItemManager->findByCoordinates(tilex, tiley)))
            player_node->pickUp(item);
        else if (player_node->mAction == Being::SIT)
            return;
        // Just walk around
        else
        {
            player_node->stopAttack();
            player_node->setDestination(tilex, tiley);
            mPlayerFollowMouse = true;
        }
    }
    else if (event.getButton() == gcn::MouseEvent::MIDDLE)
    {
        // Find the being nearest to the clicked position
        Being *target = beingManager->findBeingByPixel(x, y);

        if (target)
             player_node->setTarget(target);
    }
}

void Viewport::mouseDragged(gcn::MouseEvent &event)
{
    if (!mMap || !player_node)
        return;

    if (mPlayerFollowMouse && mWalkTime == player_node->mWalkTime)
    {
        int destX = event.getX() / 32 + mTileViewX;
        int destY = event.getY() / 32 + mTileViewY;
        player_node->setDestination(destX, destY);
    }
}

void Viewport::mouseReleased(gcn::MouseEvent &event)
{
    mPlayerFollowMouse = false;
}

void Viewport::showPopup(int x, int y, Item *item)
{
    mPopupMenu->setType(ITEM_SHORTCUT);
    mPopupMenu->setItem(item);
    mPopupMenu->showPopup(x, y);
}

void Viewport::showPopup(int x, int y, Being *being)
{
    mPopupMenu->setType(BEING);
    mPopupMenu->setBeing(being);
    mPopupMenu->showPopup(x, y);
}

void Viewport::closePopupMenu()
{
    gcn::ActionEvent actionEvent(this, "cancel");
    mPopupMenu->action(actionEvent);
}

void Viewport::optionChanged(const std::string &name)
{
    mScrollLaziness = config.getValue("ScrollLaziness", 32);
    mScrollRadius = config.getValue("ScrollRadius", 32);
}
