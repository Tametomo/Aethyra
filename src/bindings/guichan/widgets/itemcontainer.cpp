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

#include <guichan/font.hpp>
#include <guichan/mouseinput.hpp>
#include <guichan/selectionlistener.hpp>

#include "itemcontainer.h"

#include "../graphics.h"
#include "../gui.h"
#include "../palette.h"

#include "../sdl/sdlinput.h"

#include "../../../configlistener.h"
#include "../../../configuration.h"
#include "../../../inventory.h"
#include "../../../item.h"
#include "../../../itemshortcut.h"
#include "../../../log.h"

#include "../../../gui/itempopup.h"

#include "../../../resources/image.h"
#include "../../../resources/resourcemanager.h"

#include "../../../resources/sprite/localplayer.h"

#include "../../../utils/stringutils.h"

const int ItemContainer::gridWidth = 36;  // item icon width + 4
const int ItemContainer::gridHeight = 42; // item icon height + 10

static const int NO_ITEM = -1;

ItemContainerConfigListener *ItemContainer::mConfigListener = NULL;
Image *ItemContainer::mSelImg = NULL;
bool ItemContainer::mShowItemInfo = false;
int ItemContainer::mInstances = 0;
ItemPopup *ItemContainer::mItemPopup = NULL;
PopupMenu *ItemContainer::mPopupMenu = NULL;

class ItemContainerConfigListener : public ConfigListener
{
    public:
        ItemContainerConfigListener(ItemContainer *container):
            mItemContainer(container)
        {}

        void optionChanged(const std::string &name)
        {
            bool show = config.getValue("showItemPopups", true);

            if (name == "showItemPopups")
                mItemContainer->enableItemPopup(show);
        }
    private:
        ItemContainer *mItemContainer;
};

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

    if (mInstances == 0)
    {
        mItemPopup = new ItemPopup();
        mItemPopup->setOpaque(false);

        mShowItemInfo = config.getValue("showItemPopups", true);
        mConfigListener = new ItemContainerConfigListener(this);
        config.addListener("showItemPopups", mConfigListener);

        mPopupMenu = new PopupMenu(TRADE);

        ResourceManager *resman = ResourceManager::getInstance();

        mSelImg = resman->getImage("graphics/gui/selection.png");

        if (!mSelImg)
            logger->error("Unable to load selection.png");
    }

    mInstances++;

    mMaxItems = mInventory->getLastUsedSlot(); // Count from 0, usage from 2

    addFocusListener(this);
    addKeyListener(this);
    addMouseListener(this);
    addWidgetListener(this);

    setFocusable(true);
}

ItemContainer::~ItemContainer()
{
    mInstances--;

    if (mInstances == 0)
    {
        config.removeListener("showItemPopups", mConfigListener);
        delete mConfigListener;

        if (mSelImg)
            mSelImg->decRef();

        delete mItemPopup;
        delete mPopupMenu;
    }
}

void ItemContainer::logic()
{
    if (!isVisible())
        return;

    gcn::Widget::logic();

    if (mInventory->getNumberOfSlotsUsed() == 0)
        setEnabled(false);
    else
        setEnabled(true);

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
    int gridSlot = 0;   // The visible slot for drawing this item

    // Have at least 1 column
    if (columns < 1)
        columns = 1;

    for (int i = 0; i < mInventory->getSize(); i++)
    {
        Item *item = mInventory->getItem(i);

        if (!item || item->getQuantity() <= 0)
            continue;

        if (!passesFilter(item))
            continue;

        // Work out the object's position,
        int itemX = (gridSlot % columns) * gridWidth;
        int itemY = (gridSlot / columns) * gridHeight;

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

        // Move on to the next visible slot
        gridSlot++;
    }

    // If there are no visible items, make sure nothing is selected;
    // and inform the selection-listeners.
    if (!gridSlot)
        selectNone();
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
    Item* item = mInventory->getItem(index);

    if (item && passesFilter(item))
        newSelectedItemIndex = index;
    else
        newSelectedItemIndex = NO_ITEM;

    if (mSelectedItemIndex != newSelectedItemIndex)
    {
        mSelectedItemIndex = newSelectedItemIndex;

        if (mSelectedItemIndex == NO_ITEM)
            mLastSelectedItemId = NO_ITEM;
        else
            mLastSelectedItemId = item->getId();

        gcn::Rectangle scroll;
        const int columns = getWidth() / gridWidth;
        const int itemY = getVisibleSlot(item) / columns;

        if (mSelectedItemIndex == NO_ITEM)
            scroll.y = 0;
        else
            scroll.y = gridHeight * itemY;

        scroll.height = gridHeight;
        showPart(scroll);

        showItemPopup(mShowItemInfo);
        distributeValueChangedEvent();
    }
}

