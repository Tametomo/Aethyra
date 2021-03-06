/*
 *  Extended support for activating emotes
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

#ifndef EMOTESHORTCUTCONTAINER_H
#define EMOTESHORTCUTCONTAINER_H

#include <vector>

#include "../../bindings/guichan/widgets/shortcutcontainer.h"

class AnimatedSprite;
class EmoteShortcutContainerConfigListener;
class Image;

/**
 * An emote shortcut container. Used to quickly use emoticons.
 *
 * \ingroup GUI
 */
class EmoteShortcutContainer : public ShortcutContainer
{
    public:
        friend class EmoteShortcutContainerConfigListener;

        /**
         * Constructor. Initializes the graphic.
         */
        EmoteShortcutContainer();

        /**
         * Destructor.
         */
        virtual ~EmoteShortcutContainer();

        /**
         * Draws the items.
         */
        void draw(gcn::Graphics *graphics);

    private:
        static EmoteShortcutContainerConfigListener *mConfigListener;
        static int mInstances;

        static std::vector<const AnimatedSprite*> mEmoteImg;
};

#endif
