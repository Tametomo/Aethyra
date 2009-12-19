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

#include "inventorywindow.h"
#include "itemamount.h"
#include "itempopup.h"
#include "storagewindow.h"
#include "trade.h"

#include "../structs/item.h"

#include "../../bindings/guichan/gui.h"
#include "../../bindings/guichan/layout.h"

#include "../../bindings/guichan/widgets/button.h"
#include "../../bindings/guichan/widgets/container.h"
#include "../../bindings/guichan/widgets/icon.h"
#include "../../bindings/guichan/widgets/label.h"
#include "../../bindings/guichan/widgets/slider.h"

#include "../../core/map/sprite/localplayer.h"

#include "../../core/utils/dtor.h"
#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

ItemAmountWindow::ItemAmountWindow(int usage, Window *parent, Item *item):
    Window("", true, parent),
    mItemAmountLabel(NULL),
    mItem(item),
    mItemIcon(NULL),
    mMax(item->getQuantity()),
    mUsage(usage),
    mItemPopup(NULL),
    mItemAmountSlide(NULL)
{
    // If only one item is available, then the window isn't needed, so move on
    if (mMax <= 1)
    {
        action(gcn::ActionEvent(this, "all"));
        return;
    }

    setCloseButton(true);

    // Integer field
    mItemAmountLabel = new Label(strprintf("%d / %d", 1, mMax));
    mItemAmountLabel->setAlignment(gcn::Graphics::CENTER);

    // Slider
    mItemAmountSlide = new Slider(1.0, mMax);
    mItemAmountSlide->setStepLength(1.0);
    mItemAmountSlide->setHeight(10);
    mItemAmountSlide->setActionEventId("slide");
    mItemAmountSlide->addActionListener(this);

    //Item icon
    Image *image = item->getImage();
    mItemIcon = new Icon(image);

    mItemPopup = new ItemPopup();
    mItemPopup->setOpaque(false);

    // Buttons
    mOkButton = new Button(_("OK"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);
    mAddAllButton = new Button(_("All"), "all", this);

    fontChanged();
    resetAmount();

    switch (usage)
    {
        case AMOUNT_TRADE_ADD:
            // trade as in trading an item
            setCaption(strprintf(_("Select amount of items to %s."), _("trade")));
            break;
        case AMOUNT_ITEM_DROP:
            // drop as in dropping an item
            setCaption(strprintf(_("Select amount of items to %s."), _("drop")));
            break;
        case AMOUNT_STORE_ADD:
            // store as in storing an item
            setCaption(strprintf(_("Select amount of items to %s."), _("store")));
            break;
        case AMOUNT_STORE_REMOVE:
            // retrieve as in retrieving an item
            setCaption(strprintf(_("Select amount of items to %s."), _("retrieve")));
            break;
        default:
            break;
    }

    setLocationRelativeTo(getParentWindow());
    setVisible(true);

    mItemIcon->addMouseListener(this);
    mItemAmountSlide->requestFocus();
}

ItemAmountWindow::~ItemAmountWindow()
{
    destroy(mItemPopup);
}

void ItemAmountWindow::fontChanged()
{
    Window::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    // Set positions
    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mItemIcon, 1, 3);
    place(1, 1, mItemAmountSlide, 5);
    place(6, 1, mItemAmountLabel, 2);
    place(8, 1, mAddAllButton);
    place = getPlacer(0, 3);
    place(5, 0, mCancelButton);
    place(6, 0, mOkButton);

    reflowLayout(225, 0);
    restoreFocus();
}

void ItemAmountWindow::mouseMoved(gcn::MouseEvent &event)
{
    Window::mouseMoved(event);

    // Show ItemTooltip
    if (event.getSource() == mItemIcon)
    {
        mItemPopup->setItem(mItem->getInfo());
        mItemPopup->updateColors();
        mItemPopup->view(gui->getMouseX(), gui->getMouseY());
    }
    else
        mItemPopup->setVisible(false);
}

void ItemAmountWindow::resetAmount()
{
    mItemAmountLabel->setCaption(strprintf("%d / %d", 1, mMax));
}

void ItemAmountWindow::action(const gcn::ActionEvent &event)
{
    int amount = 0;

    if (event.getId() == "slide")
    {
        amount = static_cast<int>(mItemAmountSlide->getValue());

        mItemAmountLabel->setCaption(strprintf("%d / %d", amount, mMax));
        mItemAmountSlide->setValue(amount);
        return;
    }
    else if (event.getId() == "ok" || event.getId() == "all")
    {
        if (event.getId() == "all") 
            amount = mMax;
        else
            amount = (int) mItemAmountSlide->getValue();

        switch (mUsage)
        {
            case AMOUNT_TRADE_ADD:
                tradeWindow->tradeItem(mItem, amount);
                break;
            case AMOUNT_ITEM_DROP:
                if (mItem->getQuantity() == amount)
                    inventoryWindow->selectNone();
                player_node->dropItem(mItem, amount);
                break;
            case AMOUNT_STORE_ADD:
                storageWindow->addStore(mItem, amount);
                break;
            case AMOUNT_STORE_REMOVE:
                if (mItem->getQuantity() == amount)
                    storageWindow->selectNone();
                storageWindow->removeStore(mItem, amount);
                break;
            default:
                break;
        }
    }

    close();
}

void ItemAmountWindow::close()
{
    getParentWindow()->requestFocus();
    windowContainer->scheduleDelete(this);
}