void ItemContainer::distributeValueChangedEvent()
{
    gcn::SelectionEvent event(this);
    std::list<gcn::SelectionListener*>::iterator i_end = mListeners.end();
    std::list<gcn::SelectionListener*>::iterator i;

    for (i = mListeners.begin(); i != i_end; ++i)
        (*i)->valueChanged(event);
}

void ItemContainer::showPopupMenu(MenuType type, bool useMouseCoordinates)
{
    int x = 0;
    int y = 0;

    getPopupLocation(useMouseCoordinates, x, y);

    Item *item = getSelectedItem();

    if (!item)
        return;

    mPopupMenu->setType(type);
    mPopupMenu->setItem(item);
    mPopupMenu->showPopup(x, y);
}

void ItemContainer::showItemPopup(bool show)
{
    Item *item = getSelectedItem();

    if (!item)
        return;

    if (item->getInfo().getName() != mItemPopup->getItemName())
        mItemPopup->setItem(item->getInfo());

    int x = 0;
    int y = 0;

    getPopupLocation(false, x, y);

    mItemPopup->updateColors();
    mItemPopup->view(x, y);
    mItemPopup->setVisible(mShowItemInfo ? show : false);
}

void ItemContainer::enableItemPopup(bool enable)
{
    if (!enable)
        mItemPopup->setVisible(false);

    mShowItemInfo = enable;
}

void ItemContainer::getPopupLocation(bool useMouseCoordinates, int &x, int &y)
{
    Item *item = getSelectedItem();
    x = gui->getMouseX();
    y = gui->getMouseY();

    if (!item)
        return;

    if (!useMouseCoordinates)
    {
        const int columns = getWidth() / gridWidth;
        const int gridSlot = getVisibleSlot(item);
        const int itemX = gridSlot % columns;
        const int itemY = gridSlot / columns;
        const int xPos = itemX * gridWidth + (gridWidth / 2);
        const int yPos = itemY * gridHeight + (gridHeight / 2) +
                         gui->getFont()->getHeight();

        x = xPos;
        y = yPos;

        gcn::Widget *widget = this;

        while (widget)
        {
            x += widget->getX();
            y += widget->getY();

            widget = widget->getParent();
        }
    }
}

void ItemContainer::keyPressed(gcn::KeyEvent &event)
{
    const int columns = getWidth() / gridWidth;
    const int gridSlot = getVisibleSlot(getSelectedItem());
    int itemX = gridSlot % columns;
    int itemY = gridSlot / columns;

    // Handling direction keys: all of these set selectNewItem, and change
    // itemX or itemY checking only that the selection doesn't go off the top,
    // left or right of the grid.  The block below the switch statement then
    // checks that there's an item in that slot (implictly bounds-checking that
    // the selection didn't go off the bottom of the grid).
    bool selectNewItem = false;
    switch (event.getKey().getValue())
    {
        case Key::LEFT:
            if (itemX != 0)
                itemX--;
                selectNewItem = true;
            break;
        case Key::RIGHT:
            if (itemX < (columns - 1))
                itemX++;
                selectNewItem = true;
            break;
        case Key::UP:
            if (itemY != 0)
                itemY--;
                selectNewItem = true;
            break;
        case Key::DOWN:
                itemY++;
                selectNewItem = true;
            break;
        case Key::ENTER:
        case Key::SPACE:
            if (event.isShiftPressed())
            {
                const std::string actionEventId = getActionEventId();
                setActionEventId("default");
                distributeActionEvent();
                setActionEventId(actionEventId);
            }
            else
                distributeActionEvent();
            break;
    }

    if (selectNewItem)
    {
        Item* selection =
            getItemInVisibleSlot(itemX + columns*itemY);
        if (selection)
            setSelectedItemIndex(selection->getInvIndex());
    }
}

