/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra derived from original code
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

#ifndef SHORTCUT_H
#define SHORTCUT_H

#define SHORTCUTS 12

#include <string>

class ShortcutHandler
{
    public:
        /**
         * Constructor.
         */
        ShortcutHandler(const std::string &prefix = "");

        /**
         * Destructor.
         */
        ~ShortcutHandler();

        /**
         * Load the configuration information.
         */
        void load();

        /**
         * Try to use the shortcut specified by the index.
         *
         * @param index Index of the shortcut.
         */
        virtual void useShortcut(int index) {}

        /**
         * Returns the amount of shortcut items.
         */
        static int getNumOfShortcuts() { return SHORTCUTS; }

        /**
         * A flag to check if the shortcut is selected.
         */
        bool isSelected() const { return mSelection > -1; }

        /**
         * Returns the currently selected shortcut ID.
         */
        int getSelected() const { return mSelection; }

        /**
         * Set the selected shortcut.
         *
         * @param id The shortcut's ID.
         */
        void setSelected(int id) { mSelection = id; }

        /**
         * Returns the shortcut ID specified by the index.
         *
         * @param index Index of the shortcut Emote.
         */
        int getShortcut(int index) const { return mIndex[index]; }

        /**
         * Adds the current selection to the specified shortcut index.
         *
         * @param index The shortcut index.
         */
        void setShortcut(int index) { mIndex[index] = mSelection; }

        /**
         * Adds the current selection to the specified shortcut index.
         *
         * @param index The shortcut index.
         * @param id The shortcut ID.
         */
        void setShortcut(int index, int id) { mIndex[index] = id; }

        /**
         * Remove a item from the shortcut.
         */
        void removeShortcut(int index) { mIndex[index] = -1; }

        /**
         * Gets the prefix string for saving configurations.
         */
        const std::string getPrefix() const { return mPrefix; } 

    protected:
        std::string mPrefix;         /**< Prefix to use when saving shortcuts */

        int mSelection;              /**< The currently selected shortcut. */
        int mIndex[SHORTCUTS];       /**< The emote stored. */

    private:
        /**
         * Save the configuration information.
         */
        void save() const;
};

#endif
