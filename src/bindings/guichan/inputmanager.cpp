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
#include "keyboardconfig.h"

#include "dialogs/confirmdialog.h"
#include "dialogs/okdialog.h"

#include "../sdl/joystick.h"
#include "../sdl/sound.h"

#include "../../emoteshortcut.h"
#include "../../itemshortcut.h"
#include "../../log.h"
#include "../../main.h"
#include "../../playerrelations.h"

#include "../../gui/chat.h"
#include "../../gui/debugwindow.h"
#include "../../gui/emotewindow.h"
#include "../../gui/equipmentwindow.h"
#include "../../gui/help.h"
#include "../../gui/inventorywindow.h"
#include "../../gui/minimap.h"
#include "../../gui/ministatus.h"
#include "../../gui/setup.h"
#include "../../gui/shortcutwindow.h"
#include "../../gui/skill.h"
#include "../../gui/status.h"
#include "../../gui/trade.h"
#include "../../gui/viewport.h"

#include "../../resources/beingmanager.h"
#include "../../resources/flooritemmanager.h"

#include "../../resources/sprite/localplayer.h"

#include "../../utils/gettext.h"

Joystick *joystick = NULL;

extern OkDialog *weightNotice;
extern OkDialog *deathNotice;
ConfirmDialog *exitConfirm = NULL;

/**
 * Listener used for exiting handling.
 */
namespace
{
    struct ExitListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            if (event.getId() == "yes" || event.getId() == "ok")
            {
                sound.fadeOutMusic(1000);

                mInGame = false;

                state = EXIT_STATE;
            }

            exitConfirm = NULL;
        }
    } exitListener;
}

InputManager::InputManager()
{
    Joystick::init();
    // TODO: The user should be able to choose which one to use
    // Open the first device
    if (Joystick::getNumberOfJoysticks() > 0)
        joystick = new Joystick(0);
}

InputManager::~InputManager()
{
    delete joystick;
    joystick = NULL;
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
    handleJoystickInput();
    handleKeyboardInput();
}

