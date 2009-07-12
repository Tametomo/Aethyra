/*
 *  Extended support for activating emotes
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

#ifndef EMOTESHORTCUT_H
#define EMOTESHORTCUT_H

#include "../../bindings/guichan/handlers/shortcuthandler.h"

#include "../../core/image/sprite/localplayer.h"

/**
 * A handler which keeps track of emote shortcuts.
 */
class EmoteShortcut : public ShortcutHandler
{
    public:
        /**
         * Constructor.
         */
        EmoteShortcut() : ShortcutHandler("emoteshortcut") {}

        /**
         * Try to use the emote specified by the index.
         *
         * @param index Index of the emote.
         */
        void useShortcut(int index)
        {
            if (index > -1 && index <= SHORTCUTS)
                player_node->emote(mIndex[index]);
        }
};

extern EmoteShortcut *emoteShortcut;

#endif
