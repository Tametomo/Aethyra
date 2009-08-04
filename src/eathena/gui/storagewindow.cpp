/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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

#include "itemamount.h"
#include "storagewindow.h"

#include "../db/iteminfo.h"

#include "../net/messageout.h"
#include "../net/network.h"
#include "../net/protocol.h"

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

StorageWindow::StorageWindow(int invSize):
    Window(_("Storage")),
    mMaxSlots(invSize),
    mItemDesc(false)
{
    setWindowName("Storage");
    setResizable(true);
    saveVisibility(false);
    setCloseButton(true);

    setDefaultSize(375, 300, ImageRect::CENTER);

    mRetrieveButton = new Button(_("Retrieve"), "retrieve", this);
    mRetrieveButton->setEnabled(false);

    Button *closeButton = new Button(_("Close"), "close", this);

    mItems = new ItemContainer(player_node->getStorage(), "showpopupmenu", this);
    mItems->addSelectionListener(this);

    mInvenScroll = new ScrollArea(mItems);
    mInvenScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mUsedSlots = player_node->getStorage()->getNumberOfSlotsUsed();

    mSlotsLabel = new Label(_("Slots:"));

    mSlotsBar = new ProgressBar(1.0f, 100, 20, gcn::Color(225, 200, 25));
    mSlotsBar->setText(strprintf("%d/%d", mUsedSlots, mMaxSlots));
    mSlotsBar->setProgress((float) mUsedSlots / mMaxSlots);

    setMinHeight(130);
    setMinWidth(200);

    place(0, 0, mSlotsLabel).setPadding(3);
    place(1, 0, mSlotsBar, 3);
    place(0, 1, mInvenScroll, 4, 4);
    place(2, 5, closeButton);
    place(3, 5, mRetrieveButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, mRetrieveButton->getHeight());

    loadWindowState();
}

StorageWindow::~StorageWindow()
{
    delete mItems;
}

void StorageWindow::logic()
{
    if (!isVisible())
        return;

    Window::logic();

    const int usedSlots = player_node->getStorage()->getNumberOfSlotsUsed();

    if (mUsedSlots != usedSlots)
    {
        mUsedSlots = usedSlots;

        mSlotsBar->setProgress((float) mUsedSlots / mMaxSlots);
        mSlotsBar->setText(strprintf("%d/%d", mUsedSlots, mMaxSlots));
    }
}

void StorageWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "close")
    {
        mItems->selectNone();
        close();
    }
    else if (event.getId() == "retrieve" || event.getId() == "default")
    {
        Item *item = mItems->getSelectedItem();

        if (!item)
            return;

        if (item->getQuantity() == 1)
        {
            removeStore(item, 1);
            selectNone();
        }
        // Choose amount of items to retrieve
        else
            new ItemAmountWindow(AMOUNT_STORE_REMOVE, this, item);
    }
    else if (event.getId() == "showpopupmenu")
        mItems->showPopupMenu(STORAGE, false);
}

void StorageWindow::valueChanged(const gcn::SelectionEvent &event)
{
    if (event.getSource() == mItems)
        mRetrieveButton->setEnabled(mItems->getSelectedItem() != 0);
}

void StorageWindow::mouseClicked(gcn::MouseEvent &event)
{
    Window::mouseClicked(event);

    if (event.getButton() == gcn::MouseEvent::RIGHT &&
        event.getSource() == mItems)
        mItems->showPopupMenu(STORAGE);
}

Item* StorageWindow::getSelectedItem() const
{
    return mItems->getSelectedItem();
}

void StorageWindow::widgetShown(const gcn::Event& event)
{
    Window::widgetShown(event);
    mSlotsBar->reset();
}

void StorageWindow::addStore(Item *item, int amount)
{
    MessageOut outMsg(CMSG_MOVE_TO_STORAGE);
    outMsg.writeInt16(item->getInvIndex() + INVENTORY_OFFSET);
    outMsg.writeInt32(amount);
}

void StorageWindow::removeStore(Item *item, int amount)
{
    MessageOut outMsg(CSMG_MOVE_FROM_STORAGE);
    outMsg.writeInt16(item->getInvIndex() + STORAGE_OFFSET);
    outMsg.writeInt32(amount);
}

void StorageWindow::close()
{
    MessageOut outMsg(CMSG_CLOSE_STORAGE);
}

void StorageWindow::selectNone() const
{
    mItems->selectNone();
}

void StorageWindow::requestFocus()
{
    mItems->requestFocus();
}
