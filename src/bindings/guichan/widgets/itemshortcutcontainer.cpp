/*
 *  Aethyra
 *  Copyright (C) 2007  The Mana World Development Team
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

#include "itemshortcutcontainer.h"

#include "../graphics.h"
#include "../gui.h"
#include "../keyboardconfig.h"
#include "../palette.h"

#include "../../../configlistener.h"
#include "../../../configuration.h"
#include "../../../inventory.h"
#include "../../../item.h"
#include "../../../itemshortcut.h"

#include "../../../gui/inventorywindow.h"
#include "../../../gui/itempopup.h"
#include "../../../gui/popupmenu.h"

#include "../../../resources/image.h"
#include "../../../resources/resourcemanager.h"

#include "../../../resources/sprite/localplayer.h"

#include "../../../utils/stringutils.h"

int ItemShortcutContainer::mInstances = 0;
bool ItemShortcutContainer::mShowItemInfo = false;
ItemPopup *ItemShortcutContainer::mItemPopup = NULL;
PopupMenu *ItemShortcutContainer::mPopupMenu = NULL;
ItemShortcutContainerConfigListener *ItemShortcutContainer::mConfigListener = NULL;

class ItemShortcutContainerConfigListener : public ConfigListener
{
    public:
        ItemShortcutContainerConfigListener(ItemShortcutContainer *container):
            mItemContainer(container)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "showItemPopups")
            {
                const bool enable = config.getValue("showItemPopups", true);
                mItemContainer->enableItemPopup(enable);
            }

            if (name == "guialpha")
            {
                mItemContainer->mAlpha = config.getValue("guialpha", 0.8);
                mItemContainer->mBackgroundImg->setAlpha(mItemContainer->mAlpha);
            }
        }
    private:
        ItemShortcutContainer *mItemContainer;
};

ItemShortcutContainer::ItemShortcutContainer():
    ShortcutContainer(),
    mItemClicked(false),
    mItemMoved(NULL)
{
    addMouseListener(this);
    addWidgetListener(this);

    if (mInstances == 0)
    {
        mAlpha = config.getValue("guialpha", 0.8);
        mShowItemInfo = config.getValue("showItemPopups", true);

        mItemPopup = new ItemPopup();
        mItemPopup->setOpaque(false);

        mPopupMenu = new PopupMenu(ITEM_SHORTCUT);

        ResourceManager *resman = ResourceManager::getInstance();

        mBackgroundImg = resman->getImage("graphics/gui/item_shortcut_bgr.png");
        mBackgroundImg->setAlpha(mAlpha);

        mConfigListener = new ItemShortcutContainerConfigListener(this);
        config.addListener("showItemPopups", mConfigListener);
        config.addListener("guialpha", mConfigListener);
    }

    mInstances++;

    mMaxItems = itemShortcut->getItemCount();

    mBoxHeight = mBackgroundImg->getHeight();
    mBoxWidth = mBackgroundImg->getWidth();
}

ItemShortcutContainer::~ItemShortcutContainer()
{
    mInstances--;

    if (mInstances == 0)
    {
        config.removeListener("showItemPopups", mConfigListener);
        config.removeListener("guialpha", mConfigListener);
        delete mConfigListener;

        mBackgroundImg->decRef();

        delete mItemPopup;
        delete mPopupMenu;
    }
}

void ItemShortcutContainer::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    graphics->setFont(getFont());

    for (int i = 0; i < mMaxItems; i++)
    {
        const int itemX = (i % mGridWidth) * mBoxWidth;
        const int itemY = (i / mGridWidth) * mBoxHeight;

        g->drawImage(mBackgroundImg, itemX, itemY);

        // Draw item keyboard shortcut.
        const char *key = SDL_GetKeyName(
            (SDLKey) keyboard.getKeyValue(keyboard.KEY_SHORTCUT_1 + i));
        graphics->setColor(guiPalette->getColor(Palette::TEXT));
        g->drawText(key, itemX + 2, itemY + 2, gcn::Graphics::LEFT);

        if (itemShortcut->getItem(i) < 0)
            continue;

        Item *item =
            player_node->getInventory()->findItem(itemShortcut->getItem(i));

        if (item)
        {
            // Draw item icon.
            Image* image = item->getImage();

            if (image)
            {
                const std::string label =
                    item->isEquipped() ? "Eq." : toString(item->getQuantity());

                graphics->setColor(guiPalette->getColor(item->isEquipped() ? 
                                   Palette::ITEM_EQUIPPED : Palette::TEXT));

                g->drawImage(image, itemX, itemY);
                g->drawText(label, itemX + mBoxWidth / 2,
                            itemY + mBoxHeight - 14, gcn::Graphics::CENTER);
            }
        }
    }

    if (mItemMoved)
    {
        // Draw the item image being dragged by the cursor.
        Image* image = mItemMoved->getImage();
        if (image)
        {
            const int tPosX = mCursorPosX - (image->getWidth() / 2);
            const int tPosY = mCursorPosY - (image->getHeight() / 2);

            graphics->setColor(guiPalette->getColor(mItemMoved->isEquipped() ? 
                               Palette::ITEM_EQUIPPED : Palette::TEXT));

            g->drawImage(image, tPosX, tPosY);
            g->drawText(toString(mItemMoved->getQuantity()),
                        tPosX + mBoxWidth / 2, tPosY + mBoxHeight - 14,
                        gcn::Graphics::CENTER);
        }
    }
}

void ItemShortcutContainer::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (!mItemMoved && mItemClicked)
        {
            const int index = getIndexFromGrid(event.getX(), event.getY());
            const int itemId = itemShortcut->getItem(index);

            if (index == -1 || itemId < 0)
                return;

            Item *item = player_node->getInventory()->findItem(itemId);

            if (item)
            {
                mItemMoved = item;
                itemShortcut->removeItem(index);
            }
        }
        if (mItemMoved)
        {
            mCursorPosX = event.getX();
            mCursorPosY = event.getY();
        }
    }
}

void ItemShortcutContainer::mousePressed(gcn::MouseEvent &event)
{
    const int index = getIndexFromGrid(event.getX(), event.getY());

    if (index == -1)
        return;

    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        // Stores the selected item if theirs one.
        if (itemShortcut->isItemSelected() && inventoryWindow->isVisible())
        {
            itemShortcut->setItem(index);
            itemShortcut->setItemSelected(-1);
        }
        else if (itemShortcut->getItem(index))
            mItemClicked = true;
    }
    else if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Item *item = player_node->getInventory()->
                     findItem(itemShortcut->getItem(index));

        if (!item)
            return;

        mPopupMenu->setItem(item);
        mPopupMenu->showPopup(gui->getMouseX(), gui->getMouseY());
    }
}

void ItemShortcutContainer::mouseReleased(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        if (itemShortcut->isItemSelected())
            itemShortcut->setItemSelected(-1);

        const int index = getIndexFromGrid(event.getX(), event.getY());
        if (index == -1)
        {
            mItemMoved = NULL;
            return;
        }
        if (mItemMoved)
        {
            itemShortcut->setItems(index, mItemMoved->getId());
            mItemMoved = NULL;
        }
        else if (itemShortcut->getItem(index) && mItemClicked)
        {
            itemShortcut->useItem(index);
        }

        if (mItemClicked)
            mItemClicked = false;
    }
}

// Show ItemTooltip
void ItemShortcutContainer::mouseMoved(gcn::MouseEvent &event)
{
    if (!mShowItemInfo)
        return;

    const int index = getIndexFromGrid(event.getX(), event.getY());
    const int itemId = itemShortcut->getItem(index);

    if (index == -1 || itemId < 0)
        return;

    Item *item = player_node->getInventory()->findItem(itemId);

    if (item)
    {
        if (item->getInfo().getName() != mItemPopup->getItemName())
            mItemPopup->setItem(item->getInfo());
        mItemPopup->updateColors();
        mItemPopup->view(gui->getMouseX(), gui->getMouseY());
    }
    else
    {
        mItemPopup->setVisible(false);
    }
}

// Hide ItemTooltip
void ItemShortcutContainer::mouseExited(gcn::MouseEvent &event)
{
    mItemPopup->setVisible(false);
}

void ItemShortcutContainer::enableItemPopup(bool enable)
{
    mShowItemInfo = enable;

    if (!enable)
        mItemPopup->setVisible(false);
}

