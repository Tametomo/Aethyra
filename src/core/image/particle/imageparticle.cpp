/*
 *  Aethyra
 *  Copyright (C) 2006  The Mana World Development Team
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

#include "imageparticle.h"

#include "../image.h"

#include "../../../bindings/guichan/graphics.h"

ImageParticle::ImageParticle(Map *map, Image *image):
    Particle(map),
    mImage(image)
{
    if (mImage) mImage->incRef();
}

ImageParticle::~ImageParticle()
{
    if (mImage) mImage->decRef();
}

void ImageParticle::draw(Graphics *graphics, const int offsetX, const int offsetY) const
{
    if (!isAlive() || !mImage)
        return;

    const int screenX = (int) mPos.x + offsetX - mImage->getWidth() / 2;
    const int screenY = (int) mPos.y - (int) mPos.z + offsetY -
                         mImage->getHeight() / 2;

    // Check if on screen
    if (screenX + mImage->getWidth() < 0 || screenX > graphics->getWidth() ||
        screenY + mImage->getHeight() < 0 || screenY > graphics->getHeight())
    {
        return;
    }

    mImage->setAlpha(getCurrentAlpha());
    graphics->drawImage(mImage, screenX, screenY);
}
