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

#include <guichan/mouseinput.hpp>
#include <guichan/selectionlistener.hpp>

#include "itemcontainer.h"

#include "../graphics.h"
#include "../palette.h"

#include "../sdl/sdlinput.h"

#include "../../../inventory.h"
#include "../../../item.h"
#include "../../../itemshortcut.h"
#include "../../../log.h"

#include "../../../gui/itempopup.h"
#include "../../../gui/viewport.h"

#include "../../../resources/image.h"
#include "../../../resources/resourcemanager.h"

#include "../../../resources/sprite/localplayer.h"

#include "../../../utils/stringutils.h"

const int ItemContainer::gridWidth = 36;  // item icon width + 4
const int ItemContainer::gridHeight = 42; // item icon height + 10

static const int NO_ITEM = -1;

ItemContainer::ItemContainer(Inventory *inventory,
                             const std::string &actionEventId,
                             gcn::ActionListener *listener):
    mInventory(inventory),
    mSelectedItemIndex(NO_ITEM),
    mLastSelectedItemId(NO_ITEM)
{
    if (!actionEventId.empty())
        setActionEventId(actionEventId);

    if (listener && !actionEventId.empty())
        addActionListener(listener);

    mItemPopup = new ItemPopup();
    mItemPopup->setOpaque(false);
    setFocusable(true);

    ResourceManager *resman = ResourceManager::getInstance();

    mSelImg = resman->getImage("graphics/gui/selection.png");
    if (!mSelImg) logger->error("Unable to load selection.png");

    mMaxItems = mInventory->getLastUsedSlot(); // Count from 0, usage from 2

    addKeyListener(this);
    addMouseListener(this);
    addWidgetListener(this);
}

ItemContainer::~ItemContainer()
{
    mSelImg->decRef();
    delete mItemPopup;
}

void ItemContainer::logic()
{
    if (!isVisible())
        return;

    gcn::Widget::logic();

    int i = mInventory->getLastUsedSlot();

    if (i != mMaxItems)
    {
        mMaxItems = i;
        recalculateHeight();
    }
}

void ItemContainer::draw(gcn::Graphics *graphics)
{
    if (!isVisible())
        return;

    int columns = getWidth() / gridWidth;

    // Have at least 1 column
    if (columns < 1)
        columns = 1;

    for (int i = 0; i < mInventory->getSize(); i++)
    {
        Item *item = mInventory->getItem(i);

        if (!item || item->getQuantity() <= 0)
            continue;

        int itemX = (i % columns) * gridWidth;
        int itemY = (i / columns) * gridHeight;

        // Draw selection image below selected item
        if (mSelectedItemIndex == i)
            static_cast<Graphics*>(graphics)->drawImage(mSelImg, itemX, itemY);

        // Draw item icon
        Image* image = item->getImage();

        if (image)
            static_cast<Graphics*>(graphics)->drawImage(image, itemX, itemY);

        // Draw item caption
        graphics->setFont(getFont());
        graphics->setColor(guiPalette->getColor(item->isEquipped() ? 
                               Palette::ITEM_EQUIPPED : Palette::TEXT));
        graphics->drawText(
                (item->isEquipped() ? "Eq." : toString(item->getQuantity())),
                itemX + gridWidth / 2, itemY + gridHeight - 11,
                gcn::Graphics::CENTER);
    }
}

void ItemContainer::widgetResized(const gcn::Event &event)
{
    recalculateHeight();
}

void ItemContainer::recalculateHeight()
{
    int cols = getWidth() / gridWidth;

    if (cols < 1)
        cols = 1;

    const int rows = ((mMaxItems + 1) / cols) + ((mMaxItems + 1) % cols > 0 ? 1 : 0);
    const int height = rows * gridHeight + 8;

    if (height != getHeight())
        setHeight(height);
}

Item *ItemContainer::getSelectedItem()
{
    refindSelectedItem(); // Make sure that we're still current

    if (mSelectedItemIndex == NO_ITEM)
        return NULL;

    return mInventory->getItem(mSelectedItemIndex);
}

void ItemContainer::selectNone()
{
    setSelectedItemIndex(NO_ITEM);
}

