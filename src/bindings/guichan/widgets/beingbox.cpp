/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
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

#include "beingbox.h"

#include "../graphics.h"

#include "../../../core/configlistener.h"
#include "../../../core/configuration.h"
#include "../../../core/resourcemanager.h"

#include "../../../core/image/image.h"

#include "../../../core/map/sprite/animatedsprite.h"
#include "../../../core/map/sprite/being.h"

#include "../../../core/utils/dtor.h"

int BeingBox::instances = 0;
float BeingBox::mAlpha = 1.0;
ImageRect BeingBox::background;
BeingBoxConfigListener *BeingBox::mConfigListener = NULL;

class BeingBoxConfigListener : public ConfigListener
{
    public:
        BeingBoxConfigListener(BeingBox *pb):
            mBeingBox(pb)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "guialpha")
            {
                mBeingBox->mAlpha = config.getValue("guialpha", 0.8);

                for (int a = 0; a < 9; a++)
                {
                    mBeingBox->background.grid[a]->setAlpha(mBeingBox->mAlpha);
                }
            }
        }
    private:
        BeingBox *mBeingBox;
};

BeingBox::BeingBox(const Being *being):
    mBeing(being)
{
    setFrameSize(2);

    if (instances == 0)
    {
        mAlpha = config.getValue("guialpha", 0.8);

        // Load the background skin
        ResourceManager *resman = ResourceManager::getInstance();
        Image *textbox = resman->getImage("graphics/gui/deepbox.png");
        int bggridx[4] = {0, 3, 28, 31};
        int bggridy[4] = {0, 3, 28, 31};
        int a = 0, x, y;

        for (y = 0; y < 3; y++)
        {
            for (x = 0; x < 3; x++)
            {
                background.grid[a] = textbox->getSubImage(
                        bggridx[x], bggridy[y],
                        bggridx[x + 1] - bggridx[x] + 1,
                        bggridy[y + 1] - bggridy[y] + 1);
                background.grid[a]->setAlpha(config.getValue("guialpha", 0.8));
                a++;
            }
        }

        textbox->decRef();

        mConfigListener = new BeingBoxConfigListener(this);
        config.addListener("guialpha", mConfigListener);
    }

    instances++;
}

BeingBox::~BeingBox()
{
    instances--;

    if (instances == 0)
    {
        config.removeListener("guialpha", mConfigListener);
        delete mConfigListener;

        for_each(background.grid, background.grid + 9, dtor<Image*>());
    }
}

void BeingBox::draw(gcn::Graphics *graphics)
{
    if (mBeing)
    {
        // Draw being
        int x, y, bs;
        bs = getFrameSize();
        x = getWidth() / 2 - 16 + bs;
        y = getHeight() / 2 + bs;
        for (int i = 0; i < Being::VECTOREND_SPRITE; i++)
        {
            if (mBeing->getSprite(i))
            {
                mBeing->getSprite(i)->draw(static_cast<Graphics*>(graphics), x, y);
            }
        }
    }
}

void BeingBox::drawFrame(gcn::Graphics *graphics)
{
    int w, h, bs;
    bs = getFrameSize();
    w = getWidth() + bs * 2;
    h = getHeight() + bs * 2;

    static_cast<Graphics*>(graphics)->drawImageRect(0, 0, w, h, background);
}