void ItemContainer::mousePressed(gcn::MouseEvent &event)
{
    const int button = event.getButton();

    if (button == gcn::MouseEvent::LEFT || button == gcn::MouseEvent::RIGHT)
    {
        Item *item = getItem(event.getX(), event.getY());

        if (item)
        {
            setSelectedItemIndex(item->getInvIndex());
            itemShortcut->setItemSelected(item->getId());
        }
        else
        {
            setSelectedItemIndex(NO_ITEM);
            itemShortcut->setItemSelected(-1);
        }
    }
}

// Show ItemPopup
void ItemContainer::mouseMoved(gcn::MouseEvent &event)
{
    if (!mShowItemInfo || mPopupMenu->isVisible())
        return;

    Item *item = getItem(event.getX(), event.getY());

    if (item && passesFilter(item))
    {
        if (item->getInfo().getName() != mItemPopup->getItemName())
            mItemPopup->setItem(item->getInfo());

        mItemPopup->updateColors();
        mItemPopup->view(gui->getMouseX(), gui->getMouseY());
    }
    else
        mItemPopup->setVisible(false);
}

// Hide ItemPopup
void ItemContainer::mouseExited(gcn::MouseEvent &event)
{
    mItemPopup->setVisible(false);
}

void ItemContainer::focusGained(const gcn::Event &event)
{
    Item *item = getSelectedItem();

    if (mShowItemInfo && item && passesFilter(item))
        showItemPopup(true);
}

void ItemContainer::focusLost(const gcn::Event &event)
{
    mItemPopup->setVisible(false);
}

Item* ItemContainer::getItem(const int posX, const int posY)
{
    int columns = getWidth() / gridWidth;
    int gridSlot = posX / gridWidth + ((posY / gridHeight) * columns);
    return getItemInVisibleSlot(gridSlot);
}

Item* ItemContainer::getItemInVisibleSlot(const int gridSlot)
{
    int itemCount = -1;
    for (int i = 0; i < mInventory->getSize(); i++)
    {
        Item *item = mInventory->getItem(i);

        if (!item || item->getQuantity() <= 0)
            continue;

        if (!passesFilter(item))
            continue;

        itemCount++;

        if (itemCount == gridSlot)
            return item;
    }

    return NULL;
}

int ItemContainer::getVisibleSlot(const Item* searchItem) const
{
    int itemCount = -1;
    for (int i = 0; i < mInventory->getSize(); i++)
    {
        Item *item = mInventory->getItem(i);

        if (!item || item->getQuantity() <= 0)
            continue;

        if (!passesFilter(item))
            continue;

        itemCount++;

        if (searchItem == item)
            return itemCount;
    }

    return -1;
}

void ItemContainer::setTypeFilter(const std::string& type)
{
    mTypeFilter.clear();
    if (!type.empty())
    {
        // "" is special-cased to remove the filter
        mTypeFilter.push_back(type);
    }

    if (getSelectedItem() && !passesFilter(getSelectedItem()))
    {
        selectNone();
    }
}

void ItemContainer::setTypeFilter(const std::list<std::string>& types)
{
    mTypeFilter = types;

    if (getSelectedItem() && !passesFilter(getSelectedItem()))
    {
        selectNone();
    }
}

bool ItemContainer::passesFilter(const Item* item) const
{
    if (mTypeFilter.empty())
        return true;

    const std::string& itemType = item->getInfo().getType();
    for (std::list<std::string>::const_iterator iter = mTypeFilter.begin();
         iter != mTypeFilter.end(); iter++)
    {
        if (*iter == itemType)
            return true;
    }
    return false;
}
