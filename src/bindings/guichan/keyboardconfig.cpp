/*
 *  Aethyra
 *  Copyright (C) 2007  The Mana World Development Team
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

#include "keyboardconfig.h"

#include "sdl/sdlinput.h"

#include "../../configuration.h"

#include "../../gui/tabs/setup_input.h"

#include "../../utils/gettext.h"
#include "../../utils/stringutils.h"

struct KeyData
{
    const char *configField;
    int defaultValue;
    std::string caption;
};

// keyData must be in same order as enum keyAction.
static KeyData const keyData[KeyboardConfig::KEY_TOTAL] = {
    {"keyMoveUp", SDLK_UP, _("Move Up")},
    {"keyMoveDown", SDLK_DOWN, _("Move Down")},
    {"keyMoveLeft", SDLK_LEFT, _("Move Left")},
    {"keyMoveRight", SDLK_RIGHT, _("Move Right")},
    {"keyAttack", SDLK_LCTRL, _("Attack")},
    {"keySmilie", SDLK_LALT, _("Smilie")},
    {"keyTarget", SDLK_LSHIFT, _("Stop Attack")},
    {"keyTargetClosest", SDLK_a, _("Target Monster")},
    {"keyTargetNPC", SDLK_n, _("Target NPC")},
    {"keyTargetPlayer", SDLK_q, _("Target Player")},
    {"keyBeingMenu", SDLK_m, _("Being Menu")},
    {"keyPickup", SDLK_z, _("Pickup")},
    {"keyHideWindows", SDLK_h, _("Hide Windows")},
    {"keyBeingSit", SDLK_s, _("Sit")},
    {"keyScreenshot", SDLK_p, _("Screenshot")},
    {"keyTrade", SDLK_r, _("Enable/Disable Trading")},
    {"keyPathfind", SDLK_f, _("Find Path to Mouse")},
    {"keyItemShortcut", SDLK_i, _("Item Shortcut Window")},
    {"keyEmoteShortcut", SDLK_e, _("Emote Shortcut Window")},
    {"keyShortcut1", SDLK_1, strprintf(_("Item Shortcut %d"), 1)},
    {"keyShortcut2", SDLK_2, strprintf(_("Item Shortcut %d"), 2)},
    {"keyShortcut3", SDLK_3, strprintf(_("Item Shortcut %d"), 3)},
    {"keyShortcut4", SDLK_4, strprintf(_("Item Shortcut %d"), 4)},
    {"keyShortcut5", SDLK_5, strprintf(_("Item Shortcut %d"), 5)},
    {"keyShortcut6", SDLK_6, strprintf(_("Item Shortcut %d"), 6)},
    {"keyShortcut7", SDLK_7, strprintf(_("Item Shortcut %d"), 7)},
    {"keyShortcut8", SDLK_8, strprintf(_("Item Shortcut %d"), 8)},
    {"keyShortcut9", SDLK_9, strprintf(_("Item Shortcut %d"), 9)},
    {"keyShortcut10", SDLK_0, strprintf(_("Item Shortcut %d"), 10)},
    {"keyShortcut11", SDLK_MINUS, strprintf(_("Item Shortcut %d"), 11)},
    {"keyShortcut12", SDLK_EQUALS, strprintf(_("Item Shortcut %d"), 12)},
    {"keyWindowHelp", SDLK_F1, _("Help Window")},
    {"keyWindowStatus", SDLK_F2, _("Status Window")},
    {"keyWindowInventory", SDLK_F3, _("Inventory Window")},
    {"keyWindowEquipment", SDLK_F4, _("Equipment Window")},
    {"keyWindowSkill", SDLK_F5, _("Skill Window")},
    {"keyWindowMinimap", SDLK_F6, _("Minimap Window")},
    {"keyWindowChat", SDLK_F7, _("Chat Window")},
    {"keyWindowSetup", SDLK_F9, _("Setup Window")},
    {"keyWindowDebug", SDLK_F10, _("Debug Window")},
    {"keyWindowEmote", SDLK_F11, _("Emote Window")},
    {"keyEmoteShortcut1", SDLK_1, strprintf(_("Emote Shortcut %d"), 1)},
    {"keyEmoteShortcut2", SDLK_2, strprintf(_("Emote Shortcut %d"), 2)},
    {"keyEmoteShortcut3", SDLK_3, strprintf(_("Emote Shortcut %d"), 3)},
    {"keyEmoteShortcut4", SDLK_4, strprintf(_("Emote Shortcut %d"), 4)},
    {"keyEmoteShortcut5", SDLK_5, strprintf(_("Emote Shortcut %d"), 5)},
    {"keyEmoteShortcut6", SDLK_6, strprintf(_("Emote Shortcut %d"), 6)},
    {"keyEmoteShortcut7", SDLK_7, strprintf(_("Emote Shortcut %d"), 7)},
    {"keyEmoteShortcut8", SDLK_8, strprintf(_("Emote Shortcut %d"), 8)},
    {"keyEmoteShortcut9", SDLK_9, strprintf(_("Emote Shortcut %d"), 9)},
    {"keyEmoteShortcut10", SDLK_0, strprintf(_("Emote Shortcut %d"), 10)},
    {"keyEmoteShortcut11", SDLK_MINUS, strprintf(_("Emote Shortcut %d"), 11)},
    {"keyEmoteShortcut12", SDLK_EQUALS, strprintf(_("Emote Shortcut %d"), 12)},
    {"keyChat", SDLK_RETURN, _("Toggle Chat")},
    {"keyChatScrollUp", SDLK_PAGEUP, _("Scroll Chat Up")},
    {"keyChatScrollDown", SDLK_PAGEDOWN, _("Scroll Chat Down")},
    {"keyQuit", SDLK_ESCAPE, _("Quit")},
    {"keyIgnoreInput1", SDLK_LSUPER, strprintf(_("Ignore Input %d"), 1)},
    {"keyIgnoreInput2", SDLK_RSUPER, strprintf(_("Ignore Input %d"), 2)}
};

void KeyboardConfig::init()
{
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        mKey[i].configField = keyData[i].configField;
        mKey[i].defaultValue = keyData[i].defaultValue;
        mKey[i].caption = keyData[i].caption;
        mKey[i].value = KEY_NO_VALUE;
    }
    mNewKeyIndex = KEY_NO_VALUE;
    mEnabled = true;
    mPrintableKeyLock = false;

    retrieve();
}

void KeyboardConfig::retrieve()
{
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        mKey[i].value = (int) config.getValue(
            mKey[i].configField, mKey[i].defaultValue);
    }
}

void KeyboardConfig::store()
{
    for (int i = 0; i < KEY_TOTAL; i++)
        config.setValue(mKey[i].configField, mKey[i].value);
}

void KeyboardConfig::makeDefault()
{
    for (int i = 0; i < KEY_TOTAL; i++)
        mKey[i].value = mKey[i].defaultValue;
}

bool KeyboardConfig::hasConflicts()
{
    int i, j;
    /**
     * No need to parse the square matrix: only check one triangle
     * that's enough to detect conflicts
     */
    for (i = 0; i < KEY_TOTAL; i++)
    {
        for (j = i, j++; j < KEY_TOTAL; j++)
        {
            // Allow for item shortcut and emote keys to overlap, but no other keys
            // Also don't allow a key to be assigned to tab, since this
            // can mess up focus handling with the keyboard and cause unexpected
            // behavior.
            if (!(((i >= KEY_SHORTCUT_1) && (i <= KEY_SHORTCUT_12)) &&
                  ((j >= KEY_EMOTE_1) && (j <= KEY_EMOTE_12))) &&
                  (mKey[i].value == mKey[j].value || mKey[i].value == SDLK_TAB))
            {
                return true;
            }
        }
    }
    return false;
}

