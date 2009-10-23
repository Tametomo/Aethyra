/*
 *  Aethyra
 *  Copyright (C) 2007  The Mana World Development Team
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra derived from original code from The Mana World.
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

#include "../structs/inventory.h"
#include "../structs/item.h"

#include "../gui/popupmenu.h"

#include "../handlers/itemshortcut.h"

#include "../../bindings/guichan/graphics.h"
#include "../../bindings/guichan/gui.h"
#include "../../bindings/guichan/palette.h"

#include "../../bindings/sdl/keyboardconfig.h"

#include "../../core/configlistener.h"
#include "../../core/configuration.h"

#include "../../core/image/image.h"

#include "../../core/map/sprite/localplayer.h"

#include "../../core/utils/stringutils.h"

int ItemShortcutContainer::mInstances = 0;
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
    ShortcutContainer(itemShortcut)
{
    if (mInstances == 0)
    {
        mAlpha = config.getValue("guialpha", 0.8);

        mPopupMenu = new PopupMenu(ITEM_SHORTCUT);

        mConfigListener = new ItemShortcutContainerConfigListener(this);
        config.addListener("guialpha", mConfigListener);
    }

    mInstances++;
}

ItemShortcutContainer::~ItemShortcutContainer()
{
    mInstances--;

    if (mInstances == 0)
    {
        config.removeListener("guialpha", mConfigListener);

        delete mConfigListener;
        delete mPopupMenu;
    }
}

void ItemShortcutContainer::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    graphics->setFont(getFont());

    for (int i = 0; i < mShortcutHandler->getNumOfShortcuts(); i++)
    {
        const int itemX = (i % mGridWidth) * mBoxWidth;
        const int itemY = (i / mGridWidth) * mBoxHeight;

        g->drawImage(mBackgroundImg, itemX, itemY);

        // Draw item keyboard shortcut.
        const char *key = SDL_GetKeyName(
            (SDLKey) keyboard.getKeyValue(keyboard.KEY_ITEM_SHORTCUT_1 + i));
        graphics->setColor(guiPalette->getColor(Palette::TEXT));
        g->drawText(key, itemX + 2, itemY + 2, gcn::Graphics::LEFT);

        if (mShortcutHandler->getShortcut(i) < 0)
            continue;

        Item *item =
            player_node->getInventory()->findItem(mShortcutHandler->getShortcut(i));

        if (item)
        {
            // Draw item icon.
            Image* image = item->getImage();

            if (image)
            {
                const std::string label = item->isEquipped() ? "Eq." :
                                          toString(item->getQuantity());

                graphics->setColor(guiPalette->getColor(item->isEquipped() ? 
                                   Palette::ITEM_EQUIPPED : Palette::TEXT));

                g->drawImage(image, itemX, itemY);
                g->drawText(label, itemX + mBoxWidth / 2,
                            itemY + mBoxHeight - 14, gcn::Graphics::CENTER);
            }
        }
    }

    if (mShortcutDragged)
    {
        // Draw the item image being dragged by the cursor.
        const int itemId = mShortcutHandler->getSelected();
        Item* item = player_node->getInventory()->findItem(itemId);

        if (!item)
            return;

        Image* image = item->getImage();
        if (image)
        {
            const int tPosX = mCursorPosX - (image->getWidth() / 2);
            const int tPosY = mCursorPosY - (image->getHeight() / 2);
            const std::string label = item->isEquipped() ? "Eq." :
                                      toString(item->getQuantity());

            graphics->setColor(guiPalette->getColor(item->isEquipped() ? 
                               Palette::ITEM_EQUIPPED : Palette::TEXT));

            g->drawImage(image, tPosX, tPosY);
            g->drawText(label, tPosX + mBoxWidth / 2, tPosY + mBoxHeight - 14,
                        gcn::Graphics::CENTER);
        }
    }
}

void ItemShortcutContainer::mousePressed(gcn::MouseEvent &event)
{
    ShortcutContainer::mousePressed(event);

    const int index = getIndexFromGrid(event.getX(), event.getY());

    if (index == -1)
        return;

    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Item *item = player_node->getInventory()->
                     findItem(mShortcutHandler->getShortcut(index));

        if (!item)
            return;

        mPopupMenu->setItem(item);
        mPopupMenu->showPopup(gui->getMouseX(), gui->getMouseY());
    }
}

