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

#include "emoteshortcutcontainer.h"

#include "../db/emotedb.h"

#include "../handlers/emoteshortcut.h"

#include "../../bindings/guichan/graphics.h"
#include "../../bindings/guichan/palette.h"

#include "../../bindings/sdl/keyboardconfig.h"

#include "../../core/configlistener.h"
#include "../../core/configuration.h"

#include "../../core/image/image.h"

#include "../../core/map/sprite/animatedsprite.h"

#include "../../core/utils/dtor.h"

static const int MAX_ITEMS = 12;

std::vector<const AnimatedSprite*> EmoteShortcutContainer::mEmoteImg;
int EmoteShortcutContainer::mInstances = 0;
EmoteShortcutContainerConfigListener *EmoteShortcutContainer::mConfigListener = NULL;

class EmoteShortcutContainerConfigListener : public ConfigListener
{
    public:
        EmoteShortcutContainerConfigListener(EmoteShortcutContainer *container):
            mEmoteContainer(container)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "guialpha")
            {
                mEmoteContainer->mAlpha = config.getValue("guialpha", 0.8);
                mEmoteContainer->mBackgroundImg->setAlpha(mEmoteContainer->mAlpha);
            }
        }
    private:
        EmoteShortcutContainer *mEmoteContainer;
};

EmoteShortcutContainer::EmoteShortcutContainer():
    ShortcutContainer(emoteShortcut)
{
    if (mInstances == 0)
    {
        // Setup emote sprites
        for (int i = 0; i <= EmoteDB::getLast(); i++)
            mEmoteImg.push_back(EmoteDB::getAnimation(i));

        mConfigListener = new EmoteShortcutContainerConfigListener(this);
        config.addListener("guialpha", mConfigListener);
    }

    mInstances++;
}

EmoteShortcutContainer::~EmoteShortcutContainer()
{
    mInstances--;

    if (mInstances == 0)
    {
        config.removeListener("guialpha", mConfigListener);
        destroy(mConfigListener);
    }
}

void EmoteShortcutContainer::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    graphics->setFont(getFont());

    for (int i = 0; i < mShortcutHandler->getNumOfShortcuts(); i++)
    {
        const int emoteX = (i % mGridWidth) * mBoxWidth;
        const int emoteY = (i / mGridWidth) * mBoxHeight;
        const int emote = std::max(-1, mShortcutHandler->getShortcut(i) - 1);

        g->drawImage(mBackgroundImg, emoteX, emoteY);

        // Draw emote keyboard shortcut.
        const char *key = SDL_GetKeyName((SDLKey) keyboard.getKeyValue(
                                         keyboard.KEY_EMOTE_SHORTCUT_1 + i));
        graphics->setColor(guiPalette->getColor(Palette::TEXT));
        g->drawText(key, emoteX + 2, emoteY + 2, gcn::Graphics::LEFT);

        if (emote > -1 && emote <= EmoteDB::getLast() && mEmoteImg[emote])
           mEmoteImg[emote]->draw(g, emoteX + 2, emoteY + 10);
    }

    if (mShortcutDragged)
    {
        // Draw the emote being dragged by the cursor.
        const int emote = std::max(-1, mShortcutHandler->getSelected() - 1);

        if (emote > -1 && emote <= EmoteDB::getLast() && mEmoteImg[emote])
        {
            const int tPosX = mCursorPosX - (mEmoteImg[emote]->getWidth() / 2);
            const int tPosY = mCursorPosY - (mEmoteImg[emote]->getHeight() / 2);
            mEmoteImg[emote]->draw(g, tPosX, tPosY);
        }
    }
}

