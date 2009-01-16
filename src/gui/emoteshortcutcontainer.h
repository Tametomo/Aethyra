/*
 *  Aethyra
 *  Copyright 2009 Aethyra Development Team
 *
 *  This file is part of Aethyra.
 *
 *  Aethyra is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  Aethyra is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Aethyra; if not, write to the Free Software Foundation, 
 *  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _AETHYRA_EMOTESHORTCUTCONTAINER_H__
#define _AETHYRA_EMOTESHORTCUTCONTAINER_H__

#include <vector>

#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>

#include "shortcutcontainer.h"

#include "../guichanfwd.h"

class AnimatedSprite;
class Image;

/**
 * An emote shortcut container. Used to quickly use emoticons.
 *
 * \ingroup GUI
 */
class EmoteShortcutContainer : public ShortcutContainer
{
    public:
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

        /**
         * Handles mouse when dragged.
         */
        void mouseDragged(gcn::MouseEvent &event);

        /**
         * Handles mouse when pressed.
         */
        void mousePressed(gcn::MouseEvent &event);

        /**
         * Handles mouse release.
         */
        void mouseReleased(gcn::MouseEvent &event);

    private:
        std::vector<AnimatedSprite*> mEmoteImg;

        bool mEmoteClicked;
        int mEmoteMoved;
};

#endif