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

#include "messagein.h"
#include "protocol.h"
#include "tradehandler.h"

#include "../playerrelations.h"

#include "../gui/chat.h"
#include "../gui/trade.h"

#include "../structs/inventory.h"
#include "../structs/item.h"

#include "../../bindings/guichan/dialogs/confirmdialog.h"

#include "../../core/map/sprite/localplayer.h"

#include "../../core/utils/dtor.h"
#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

std::string tradePartnerName;
ConfirmDialog *confirmDlg;

/**
 * Listener for request trade dialogs
 */
namespace {
    struct RequestTradeListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            confirmDlg = NULL;
            player_node->tradeReply(event.getId() == "yes");
        }
    } listener;
}

TradeHandler::TradeHandler()
{
    static const uint16_t _messages[] = {
        SMSG_TRADE_REQUEST,
        SMSG_TRADE_RESPONSE,
        SMSG_TRADE_ITEM_ADD,
        SMSG_TRADE_ITEM_ADD_RESPONSE,
        SMSG_TRADE_OK,
        SMSG_TRADE_CANCEL,
        SMSG_TRADE_COMPLETE,
        0
    };
    handledMessages = _messages;

    confirmDlg = NULL;
}


void TradeHandler::handleMessage(MessageIn *msg)
{
    switch (msg->getId())
    {
        case SMSG_TRADE_REQUEST:
            {
                // If a trade window or request window is already open, send a
                // trade cancel to any other trade request.
                //
                // Note that it would be nice if the server would prevent this
                // situation, and that the requesting player would get a
                // special message about the player being occupied.
                std::string tradePartnerNameTemp = msg->readString(24);

                if (player_relations.hasPermission(tradePartnerName,
                                                   PlayerRelation::TRADE))
                {
                    if (!player_node->tradeRequestOk() || confirmDlg)
                    {
                        player_node->tradeReply(false);
                        break;
                    }

                    tradePartnerName = tradePartnerNameTemp;
                    player_node->setTrading(true);
                    confirmDlg = new ConfirmDialog(_("Request for Trade"),
                                                     strprintf(_("%s wants "
                                                     "to trade with you, do "
                                                     "you accept?"),
                                                     tradePartnerName.c_str()));
                    confirmDlg->addActionListener(&listener);
                }
                else
                {
                    player_node->tradeReply(false);
                    break;
                }
            }
            break;

        case SMSG_TRADE_RESPONSE:
            switch (msg->readInt8())
            {
                case 0: // Too far away
                    chatWindow->chatLog(_("Trading isn't possible. Trade "
                                          "partner is too far away."));
                    break;
                case 1: // Character doesn't exist
                    chatWindow->chatLog(_("Trading isn't possible. Character "
                                          "doesn't exist."));
                    break;
                case 2: // Invite request check failed...
                    chatWindow->chatLog(_("Trade cancelled due to an unknown "
                                          "reason."));
                    break;
                case 3: // Trade accepted
                    tradeWindow->reset();
                    tradeWindow->setCaption(strprintf(_("Trade: You and %s"),
                                            tradePartnerName.c_str()));
                    tradeWindow->setVisible(true);
                    break;
                case 4: // Trade cancelled
                    if (player_relations.hasPermission(tradePartnerName,
                                                       PlayerRelation::SPEECH_LOG))
                        chatWindow->chatLog(strprintf(_("Trade with %s "
                                                        "cancelled."),
                                            tradePartnerName.c_str()));
                    // otherwise ignore silently

                    tradeWindow->setVisible(false);
                    player_node->setTrading(false);
                    break;
                default: // Shouldn't happen as well, but to be sure
                    chatWindow->chatLog(_("Unhandled trade cancel packet."));
                    break;
            }
            break;

        case SMSG_TRADE_ITEM_ADD:
            {
                int amount = msg->readInt32();
                int type = msg->readInt16();
                msg->readInt8();  // identified flag
                msg->readInt8();  // attribute
                msg->readInt8();  // refine
                msg->skip(8);     // card (4 shorts)

                // TODO: handle also identified, etc
                if (type == 0)
                    tradeWindow->addMoney(amount);
                else
                    tradeWindow->addItem(type, false, amount, false);
            }
            break;

        case SMSG_TRADE_ITEM_ADD_RESPONSE:
            // Trade: New Item add response (was 0x00ea, now 01b1)
            {
                const int index = msg->readInt16();
                Item *item = player_node->getInventory()->getItem(index -
                                                                  INVENTORY_OFFSET);
                if (!item)
                {
                    tradeWindow->receivedOk(true);
                    return;
                }
                int quantity = msg->readInt16();

                switch (msg->readInt8())
                {
                    case 0:
                        // Successfully added item
                        if (item->isEquipment() && item->isEquipped())
                            player_node->unequipItem(item);

                        tradeWindow->addItem(item->getId(), true, quantity,
                                             item->isEquipment());
                        item->increaseQuantity(-quantity);
                        break;
                    case 1:
                        // Add item failed - player overweighted
                        chatWindow->chatLog(_("Failed adding item. Trade "
                                              "partner is over weighted."));
                        break;
                    case 2:
                         // Add item failed - player has no free slot
                         chatWindow->chatLog(_("Failed adding item. Trade "
                                               "partner has no free slot."));
                         break;
                    default:
                        chatWindow->chatLog(_("Failed adding item for "
                                              "unknown reason."));
                        break;
                }
            }
            break;

        case SMSG_TRADE_OK:
            // 0 means ok from myself, 1 means ok from other;
            tradeWindow->receivedOk(msg->readInt8() == 0);
            break;

        case SMSG_TRADE_CANCEL:
            chatWindow->chatLog(_("Trade canceled."));
            tradeWindow->setVisible(false);
            tradeWindow->reset();
            player_node->setTrading(false);
            break;

        case SMSG_TRADE_COMPLETE:
            chatWindow->chatLog(_("Trade completed."));
            tradeWindow->setVisible(false);
            tradeWindow->reset();
            player_node->setTrading(false);
            break;
    }
}