void ItemContainer::refindSelectedItem()
{
    if (mSelectedItemIndex != NO_ITEM)
    {

        if (mInventory->getItem(mSelectedItemIndex) &&
            mInventory->getItem(mSelectedItemIndex)->getId() == mLastSelectedItemId)
            return; // we're already fine

        // Otherwise ensure the invariant: we must point to an item of the specified last ID,
        // or nowhere at all.

        for (int i = 0; i < mMaxItems; i++)
            if (mInventory->getItem(i) &&
                mInventory->getItem(i)->getId() == mLastSelectedItemId)
            {
                mSelectedItemIndex = i;
                return;
            }
    }

    mLastSelectedItemId = mSelectedItemIndex = NO_ITEM;
}

void ItemContainer::setSelectedItemIndex(int index)
{
    int newSelectedItemIndex;

    if (index < 0 || index > mMaxItems || mInventory->getItem(index) == NULL)
        newSelectedItemIndex = NO_ITEM;
    else
        newSelectedItemIndex = index;
    if (mSelectedItemIndex != newSelectedItemIndex)
    {
        mSelectedItemIndex = newSelectedItemIndex;

        if (mSelectedItemIndex == NO_ITEM)
            mLastSelectedItemId = NO_ITEM;
        else
            mLastSelectedItemId = mInventory->getItem(index)->getId();

        distributeValueChangedEvent();
    }
}

void ItemContainer::distributeValueChangedEvent()
{
    gcn::SelectionEvent event(this);
    std::list<gcn::SelectionListener*>::iterator i_end = mListeners.end();
    std::list<gcn::SelectionListener*>::iterator i;

    for (i = mListeners.begin(); i != i_end; ++i)
    {
        (*i)->valueChanged(event);
    }
}

void ItemContainer::keyPressed(gcn::KeyEvent &event)
{
    int columns = getWidth() / gridWidth;
    const int rows = mInventory->getNumberOfSlotsUsed() / columns;
    const int itemX = mSelectedItemIndex % columns;
    const int itemY = mSelectedItemIndex / columns;

    if (columns > mInventory->getNumberOfSlotsUsed())
        columns = mInventory->getNumberOfSlotsUsed();

    switch (event.getKey().getValue())
    {
        case Key::LEFT:
            if (itemX != 0)
                setSelectedItemIndex((itemY * columns) + itemX - 1);
            break;
        case Key::RIGHT:
            if (itemX < (columns - 1) &&
               ((itemY * columns) + itemX + 1) < mInventory->getNumberOfSlotsUsed())
                setSelectedItemIndex((itemY * columns) + itemX + 1);
            break;
        case Key::UP:
            if (itemY != 0)
                setSelectedItemIndex(((itemY - 1) * columns) + itemX);
            break;
        case Key::DOWN:
            if (itemY < rows &&
               (((itemY + 1) * columns) + itemX) < mInventory->getNumberOfSlotsUsed())
                setSelectedItemIndex(((itemY + 1) * columns) + itemX);
            break;
        case Key::ENTER:
            distributeActionEvent();
            break;
    }
}

void ItemContainer::mousePressed(gcn::MouseEvent &event)
{
    const int button = event.getButton();

    if (button == gcn::MouseEvent::LEFT || button == gcn::MouseEvent::RIGHT)
    {
        int columns = getWidth() / gridWidth;
        int mx = event.getX();
        int my = event.getY();
        int index = mx / gridWidth + ((my / gridHeight) * columns);

        itemShortcut->setItemSelected(-1);
        setSelectedItemIndex(index);

        Item *item = mInventory->getItem(index);

        if (item)
            itemShortcut->setItemSelected(item->getId());
    }
}

// Show ItemTooltip
void ItemContainer::mouseMoved(gcn::MouseEvent &event)
{
    Item *item = mInventory->getItem(getSlotIndex(event.getX(), event.getY()));

    if (item)
    {
        if (item->getInfo().getName() != mItemPopup->getItemName())
            mItemPopup->setItem(item->getInfo());
        mItemPopup->updateColors();
        mItemPopup->view(viewport->getMouseX(), viewport->getMouseY());
    }
    else
    {
        mItemPopup->setVisible(false);
    }
}

// Hide ItemTooltip
void ItemContainer::mouseExited(gcn::MouseEvent &event)
{
    mItemPopup->setVisible(false);
}

int ItemContainer::getSlotIndex(const int posX, const int posY) const
{
    int columns = getWidth() / gridWidth;
    int index = posX / gridWidth + ((posY / gridHeight) * columns);

    return (index);
}

