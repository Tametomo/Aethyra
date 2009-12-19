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

#include "resizegrip.h"

#include "../graphics.h"

#include "../../../core/configlistener.h"
#include "../../../core/configuration.h"
#include "../../../core/resourcemanager.h"

#include "../../../core/image/image.h"

#include "../../../core/utils/dtor.h"

Image *ResizeGrip::mGripImage = NULL;
int ResizeGrip::mInstances = 0;

float ResizeGrip::mAlpha = 1.0;
ResizeGripConfigListener *ResizeGrip::mConfigListener = NULL;

class ResizeGripConfigListener : public ConfigListener
{
    public:
        ResizeGripConfigListener(ResizeGrip *rg):
            mResizeGrip(rg)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "guialpha")
            {
                mResizeGrip->mAlpha = config.getValue("guialpha", 0.8);

                mResizeGrip->mGripImage->setAlpha(mResizeGrip->mAlpha);
            }
        }
    private:
        ResizeGrip *mResizeGrip;
};

ResizeGrip::ResizeGrip()
{
    if (mInstances == 0)
    {
        mAlpha = config.getValue("guialpha", 0.8);

        // Load the grip image
        ResourceManager *resman = ResourceManager::getInstance();
        mGripImage = resman->getImage("graphics/gui/resize.png");
        mGripImage->setAlpha(mAlpha);

        mConfigListener = new ResizeGripConfigListener(this);
        config.addListener("guialpha", mConfigListener);
    }

    mInstances++;

    setWidth(mGripImage->getWidth() + 2);
    setHeight(mGripImage->getHeight() + 2);
}

ResizeGrip::~ResizeGrip()
{
    mInstances--;

    if (mInstances == 0)
    {
        config.removeListener("guialpha", mConfigListener);
        destroy(mConfigListener);

        mGripImage->decRef();
    }
}

void ResizeGrip::draw(gcn::Graphics *graphics)
{
    static_cast<Graphics*>(graphics)->drawImage(mGripImage, 0, 0);
}
