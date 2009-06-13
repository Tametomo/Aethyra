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

#include "imagebutton.h"

#include "../graphics.h"

#include "../../../configuration.h"

#include "../../../resources/image.h"
#include "../../../resources/resourcemanager.h"

ImageButton::ImageButton(const std::string &image,
                         const std::string &actionEventId,
                         gcn::ActionListener *listener, unsigned int padding):
    Button(),
    mPadding(padding)
{
    mImage = ResourceManager::getInstance()->getImage(image);

    if (mImage)
        setSize(mImage->getWidth() + 2 * mPadding, mImage->getHeight() + 2 * mPadding);
    else
        setSize(2 * mPadding, 2 * mPadding);

    setActionEventId(actionEventId);

    if (listener)
        addActionListener(listener);
}

ImageButton::ImageButton(Image *image, const std::string &actionEventId,
                         gcn::ActionListener *listener, unsigned int padding):
    Button(),
    mPadding(padding)
{
    mImage = image;

    if (mImage)
    {
        mImage->incRef();
        setSize(mImage->getWidth() + 2 * mPadding, mImage->getHeight() + 2 * mPadding);
    }
    else
        setSize(2 * mPadding, 2 * mPadding);

    setActionEventId(actionEventId);

    if (listener)
        addActionListener(listener);
}

ImageButton::~ImageButton()
{
    if (mImage)
        mImage->decRef();
}

void ImageButton::changeImage(Image *image)
{
    if (mImage)
        mImage->decRef();

    mImage = image;

    if (mImage)
    {
        mImage->incRef();
        setSize(mImage->getWidth() + 2 * mPadding, mImage->getHeight() + 2 * mPadding);
    }
    else
        setSize(2 * mPadding, 2 * mPadding);
}

void ImageButton::changeImage(const std::string &image)
{
    if (mImage)
        mImage->decRef();

    mImage = ResourceManager::getInstance()->getImage(image);

    if (mImage)
        setSize(mImage->getWidth() + 2 * mPadding, mImage->getHeight() + 2 * mPadding);
    else
        setSize(2 * mPadding, 2 * mPadding);
}

void ImageButton::draw(gcn::Graphics *graphics)
{
    Button::draw(graphics);

    if (!mImage)
        return;

    if (mImage->getAlpha() != mAlpha)
        mImage->setAlpha(mAlpha);

    const int x = (getWidth() - mImage->getWidth()) / 2;
    const int y = (getHeight() - mImage->getHeight()) / 2;

    static_cast<Graphics*>(graphics)->drawImage(mImage, x, y);
}
