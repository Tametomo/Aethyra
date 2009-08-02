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

#include <sstream>

#include <guichan/font.hpp>

#include "chat.h"
#include "itemamount.h"
#include "trade.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../structs/inventory.h"
#include "../structs/item.h"

#include "../widgets/itemcontainer.h"

#include "../../bindings/guichan/layout.h"

#include "../../bindings/guichan/widgets/button.h"
#include "../../bindings/guichan/widgets/label.h"
#include "../../bindings/guichan/widgets/scrollarea.h"
#include "../../bindings/guichan/widgets/inttextfield.h"

#include "../../core/image/sprite/localplayer.h"

#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

bool TradeWindow::mCanTrade = false;

TradeWindow::TradeWindow():
    Window(_("Trade")),
    mMyInventory(new Inventory(INVENTORY_SIZE)),
    mPartnerInventory(new Inventory(INVENTORY_SIZE))
{
    setWindowName("Trade");
    setDefaultSize(342, 209, ImageRect::CENTER);
    setResizable(true);
    saveVisibility(false);
    setCloseButton(true);

    setMinWidth(342);
    setMinHeight(209);

    std::string longestName = getFont()->getWidth(_("OK")) >
                              getFont()->getWidth(_("Trade")) ?
                              _("OK") : _("Trade");

    mOkButton = new Button(longestName, "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    mMyItemContainer = new ItemContainer(mMyInventory.get(), "showpopupmenumine", this);
    mMyItemContainer->setWidth(160);
    mMyItemContainer->addSelectionListener(this);

    mMyScroll = new ScrollArea(mMyItemContainer);

    mPartnerItemContainer = new ItemContainer(mPartnerInventory.get(), "showpopupmenutheirs", this);
    mPartnerItemContainer->setWidth(160);
    mPartnerItemContainer->addSelectionListener(this);

    mPartnerScroll = new ScrollArea(mPartnerItemContainer);

    mPartnerMoneyLabel = new Label(strprintf(_("You get %d GP."), 0));
    mOwnMoneyLabel = new Label(_("You give:"));
    mMoneyField = new IntTextField();
    mMoneyField->setWidth(50);
    mMoneyField->setRange(0, player_node->mGp);

    place(1, 0, mPartnerMoneyLabel);
    place(0, 1, mMyScroll).setPadding(3);
    place(1, 1, mPartnerScroll).setPadding(3);
    ContainerPlacer place;
    place = getPlacer(0, 0);
    place(0, 0, mOwnMoneyLabel);
    place(1, 0, mMoneyField);
    place = getPlacer(0, 2);
    place(6, 0, mCancelButton);
    place(7, 0, mOkButton);
    Layout &layout = getLayout();
    layout.extend(0, 2, 2, 1);
    layout.setRowHeight(1, Layout::AUTO_SET);
    layout.setRowHeight(2, 0);
    layout.setColWidth(0, Layout::AUTO_SET);
    layout.setColWidth(1, Layout::AUTO_SET);

    mOkButton->setCaption(_("OK"));

    loadWindowState();
}

TradeWindow::~TradeWindow()
{
}

void TradeWindow::addMoney(int amount)
{
    mPartnerMoneyLabel->setCaption(strprintf(_("You get %d GP."), amount));
    mPartnerMoneyLabel->adjustSize();
}

void TradeWindow::addItem(int id, bool own, int quantity, bool equipment)
{
    if (own)
    {
        mMyItemContainer->setWidth(mMyScroll->getWidth());
        mMyInventory->addItem(id, quantity, equipment);
    }
    else
    {
        mPartnerItemContainer->setWidth(mPartnerScroll->getWidth());
        mPartnerInventory->addItem(id, quantity, equipment);
    }
}

void TradeWindow::removeItem(int id, bool own)
{
    if (own)
        mMyInventory->removeItem(id);
    else
        mPartnerInventory->removeItem(id);
}

void TradeWindow::changeQuantity(int index, bool own, int quantity)
{
    if (own)
        mMyInventory->getItem(index)->setQuantity(quantity);
    else
        mPartnerInventory->getItem(index)->setQuantity(quantity);
}

void TradeWindow::increaseQuantity(int index, bool own, int quantity)
{
    if (own)
        mMyInventory->getItem(index)->increaseQuantity(quantity);
    else
        mPartnerInventory->getItem(index)->increaseQuantity(quantity);
}

void TradeWindow::reset()
{
    mCanTrade = true;
    mMyInventory->clear();
    mPartnerInventory->clear();
    mOkButton->setCaption(_("OK"));
    mOkButton->setActionEventId("ok");
    mOkButton->setEnabled(true);
    mOkOther = false;
    mOkMe = false;
    mPartnerMoneyLabel->setCaption(strprintf(_("You get %d GP."), 0));
    mMoneyField->setEnabled(true);
    mMoneyField->setValue(0);
    mMoneyField->setRange(0, player_node->mGp);
}

void TradeWindow::receivedOk(bool own)
{
    if (own)
    {
        mOkMe = true;
        if (mOkOther)
        {
            mOkButton->setCaption(_("Trade"));
            mOkButton->setActionEventId("trade");
            mOkButton->setEnabled(true);
        }
        else
            mOkButton->setEnabled(false);
    }
    else
    {
        mOkOther = true;
        if (mOkMe)
        {
            mOkButton->setCaption(_("Trade"));
            mOkButton->setActionEventId("trade");
            mOkButton->setEnabled(true);
        }
    }
}

void TradeWindow::tradeItem(Item *item, int quantity)
{
    // TODO: Our newer version of eAthena doesn't register this following
    //       function. Detect the actual server version, and re-enable this
    //       for that version only.
    //addItem(item->getId(), true, quantity, item->isEquipment());
    MessageOut outMsg(CMSG_TRADE_ITEM_ADD_REQUEST);
    outMsg.writeInt16(item->getInvIndex() + INVENTORY_OFFSET);
    outMsg.writeInt32(quantity);
}

const bool TradeWindow::tradingItem(const Item* item)
{
    return mMyInventory->contains(item);
}

void TradeWindow::valueChanged(const gcn::SelectionEvent &event)
{
    Item *item;

    /* If an item is selected in one container, make sure no item is selected
     * in the other container.
     */
    if (event.getSource() == mMyItemContainer &&
       (item = mMyItemContainer->getSelectedItem()))
        mPartnerItemContainer->selectNone();
    else if ((item = mPartnerItemContainer->getSelectedItem()))
        mMyItemContainer->selectNone();
}

void TradeWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "cancel")
    {
        close();
    }
    else if (event.getId() == "ok")
    {
        std::stringstream tempMoney(mMoneyField->getText());
        int tempInt;
        if (tempMoney >> tempInt)
        {
            mMoneyField->setText(toString(tempInt));

            MessageOut outMsg(CMSG_TRADE_ITEM_ADD_REQUEST);
            outMsg.writeInt16(0);
            outMsg.writeInt32(tempInt);
        }
        else
            mMoneyField->setText("");

        mMoneyField->setEnabled(false);
        mCanTrade = true;
        MessageOut outMsg(CMSG_TRADE_ADD_COMPLETE);
    }
    else if (event.getId() == "trade")
        MessageOut outMsg(CMSG_TRADE_OK);
    else if (event.getId() == "showpopupmenumine")
        mMyItemContainer->showPopupMenu(TRADE, false);
    else if (event.getId() == "showpopupmenutheirs")
        mPartnerItemContainer->showPopupMenu(TRADE, false);
}

void TradeWindow::close()
{
    mCanTrade = false;
    MessageOut outMsg(CMSG_TRADE_CANCEL_REQUEST);
}

void TradeWindow::mouseClicked(gcn::MouseEvent &event)
{
    Window::mouseClicked(event);

    if (event.getButton() == gcn::MouseEvent::RIGHT &&
        event.getSource() == mMyItemContainer)
        mMyItemContainer->showPopupMenu(TRADE);

    else if (event.getButton() == gcn::MouseEvent::RIGHT &&
             event.getSource() == mPartnerItemContainer)
        mPartnerItemContainer->showPopupMenu(TRADE);
}

void TradeWindow::requestFocus()
{
    mOkButton->requestFocus();
}

const bool TradeWindow::canTrade()
{
    return mCanTrade && tradeWindow->isVisible();
}
