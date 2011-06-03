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

#ifndef KEYBOARDCONFIG_H
#define KEYBOARDCONFIG_H

#include <stdint.h>
#include <list>
#include <string>
#include <utility>

/**
 * Each key represents a key function. Such as 'Move up', 'Attack' etc.
 */
struct KeyFunction
{
    const char* configField;    /** Field index that is in the config file. */
    int defaultValue;           /** The default key value used. */
    std::string caption;        /** The caption value for the key function. */
    int value;                  /** The actual value that is used. */
};

class Setup_Input;

class KeyboardConfig
{
    public:
        /**
         * Initializes the keyboard config explicitly.
         */
        void init();

        /**
         * Retrieve the key values from config file.
         */
        void retrieve();

        /**
         * Store the key values to config file.
         */
        void store();

        /**
         * Make the keys their default values.
         */
        void makeDefault();

        /**
         * Calls a function back so the key re-assignment(s) can be seen.
         */
        void callbackNewKey();

        /**
         * Sets whether to lock printable keys from input or not.
         */
        void setPrintableKeyLock(const bool lock) { mPrintableKeyLock = lock; }

        /**
         * Locks a key from being used outside of GUIChan key calls.
         */
        void lockKey(const int keyValue);

        /**
         * Unlocks a key from being used outside of GUIChan key calls.
         */
        void unlockKey(const int keyValue);

        /**
         * Whether input is currently being locked on a key.
         */
        const bool isKeyLocked(const int keyValue);

        /**
         * Obtain the value stored in memory.
         */
        const int getKeyValue(const int index) const { return mKey[index].value; }

        /**
         * Get the index of the new key to be assigned.
         */
        const int getNewKeyIndex() const { return mNewKeyIndex; }

        /**
         * Get the enable flag, which will stop the user from doing actions.
         */
        const bool isEnabled() const { return mEnabled; }

        /**
         * Get the key caption, providing more meaning to the user.
         */
        const std::string &getKeyCaption(const int index) const
        { return mKey[index].caption; }

        /**
         * Get the key function index by providing the keys value.
         */
        const int getKeyIndex(const int keyValue) const;

        /**
         * Set the enable flag, which will stop the user from doing actions.
         */
        void setEnabled(const bool flag) { mEnabled = flag; }

        /**
         * Set the index of the new key to be assigned.
         */
        void setNewKeyIndex(const int value) { mNewKeyIndex = value; }

        /**
         * Set the value of the new key.
         */
        void setNewKey(const int value) { mKey[mNewKeyIndex].value = value; }

        /**
         * Set a reference to the key setup window.
         */
        void setSetupKeyboard(Setup_Input *setupKey) { mSetupKey = setupKey; }

        /**
         * Checks if the key is active, by providing the key function index.
         */
        const bool isKeyActive(const int index);

        /**
         * Takes a snapshot of all the active keys.
         */
        void refreshActiveKeys();

        /**
         * All the key functions.
         * KEY_NO_VALUE is used in initialization, and should be unchanged.
         * KEY_TOTAL should always be last (used as a conditional in loops).
         * The key assignment view gets arranged according to the order of
         * these values.
         */
        enum KeyAction
        {
            KEY_NO_VALUE = -1,
            KEY_MOVE_UP,
            KEY_MOVE_DOWN,
            KEY_MOVE_LEFT,
            KEY_MOVE_RIGHT,
            KEY_ATTACK,
            KEY_METAKEY,
            KEY_CLEAR_TARGET,
            KEY_TARGET_MONSTER,
            KEY_TARGET_NPC,
            KEY_TARGET_PLAYER,
            KEY_BEING_MENU,
            KEY_PICKUP,
            KEY_HIDE_WINDOWS,
            KEY_SIT,
            KEY_SCREENSHOT,
            KEY_TRADE,
            KEY_PATHFIND,
            KEY_WINDOW_CHAT,
            KEY_WINDOW_DEBUG,
            KEY_WINDOW_EMOTE,
            KEY_WINDOW_EMOTE_SHORTCUT,
            KEY_WINDOW_EQUIPMENT,
            KEY_WINDOW_HELP,
            KEY_WINDOW_INVENTORY,
            KEY_WINDOW_ITEM_SHORTCUT,
            KEY_WINDOW_MINIMAP,
            KEY_WINDOW_SETUP,
            KEY_WINDOW_SKILL,
            KEY_WINDOW_STATUS,
            KEY_EMOTE_SHORTCUT_1,
            KEY_EMOTE_SHORTCUT_2,
            KEY_EMOTE_SHORTCUT_3,
            KEY_EMOTE_SHORTCUT_4,
            KEY_EMOTE_SHORTCUT_5,
            KEY_EMOTE_SHORTCUT_6,
            KEY_EMOTE_SHORTCUT_7,
            KEY_EMOTE_SHORTCUT_8,
            KEY_EMOTE_SHORTCUT_9,
            KEY_EMOTE_SHORTCUT_10,
            KEY_EMOTE_SHORTCUT_11,
            KEY_EMOTE_SHORTCUT_12,
            KEY_ITEM_SHORTCUT_1,
            KEY_ITEM_SHORTCUT_2,
            KEY_ITEM_SHORTCUT_3,
            KEY_ITEM_SHORTCUT_4,
            KEY_ITEM_SHORTCUT_5,
            KEY_ITEM_SHORTCUT_6,
            KEY_ITEM_SHORTCUT_7,
            KEY_ITEM_SHORTCUT_8,
            KEY_ITEM_SHORTCUT_9,
            KEY_ITEM_SHORTCUT_10,
            KEY_ITEM_SHORTCUT_11,
            KEY_ITEM_SHORTCUT_12,
            KEY_TOGGLE_CHAT,
            KEY_SCROLL_CHAT_UP,
            KEY_SCROLL_CHAT_DOWN,
            KEY_QUIT,
            KEY_IGNORE_INPUT_1,
            KEY_IGNORE_INPUT_2,
            KEY_TOTAL
        };

        typedef std::pair<int, int> KeyPair;

        /**
         * Determines if any key assignments are the same as each other.
         * If conflict found, returns the pair of keys that conflict. If not,
         * it returns a key pair containing two KEY_NO_VALUEs.
         */
        KeyPair getConflicts();
    private:
        int mNewKeyIndex;              /**< Index of new key to be assigned */
        bool mEnabled;                 /**< Flag to respond to key input */
        bool mPrintableKeyLock;        /**< Flag to state whether a printable
                                            (character, number, punctuation, etc.)
                                            key input should be ignored */

        Setup_Input *mSetupKey;        /**< Reference to setup window */

        std::list<int> mLockedKeys;    /**< List containing all current
                                            GUIChan locked keys */

        KeyFunction mKey[KEY_TOTAL];   /**< Pointer to all the key data */

        uint8_t *mActiveKeys;            /**< Stores a list of all the keys */
};

extern KeyboardConfig keyboard;

#endif
