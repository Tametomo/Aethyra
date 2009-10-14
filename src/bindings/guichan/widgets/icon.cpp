/*
 *  Aethyra
 *  Copyright (C) 2008  The Mana World Development Team
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

#include "icon.h"

#include "../graphics.h"

#include "../../../core/resourcemanager.h"

#include "../../../core/image/image.h"

Icon::Icon(const std::string &file, bool fixed):
    mImage(NULL),
    mFixed(fixed)
{
    mImage = ResourceManager::getInstance()->getImage(file);
    fontChanged();
}

Icon::Icon(Image *image, bool fixed):
    mImage(image),
    mFixed(fixed)
{
    if (mImage)
    {
        mImage->incRef();
        setSize(mImage->getWidth(), mImage->getHeight());
    }
    else
        setSize(0, 0);
}

Icon::~Icon()
{
    if (mImage)
        mImage->decRef();
}

void Icon::setImage(Image *image)
{
    if (mImage)
        mImage->decRef();

    mImage = image;

    if (mImage)
    {
        mImage->incRef();

        if (!mFixed)
            setSize(mImage->getWidth(), mImage->getHeight());
    }
}

void Icon::draw(gcn::Graphics *g)
{
    if (mImage)
    {
        Graphics *graphics = static_cast<Graphics*>(g);
        const int x = (getWidth() - mImage->getWidth()) / 2;
        const int y = (getHeight() - mImage->getHeight()) / 2;
        graphics->drawImage(mImage, x, y);
    }
}

