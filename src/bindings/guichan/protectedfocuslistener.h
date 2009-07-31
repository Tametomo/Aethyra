/*
 *  Aethyra
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

#ifndef PROTECTEDFOCUSLISTENER_H
#define PROTECTEDFOCUSLISTENER_H

#include <list>

#include <guichan/focuslistener.hpp>

#include "sdl/sdlinput.h"

/**
 * A Focus listener implementation which locks keys which the widget which
 * derives from this listener uses.
 */
class ProtectedFocusListener : public gcn::FocusListener
{
    public:
        /**
         * Constructor.
         */
        ProtectedFocusListener() : mPrintableKeyLock(false) {}

        /**
         * Locks keyboard keys that the using widget will use.
         */
        virtual void focusGained(const gcn::Event &event);

        /**
         * Unlocks any keys which this listener has locked.
         */
        virtual void focusLost(const gcn::Event &event);

        /**
         * Sets whether to lock printable keys from input or not.
         */
        void setPrintableKeyLock(bool lock) { mPrintableKeyLock = lock; }

        /**
         * Adds a key to this listener's block list
         */
        void blockKey(const int key);

        /**
         * Removes a key to this listener's block list
         */
        void unblockKey(const int key);

    private:
        bool mPrintableKeyLock;        /**< Flag to state whether a printable
                                            (character, number, punctuation, etc.)
                                            key input should be ignored */

        std::list<int> mBlockList;    /**< List containing keys which the class
                                           using this listener should block on
                                           focus */
};

#endif
