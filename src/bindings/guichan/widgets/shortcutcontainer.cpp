/*
 *  Aethyra
 *  Copyright 2007 The Mana World Development Team
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

#include "shortcutcontainer.h"

#include "../../../core/resourcemanager.h"

#include "../../../core/image/image.h"

int ShortcutContainer::mInstances = 0;
float ShortcutContainer::mAlpha = 1.0;
Image *ShortcutContainer::mBackgroundImg = NULL;
int ShortcutContainer::mBoxWidth = 0;
int ShortcutContainer::mBoxHeight = 0;

ShortcutContainer::ShortcutContainer(ShortcutHandler *shortcut):
    mShortcutHandler(shortcut),
    mGridWidth(1),
    mGridHeight(1),
    mShortcutClicked(false),
    mShortcutDragged(false)
{
    addMouseListener(this);
    addWidgetListener(this);

    if (mInstances == 0)
    {
        ResourceManager *resman = ResourceManager::getInstance();

        mBackgroundImg = resman->getImage("graphics/gui/item_shortcut_bgr.png");
        mBackgroundImg->setAlpha(mAlpha);

        mBoxHeight = mBackgroundImg->getHeight();
        mBoxWidth = mBackgroundImg->getWidth();
    }

    mInstances++;
}

ShortcutContainer::~ShortcutContainer()
{
    delete mShortcutHandler;
    mInstances--;

    if (mInstances == 0)
        mBackgroundImg->decRef();
}

void ShortcutContainer::widgetResized(const gcn::Event &event)
{
    const int shortcuts = mShortcutHandler->getNumOfShortcuts();

    mGridWidth = getWidth() > mBoxWidth ? getWidth() / mBoxWidth : 1;
    mGridHeight = mShortcutHandler->getNumOfShortcuts() / mGridWidth;

    if (shortcuts % mGridWidth != 0 || mGridHeight < 1)
        ++mGridHeight;

    setHeight(mGridHeight * mBoxHeight);
}

int ShortcutContainer::getIndexFromGrid(int pointX, int pointY) const
{
    const gcn::Rectangle tRect = gcn::Rectangle(0, 0, mGridWidth * mBoxWidth,
                                                mGridHeight * mBoxHeight);
    const int index = ((pointY / mBoxHeight) * mGridWidth) + pointX / mBoxWidth;

    return (index >= mShortcutHandler->getNumOfShortcuts() ||
           !tRect.isPointInRect(pointX, pointY)) ? -1 : index;
}

void ShortcutContainer::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (mShortcutClicked)
        {
            const int index = getIndexFromGrid(event.getX(), event.getY());

            mCursorPosX = event.getX();
            mCursorPosY = event.getY();

            if (!mShortcutDragged && index != -1)
            {
                const int id = mShortcutHandler->getShortcut(index);

                mShortcutHandler->setSelected(id);
                mShortcutHandler->removeShortcut(index);
                mShortcutDragged = true;
            }
        }
    }
}

void ShortcutContainer::mouseReleased(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        const int index = getIndexFromGrid(event.getX(), event.getY());

        if (index == -1)
        {
            mShortcutClicked = false;
            mShortcutDragged = false;
            mShortcutHandler->setSelected(-1);
            return;
        }

        if (mShortcutDragged)
        {
            mShortcutHandler->setShortcut(index, mShortcutHandler->getSelected());
            mShortcutHandler->setSelected(-1);
        }
        else if (mShortcutHandler->getShortcut(index) != -1 && mShortcutClicked)
            mShortcutHandler->useShortcut(index);

        mShortcutClicked = false;
        mShortcutDragged = false;

        if (mShortcutHandler->isSelected())
            mShortcutHandler->setSelected(-1);
    }
}