void KeyboardConfig::callbackNewKey()
{
    mSetupKey->newKeyCallback(mNewKeyIndex);
}

void KeyboardConfig::lockKey(const int &keyValue)
{
    mLockedKeys.push_back(keyValue);
}

void KeyboardConfig::unlockKey(const int &keyValue)
{
    for (std::list<int>::iterator it = mLockedKeys.begin();
         it != mLockedKeys.end(); it++)
    {
        if (*it == keyValue)
            it = mLockedKeys.erase(it);
    }
}

bool KeyboardConfig::isKeyLocked(const int &index)
{
    bool locked = false;

    if (mPrintableKeyLock)
    {
        if ((mKey[index].value & 0xFF80) == 0)
        {
            char key = mKey[index].value & 0x7F;

            if (isalpha(key) || isdigit(key) || ispunct(key) || key == ' ')
                locked = true;
        }
        /** TODO: Create an else statement which catches Unicode characters, but
         *        not keys like the function keys (as an example). This version
         *        below might work for some people, but their are still some
         *        keys that can still be found in upper mappings besides just
         *        these (F12 is one such possibility), and because of this, we
         *        need a better solution.
        else if (mKey[index].value > SDLK_UNDO) // Unicode character, so lock it
            locked = true; */
    }

    if (!locked)
    {
        for (std::list<int>::iterator it = mLockedKeys.begin();
             it != mLockedKeys.end(); it++)
        {
            if (*it == mKey[index].value)
            {
                locked = true;
                break;
            }
        }
    }

    return mEnabled && locked;
}

int KeyboardConfig::getKeyIndex(int keyValue) const
{
    for (int i = 0; i < KEY_TOTAL; i++)
    {
        if (keyValue == mKey[i].value)
            return i;
    }
    return KEY_NO_VALUE;
}

int KeyboardConfig::getKeyEmoteOffset(int keyValue) const
{
    for (int i = KEY_EMOTE_1; i <= KEY_EMOTE_12; i++)
    {
        if (keyValue == mKey[i].value)
            return 1 + i - KEY_EMOTE_1;
    }
    return 0;
}

bool KeyboardConfig::isKeyActive(int index)
{
    return mActiveKeys && mEnabled ? mActiveKeys[mKey[index].value] : false;
}

void KeyboardConfig::refreshActiveKeys()
{
    mActiveKeys = SDL_GetKeyState(NULL);
}
