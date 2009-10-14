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

#include <string>

#include <guichan/font.hpp>
#include <guichan/mouseinput.hpp>

#include "inventorywindow.h"
#include "itemamount.h"
#include "storagewindow.h"
#include "trade.h"

#include "../db/iteminfo.h"

#include "../handlers/itemshortcut.h"

#include "../structs/inventory.h"
#include "../structs/item.h"

#include "../widgets/itemcontainer.h"

#include "../../bindings/guichan/layout.h"

#include "../../bindings/guichan/widgets/button.h"
#include "../../bindings/guichan/widgets/label.h"
#include "../../bindings/guichan/widgets/progressbar.h"
#include "../../bindings/guichan/widgets/scrollarea.h"

#include "../../core/map/sprite/localplayer.h"

#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

extern Window *equipmentWindow;
extern Window *itemShortcutWindow;

InventoryWindow::InventoryWindow(int invSize):
    Window(_("Inventory")),
    mMaxSlots(invSize),
    mItemDesc(false)
{
    setWindowName("Inventory");
    setResizable(true);
    setCloseButton(true);

    setDefaultSize(375, 300, ImageRect::CENTER);

    mTradeButton = new Button(_("Trade"), "trade", this);
    mTradeButton->setEnabled(false);
    mTradeButton->setVisible(false);

    mStoreButton = new Button(_("Store"), "store", this);
    mStoreButton->setEnabled(false);
    mStoreButton->setVisible(false);

    mUseButton = new Button(_("Use"), "use", this);
    mUseButton->setEnabled(false);

    mDropButton = new Button(_("Drop"), "drop", this);
    mDropButton->setEnabled(false);

    mShortcutButton = new Button(_("Shortcuts"), "shortcuts", this);

    mItems = new ItemContainer(player_node->getInventory(), "showpopupmenu", this);
    mItems->addSelectionListener(this);

    mInvenScroll = new ScrollArea(mItems);
    mInvenScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mTotalWeight = player_node->mTotalWeight;
    mMaxWeight = player_node->mMaxWeight;
    mUsedSlots = player_node->getInventory()->getNumberOfSlotsUsed();

    mSlotsLabel = new Label(_("Slots: "));
    mWeightLabel = new Label(_("Weight: "));

    mSlotsBar = new ProgressBar(0.0f, 100, 20, gcn::Color(225, 200, 25));
    mWeightBar = new ProgressBar(0.0f, 100, 20, gcn::Color(0, 0, 255));
    mWeightBar->addColor(255, 255, 0);
    mWeightBar->addColor(255, 0, 0);

    fontChanged();
    loadWindowState();
}

void InventoryWindow::fontChanged()
{
    Window::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    setMinHeight(130);
    setMinWidth(mWeightLabel->getWidth() + mSlotsLabel->getWidth() + 260);

    place(0, 0, mWeightLabel).setPadding(3);
    place(1, 0, mWeightBar, 5);
    place(6, 0, mSlotsLabel).setPadding(3);
    place(7, 0, mSlotsBar, 2);
    place(0, 1, mInvenScroll, 9, 4);
    place(0, 5, mShortcutButton);
    place(5, 5, mStoreButton);
    place(6, 5, mTradeButton);
    place(7, 5, mDropButton);
    place(8, 5, mUseButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, mDropButton->getHeight());
}

void InventoryWindow::logic()
{
    if (!isVisible())
        return;

    Window::logic();

    const int usedSlots = player_node->getInventory()->getNumberOfSlotsUsed();

    if (mMaxWeight != player_node->mMaxWeight ||
        mTotalWeight != player_node->mTotalWeight)
    {
        mTotalWeight = player_node->mTotalWeight;
        mMaxWeight = player_node->mMaxWeight;

        // Adjust weight progress bar
        mWeightBar->setProgress((float) mTotalWeight / mMaxWeight);
        mWeightBar->setText(strprintf("%dg/%dg", mTotalWeight, mMaxWeight));
    }

    if (mUsedSlots != usedSlots)
    {
        mUsedSlots = usedSlots;

        mSlotsBar->setProgress((float) mUsedSlots / mMaxSlots);
        mSlotsBar->setText(strprintf("%d/%d", mUsedSlots, mMaxSlots));
    }

    mStoreButton->setVisible(storageWindow->isVisible());
    mTradeButton->setVisible(tradeWindow->isVisible());
}

