/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra.
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

#include <guichan/exception.hpp>

#include "gui.h"
#include "inputmanager.h"

#include "dialogs/helpdialog.h"
#include "dialogs/okdialog.h"
#include "dialogs/setupdialog.h"

#include "handlers/shortcuthandler.h"

#include "widgets/shortcutwindow.h"

#include "../sdl/joystick.h"
#include "../sdl/keyboardconfig.h"

#include "../../core/log.h"

#include "../../core/map/map.h"

#include "../../core/map/sprite/localplayer.h"

#include "../../core/utils/dtor.h"
#include "../../core/utils/gettext.h"

#include "../../eathena/beingmanager.h"
#include "../../eathena/flooritemmanager.h"
#include "../../eathena/playerrelations.h"
#include "../../eathena/statemanager.h"

#include "../../eathena/gui/chat.h"
#include "../../eathena/gui/debugwindow.h"
#include "../../eathena/gui/emotewindow.h"
#include "../../eathena/gui/equipmentwindow.h"
#include "../../eathena/gui/inventorywindow.h"
#include "../../eathena/gui/minimap.h"
#include "../../eathena/gui/ministatus.h"
#include "../../eathena/gui/skill.h"
#include "../../eathena/gui/status.h"
#include "../../eathena/gui/trade.h"
#include "../../eathena/gui/viewport.h"

#include "../../eathena/handlers/emoteshortcut.h"
#include "../../eathena/handlers/itemshortcut.h"

KeyboardConfig keyboard;
Joystick *joystick = NULL;

namespace
{
    bool targetKeyHeld = false;
}

InputManager::InputManager()
{
    keyboard.init();
    Joystick::init();
    // TODO: The user should be able to choose which one to use
    // Open the first device
    if (Joystick::getNumberOfJoysticks() > 0)
        joystick = new Joystick(0);
}

InputManager::~InputManager()
{
    destroy(joystick);
}

void InputManager::forwardInput(const SDL_Event &event)
{
    try
    {
        guiInput->pushInput(event);
    }
    catch (gcn::Exception e)
    {
        const char* err = e.getMessage().c_str();
        logger->log("Warning: guichan input exception: %s", err);
    }
}

void InputManager::handleInput()
{
    // Events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        bool used = false;

        // Key press events
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            used = handleKeyboardInput(event);
            
        if (event.type == SDL_JOYAXISMOTION || event.type == SDL_JOYBALLMOTION ||
            event.type == SDL_JOYHATMOTION || event.type == SDL_JOYBUTTONDOWN ||
            event.type == SDL_JOYBUTTONUP)
            handleJoystickInput(event);

        // Quit event
        else if (event.type == SDL_QUIT)
            stateManager->setState(QUIT_STATE);

        // Push input to GUI when not used
        if (!used)
            forwardInput(event);
    }
}

void InputManager::handleJoystickInput(const SDL_Event &event)
{
    if (joystick)
        joystick->update();
    else
        return;

    if (!stateManager->isInGame())
        return;

    if (event.type == SDL_JOYBUTTONUP && !joystick->buttonPressed(3))
    {
        targetKeyHeld = false; // Stop protecting the target key
    }
    else if (player_node && player_node->mAction != Being::DEAD)
    {
        const Uint16 x = player_node->mX;
        const Uint16 y = player_node->mY;

        if (!keyboard.isKeyActive(keyboard.KEY_CLEAR_TARGET))
        {
            Being *target = player_node->getTarget();

            // Target the nearest monster
            if (joystick->buttonPressed(3) && !targetKeyHeld)
            {
                target = beingManager->findNearestLivingBeing(x, y, 20,
                                                              Being::MONSTER);
                player_node->setTarget(target);
                targetKeyHeld = true;
            }

            if (joystick->buttonPressed(0) && target &&
                target->getType() != Being::NPC)
            {
                player_node->attack(target, true);
            }
        }

        if (joystick->buttonPressed(1))
        {
            FloorItem *item = floorItemManager->findByCoordinates(x, y);

            if (item)
                player_node->pickUp(item);
        }

        if (joystick->buttonPressed(2))
            player_node->toggleSit();

        unsigned char direction = 0;

        // Translate pressed keys to movement and direction
        if (joystick->isUp())
            direction |= Being::UP;
        else if (joystick->isDown())
            direction |= Being::DOWN;

        if (joystick->isLeft())
            direction |= Being::LEFT;
        else if (joystick->isRight())
            direction |= Being::RIGHT;

        player_node->setWalkingDir(direction);
    }
}