void InputManager::handleJoystickInput()
{
    if (joystick)
        joystick->update();
    else
        return;

    if (!mInGame)
        return;

    // Player actions
    if (player_node && player_node->mAction != Being::DEAD)
    {
        const Uint16 x = player_node->mX;
        const Uint16 y = player_node->mY;

        if (!keyboard.isKeyActive(keyboard.KEY_TARGET))
        {
            Being *target = player_node->getTarget();

            // Target the nearest monster
            if (joystick->buttonPressed(3))
            {
                target = beingManager->findNearestLivingBeing(x, y, 20,
                                                              Being::MONSTER);
                player_node->setTarget(target);
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

void InputManager::handleKeyboardInput()
{
    // Events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        bool used = false;

        // Keyboard events (for discontinuous keys)
        if (event.type == SDL_KEYDOWN)
        {
            if (setupWindow->isVisible() &&
                keyboard.getNewKeyIndex() > keyboard.KEY_NO_VALUE)
            {
                keyboard.setNewKey((int) event.key.keysym.sym);
                keyboard.callbackNewKey();
                keyboard.setNewKeyIndex(keyboard.KEY_NO_VALUE);
                return;
            }

            // If the user is configuring the keys then don't respond.
            if (!keyboard.isEnabled())
                return;

            const int tKey = keyboard.getKeyIndex(event.key.keysym.sym);

            // Ignore input if either "ignore" key is pressed
            // Stops the character moving about if the user's window manager
            // uses "ignore+arrow key" to switch virtual desktops.
            if (keyboard.isKeyActive(keyboard.KEY_IGNORE_INPUT_1) ||
                keyboard.isKeyActive(keyboard.KEY_IGNORE_INPUT_2))
                continue;

            if (keyboard.isKeyLocked(tKey))
            {
                used = true;
                forwardInput(event);
                continue;
            }

            gcn::Window *requestedWindow = NULL;

            switch (tKey)
            {
                // In-game Help
                case KeyboardConfig::KEY_WINDOW_HELP:
                    requestedWindow = helpWindow;
                    break;
                // Quitting confirmation dialog
                case KeyboardConfig::KEY_QUIT:
                    if (!exitConfirm)
                    {
                        exitConfirm = new ConfirmDialog(_("Quit"),
                                                        _("Are you sure you "
                                                          "want to quit?"));
                        exitConfirm->addActionListener(&exitListener);
                        exitConfirm->requestMoveToTop();
                    }
                    else
                    {
                        exitConfirm->action(gcn::ActionEvent(NULL, "no"));
                    }
                    used = true;
                    break;
                case KeyboardConfig::KEY_WINDOW_DEBUG:
                    requestedWindow = debugWindow;
                    break;
                case KeyboardConfig::KEY_WINDOW_SETUP:
                    requestedWindow = setupWindow;
                    break;
                case KeyboardConfig::KEY_DROP_FOCUS:
                    gui->focusNone();
                    used = true;
                    break;
            }

            if (mInGame)
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
                    default:
                        break;
                }

                if (keyboard.isKeyActive(keyboard.KEY_EMOTE))
                {
                    const int emotion = keyboard.getKeyEmoteOffset(event.key.keysym.sym);

                    if (emotion)
                    {
                        emoteShortcut->useEmote(emotion);
                        used = true;
                        break;
                    }
                }

                if (keyboard.isKeyActive(keyboard.KEY_TOGGLE_CHAT) &&
                    chatWindow)
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

                    // Do not activate item shortcuts if tradewindow is visible
                    if (!tradeWindow->isVisible())
                    {
                        for (int i = KeyboardConfig::KEY_SHORTCUT_1;
                                 i <= KeyboardConfig::KEY_SHORTCUT_12;
                                 i++)
                        {
                            if (tKey == i)
                            {
                                itemShortcut->useItem(i - KeyboardConfig::KEY_SHORTCUT_1);
                                used = true;
                                break;
                            }
                        }
                    }

                    if (!keyboard.isKeyActive(keyboard.KEY_TARGET))
                    {
                        bool targetKeyHit = true;

                        // Target the nearest monster
                        if (keyboard.isKeyActive(keyboard.KEY_TARGET_CLOSEST))
                        {
                            target = beingManager->findNearestLivingBeing(
                                                   x, y, 20, Being::MONSTER);
                        }
                        // Target the nearest player
                        else if (keyboard.isKeyActive(keyboard.KEY_TARGET_PLAYER))
                        {
                            target = beingManager->findNearestLivingBeing(
                                                   x, y, 20, Being::PLAYER);
                        }
                        // Target the nearest npc
                        else if (keyboard.isKeyActive(keyboard.KEY_TARGET_NPC))
                        {
                            target = beingManager->findNearestLivingBeing(
                                                   x, y, 20, Being::NPC);
                        }
                        else
                            targetKeyHit = false;

                        if (targetKeyHit)
                        {
                            player_node->setTarget(target);
                            used = true;
                        }

                        if (keyboard.isKeyActive(keyboard.KEY_ATTACK) && 
                            target && target->getType() != Being::NPC)
                        {
                            player_node->attack(target, true);
                            used = true;
                        }
                    }
                    // Stop attacking
                    else
                    {
                        player_node->stopAttack();
                    }

                    if (keyboard.isKeyActive(keyboard.KEY_BEING_MENU))
                    {
                        if (!target)
                            target = beingManager->findNearestLivingBeing(
                                                   x, y, 20);

                        if (target)
                        {
                            viewport->showPopup(target->mX * 32 -
                                                viewport->getCameraX() + 16,
                                                target->mY * 32 -
                                                viewport->getCameraY(),
                                                target);
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
                                                      findByCoordinates(x, y);

                                // If none below the player, try the tile in
                                // front of the player
                                if (!item)
                                {
                                    if (player_node->getDirection() &
                                        Being::UP)
                                        y--;
                                    if (player_node->getDirection() &
                                        Being::DOWN)
                                        y++;
                                    if (player_node->getDirection() &
                                        Being::LEFT)
                                        x--;
                                    if (player_node->getDirection() &
                                        Being::RIGHT)
                                        x++;

                                    item = floorItemManager->
                                               findByCoordinates(x, y);
                                }

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

                        case KeyboardConfig::KEY_ITEM_SHORTCUT:
                            requestedWindow = itemShortcutWindow;
                            break;
                        case KeyboardConfig::KEY_EMOTE_SHORTCUT:
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
                            helpWindow->hide();
                            emoteShortcutWindow->hide();
                            minimap->hide();
                            used = true;
                            break;
                        // Screenshot (picture, hence the p)
                        case KeyboardConfig::KEY_SCREENSHOT:
                            saveScreenshot();
                            used = true;
                            break;
                        // Find path to mouse (debug purpose)
                        case KeyboardConfig::KEY_PATHFIND:
                            viewport->toggleDebugPath();
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
                                                          BY_SERVER);
                                    deflt &= ~PlayerRelation::TRADE;
                                }
                                else
                                {
                                    chatWindow->chatLog(_("Accepting incoming "
                                                          "trade requests"),
                                                          BY_SERVER);
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

        // Quit event
        else if (event.type == SDL_QUIT)
        {
            mInGame = false;

            state = EXIT_STATE;
        }

        // Push input to GUI when not used
        if (!used)
            forwardInput(event);

        // At the moment, this is the only bit of logic left out of the SDL
        // input poll, because it was causing continuous walking without
        // stopping. This might very well be the best place for this bit of
        // logic, but it would be better in the long run if it could be placed
        // there as well (in case we ever use other input libraries. If they're
        // all inside that loop, their implementing logic could be reduced to a
        // single function call)
        if (mInGame && player_node->mAction != Being::DEAD)
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

            player_node->setWalkingDir(direction);
       }
    } // End while
}