void InventoryWindow::distributeValueChangedEvent()
{
    gcn::SelectionEvent event(this);
    std::list<gcn::SelectionListener*>::iterator i_end = mListeners.end();
    std::list<gcn::SelectionListener*>::iterator i;

    for (i = mListeners.begin(); i != i_end; ++i)
        (*i)->valueChanged(event);
}

void InventoryWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "shortcuts")
    {
        itemShortcutWindow->setVisible(!itemShortcutWindow->isVisible());
        return;
    }

    Item *item = mItems->getSelectedItem();

    if (!item)
        return;

    if (event.getId() == "trade" && tradeWindow->canTrade())
    {
        if (item->getQuantity() == 1)
        {
            tradeWindow->tradeItem(item, 1);
            selectNone();
        }
        // Choose amount of items to trade
        else
            new ItemAmountWindow(AMOUNT_TRADE_ADD, tradeWindow, item);
    }
    else if (event.getId() == "store" && storageWindow->isVisible())
    {
        if (item->getQuantity() == 1)
        {
            storageWindow->addStore(item, 1);
            selectNone();
        }
        // Choose amount of items to store
        else
            new ItemAmountWindow(AMOUNT_STORE_ADD, storageWindow, item);
    }
    else if (event.getId() == "use" || event.getId() == "default")
    {
        if (item->isEquipment())
        {
            if (item->isEquipped())
                player_node->unequipItem(item);
            else
                player_node->equipItem(item);
        }
        else
        {
            if (item->getQuantity() == 1)
                selectNone();

            player_node->useItem(item);
        }
    }
    else if (event.getId() == "drop")
    {
        if (item->getQuantity() == 1)
        {
            player_node->dropItem(item, 1);
            selectNone();
        }
        // Choose amount of items to drop
        else
            new ItemAmountWindow(AMOUNT_ITEM_DROP, this, item);
    }
    else if (event.getId() == "showpopupmenu")
        mItems->showPopupMenu(INVENTORY, false);
}

void InventoryWindow::mouseClicked(gcn::MouseEvent &event)
{
    Window::mouseClicked(event);

    if (event.getButton() == gcn::MouseEvent::RIGHT &&
        event.getSource() == mItems)
        mItems->showPopupMenu(INVENTORY);
}

void InventoryWindow::updateButtons()
{
    const Item *selectedItem = mItems->getSelectedItem();
    const std::string &caption = (selectedItem && selectedItem->isEquipment()) ?
                                 (selectedItem->isEquipped()) ? _("Unequip") :
                                 _("Equip") : _("Use");

    mUseButton->setCaption(caption);
    mUseButton->setEnabled(selectedItem != NULL);
    mDropButton->setEnabled(selectedItem != NULL);
    mStoreButton->setEnabled(selectedItem != NULL);
    mTradeButton->setEnabled(selectedItem != NULL && tradeWindow->canTrade() &&
                             !tradeWindow->tradingItem(selectedItem));
    fontChanged();
}

Item* InventoryWindow::getSelectedItem() const
{
    return mItems->getSelectedItem();
}

void InventoryWindow::valueChanged(const gcn::SelectionEvent &event)
{
    if (event.getSource() == mItems)
    {
        Item *item = mItems->getSelectedItem();

        item ? itemShortcut->setSelected(item->getId()) : 
               itemShortcut->setSelected(-1);

        updateButtons();
        distributeValueChangedEvent();
    }
}

void InventoryWindow::selectNone() const
{
    mItems->selectNone();
}

void InventoryWindow::requestFocus()
{
    mItems->requestFocus();
}

void InventoryWindow::widgetShown(const gcn::Event& event)
{
    Window::widgetShown(event);

    mWeightBar->reset();
    mSlotsBar->reset();
}

void InventoryWindow::widgetHidden(const gcn::Event& event)
{
    Window::widgetHidden(event);
    selectNone();
}