bool InputManager::handleKeyboardInput(const SDL_Event &event)
{
    bool used = false;
    bool metaKeyHeld = false;

    // Key press events
    if (event.type == SDL_KEYDOWN)
    {
        if (setupWindow->isVisible() &&
            keyboard.getNewKeyIndex() > keyboard.KEY_NO_VALUE)
        {
            keyboard.setNewKey((int) event.key.keysym.sym);
            keyboard.callbackNewKey();
            keyboard.setNewKeyIndex(keyboard.KEY_NO_VALUE);
            return false;
        }

        // If the user is configuring the keys then don't respond.
        if (!keyboard.isEnabled())
            return false;

        const int tKey = keyboard.getKeyIndex(event.key.keysym.sym);

        // Ignore input if either "ignore" key is pressed
        // Stops the character moving about if the user's window manager
        // uses "ignore+arrow key" to switch virtual desktops.
        if (keyboard.isKeyActive(keyboard.KEY_IGNORE_INPUT_1) ||
            keyboard.isKeyActive(keyboard.KEY_IGNORE_INPUT_2))
        {
            return false;
        }

        if (keyboard.isKeyLocked(event.key.keysym.sym))
            return false;

        gcn::Window *requestedWindow = NULL;

        switch (tKey)
        {
            // In-game Help
            case KeyboardConfig::KEY_WINDOW_HELP:
                requestedWindow = helpDialog;
                break;
            // Quitting confirmation dialog
            case KeyboardConfig::KEY_QUIT:
                stateManager->promptForQuit();
                used = true;
                break;
            case KeyboardConfig::KEY_WINDOW_DEBUG:
                requestedWindow = debugWindow;
                break;
            case KeyboardConfig::KEY_WINDOW_SETUP:
                requestedWindow = setupWindow;
                break;
            // Screenshot (picture, hence the p)
            case KeyboardConfig::KEY_SCREENSHOT:
                saveScreenshot();
                used = true;
                break;
        }

        if (stateManager->isInGame())
        {
            switch (tKey)
            {
                case KeyboardConfig::KEY_SCROLL_CHAT_UP:
                    if (chatWindow && chatWindow->isVisible())
                    {
                        chatWindow->scroll(-DEFAULT_CHAT_WINDOW_SCROLL);
                        used = true;
                    }
                    break;
                case KeyboardConfig::KEY_SCROLL_CHAT_DOWN:
                    if (chatWindow && chatWindow->isVisible())
                    {
                        chatWindow->scroll(DEFAULT_CHAT_WINDOW_SCROLL);
                        used = true;
                    }
                    break;
                case KeyboardConfig::KEY_WINDOW_STATUS:
                    requestedWindow = statusWindow;
                    break;
                case KeyboardConfig::KEY_WINDOW_INVENTORY:
                    requestedWindow = inventoryWindow;
                    break;
                case KeyboardConfig::KEY_WINDOW_EQUIPMENT:
                    requestedWindow = equipmentWindow;
                    break;
                case KeyboardConfig::KEY_WINDOW_SKILL:
                    requestedWindow = skillDialog;
                    break;
                case KeyboardConfig::KEY_WINDOW_MINIMAP:
                    minimap->toggle();
                    break;
                case KeyboardConfig::KEY_WINDOW_CHAT:
                    requestedWindow = chatWindow;
                    break;
                case KeyboardConfig::KEY_WINDOW_EMOTE:
                    requestedWindow = emoteWindow;
                    break;
                case KeyboardConfig::KEY_WINDOW_ITEM_SHORTCUT:
                    requestedWindow = itemShortcutWindow;
                    break;
                case KeyboardConfig::KEY_WINDOW_EMOTE_SHORTCUT:
                    requestedWindow = emoteShortcutWindow;
                    break;
                // Hide certain windows
                case KeyboardConfig::KEY_HIDE_WINDOWS:
                    statusWindow->hide();
                    inventoryWindow->hide();
                    equipmentWindow->hide();
                    skillDialog->hide();
                    chatWindow->hide();
                    itemShortcutWindow->hide();
                    setupWindow->hide();
                    debugWindow->hide();
                    emoteWindow->hide();
                    helpDialog->hide();
                    emoteShortcutWindow->hide();
                    minimap->hide();
                    used = true;
                    break;
                // Find path to mouse (debug purpose)
                case KeyboardConfig::KEY_PATHFIND:
                    viewport->toggleDebugPath();
                    used = true;
                    break;
                default:
                    break;
            }

            metaKeyHeld = keyboard.isKeyActive(keyboard.KEY_METAKEY);

            for (int i = 0; i <= SHORTCUTS; i++)
            {
                ShortcutHandler *shortcut = metaKeyHeld ? (ShortcutHandler *) emoteShortcut :
                                                          (ShortcutHandler *) itemShortcut;
                const int offset = metaKeyHeld ? KeyboardConfig::KEY_EMOTE_SHORTCUT_1 :
                                                 KeyboardConfig::KEY_ITEM_SHORTCUT_1;

                if (keyboard.isKeyActive(i + offset))
                {
                    shortcut->useShortcut(i);
                    used = true;
                    break;
                }
            }

            if (keyboard.isKeyActive(keyboard.KEY_TOGGLE_CHAT) && chatWindow)
            {
                chatWindow->requestChatFocus();
                used = true;
            }

            // Player actions
            if (player_node && player_node->mAction != Being::DEAD)
            {
                Being *target = player_node->getTarget();
                const Uint16 x = player_node->mX;
                const Uint16 y = player_node->mY;

                if (!keyboard.isKeyActive(keyboard.KEY_CLEAR_TARGET))
                {
                    Being::Type type = Being::INVALID;

                    // Target the nearest monster
                    if (keyboard.isKeyActive(keyboard.KEY_TARGET_MONSTER))
                        type = Being::MONSTER;
                    // Target the nearest player
                    else if (keyboard.isKeyActive(keyboard.KEY_TARGET_PLAYER))
                        type = Being::PLAYER;
                    // Target the nearest npc
                    else if (keyboard.isKeyActive(keyboard.KEY_TARGET_NPC))
                        type = Being::NPC;

                    target = beingManager->findNearestLivingBeing(x, y, 20,
                                           type != Being::INVALID ? type :
                                                   Being::UNKNOWN);

                    if (type != Being::INVALID && !targetKeyHeld)
                    {
                        player_node->setTarget(target);
                        targetKeyHeld = true;
                        used = true;
                    }
                    else if (player_node->isAttacking())
                        target = NULL;

                    if (keyboard.isKeyActive(keyboard.KEY_ATTACK) && target)
                    {
                        player_node->attack(player_node->getTarget() ?
                                            player_node->getTarget() : target,
                                            target->getType() != Being::NPC);
                        used = true;
                    }
                }
                // Stop attacking
                else if (keyboard.isKeyActive(keyboard.KEY_CLEAR_TARGET))
                {
                    player_node->stopAttack();
                    targetKeyHeld = false;
                }

                if (keyboard.isKeyActive(keyboard.KEY_BEING_MENU))
                {
                    target = player_node->getTarget();

                    if (!target)
                        target = beingManager->findNearestLivingBeing(x, y, 20);

                    if (target)
                    {
                        Map *map = viewport->getMap();
                        viewport->showPopup(target->mX * map->getTileWidth() -
                                            viewport->getCameraX() +
                                           (map->getTileWidth() / 2),
                                            target->mY * map->getTileHeight() -
                                            viewport->getCameraY(), target);
                    }
                    used = true;
                }

                switch (tKey)
                {
                    case KeyboardConfig::KEY_PICKUP:
                        {
                            Uint16 x = player_node->mX;
                            Uint16 y = player_node->mY;
                            FloorItem *item = floorItemManager->
                                                  findNearestItem(x, y);

                            if (item)
                                player_node->pickUp(item);

                            used = true;
                        }
                        break;
                    // Player sit action
                    case KeyboardConfig::KEY_SIT:
                        player_node->toggleSit();
                        used = true;
                        break;
                    // Toggle accepting of incoming trade requests
                    case KeyboardConfig::KEY_TRADE:
                        {
                            unsigned int deflt = player_relations.getDefault();
                            if (deflt & PlayerRelation::TRADE)
                            {
                                chatWindow->chatLog(_("Ignoring incoming "
                                                      "trade requests"),
                                                      Palette::SYSTEM);
                                deflt &= ~PlayerRelation::TRADE;
                            }
                            else
                            {
                                chatWindow->chatLog(_("Accepting incoming "
                                                      "trade requests"),
                                                      Palette::SYSTEM);
                                deflt |= PlayerRelation::TRADE;
                            }

                            player_relations.setDefault(deflt);
                            used = true;
                        }
                        break;
               }
            }
        }

        if (requestedWindow)
        {
            requestedWindow->setVisible(!requestedWindow->isVisible());
            used = true;
        }
    }
    else if (event.type == SDL_KEYUP)
    {
        const int tKey = keyboard.getKeyIndex(event.key.keysym.sym);

        // Stop protecting the target keys
        if (tKey == KeyboardConfig::KEY_TARGET_MONSTER ||
            tKey == KeyboardConfig::KEY_TARGET_PLAYER || 
            tKey == KeyboardConfig::KEY_TARGET_NPC)
        {
            targetKeyHeld = false;
        }
    }

    // At the moment, this is the only bit of logic left not assigned to a
    // specific SDL input poll, because it was causing continuous walking
    // without stopping. It would be better in the long run if this would get
    // fixed and then moved to one of them (in case we ever use other input
    // libraries. If everything is already grouped together, it'd be easier to
    // adapt to a new library, instead of having to rely on special cases)
    if (stateManager->isInGame() && player_node->mAction != Being::DEAD && !used)
    {
        unsigned char direction = 0;

        // Get the state of the keyboard keys
        keyboard.refreshActiveKeys();

        // Translate pressed keys to movement and direction
        if (keyboard.isKeyActive(keyboard.KEY_MOVE_UP))
            direction |= Being::UP;
        else if (keyboard.isKeyActive(keyboard.KEY_MOVE_DOWN))
            direction |= Being::DOWN;

        if (keyboard.isKeyActive(keyboard.KEY_MOVE_LEFT))
            direction |= Being::LEFT;
        else if (keyboard.isKeyActive(keyboard.KEY_MOVE_RIGHT))
            direction |= Being::RIGHT;

        if (metaKeyHeld && direction != 0)
            player_node->setDirection(direction);
        else
            player_node->setWalkingDir(direction);
    }

    return used;
}
