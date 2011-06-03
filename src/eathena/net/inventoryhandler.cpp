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

#include <stdint.h>

#include "inventoryhandler.h"
#include "messagein.h"
#include "protocol.h"

#include "../db/iteminfo.h"

#include "../gui/chat.h"
#include "../gui/storagewindow.h"

#include "../structs/inventory.h"
#include "../structs/item.h"

#include "../../core/configuration.h"
#include "../../core/log.h"

#include "../../core/map/sprite/localplayer.h"

#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

enum { debugInventory = 1 };

InventoryHandler::InventoryHandler()
{
    static const uint16_t _messages[] = {
        SMSG_PLAYER_INVENTORY,
        SMSG_PLAYER_INVENTORY_ADD,
        SMSG_PLAYER_INVENTORY_REMOVE,
        SMSG_PLAYER_INVENTORY_USE,
        SMSG_ITEM_USE_RESPONSE,
        SMSG_PLAYER_STORAGE_ITEMS,
        SMSG_PLAYER_STORAGE_EQUIP,
        SMSG_PLAYER_STORAGE_STATUS,
        SMSG_PLAYER_STORAGE_ADD,
        SMSG_PLAYER_STORAGE_REMOVE,
        SMSG_PLAYER_STORAGE_CLOSE,
        0
    };
    handledMessages = _messages;
}

void InventoryHandler::handleMessage(MessageIn *msg)
{
    int number;
    int index, amount, itemId, equipType, arrow;
    int identified, cards[4], itemType;
    Inventory *inventory = player_node->getInventory();
    Inventory *storage = player_node->getStorage();

    switch (msg->getId())
    {
        case SMSG_PLAYER_INVENTORY:
        case SMSG_PLAYER_STORAGE_ITEMS:
            // Clear inventory - this will be a complete refresh
            if (msg->getId() == SMSG_PLAYER_INVENTORY)
                inventory->clear();

            msg->readInt16();  // length
            number = (msg->getLength() - 4) / 18;

            for (int loop = 0; loop < number; loop++)
            {
                index = msg->readInt16();
                itemId = msg->readInt16();
                itemType = msg->readInt8();
                identified = msg->readInt8();
                amount = msg->readInt16();
                arrow = msg->readInt16();
                for (int i = 0; i < 4; i++)
                    cards[i] = msg->readInt16();

                index -= (msg->getId() == SMSG_PLAYER_INVENTORY) ?
                         INVENTORY_OFFSET : STORAGE_OFFSET;

                if (debugInventory)
                    logger->log("Index: %d, ID: %d, Type: %d, Identified: %d, "
                                "Qty: %d, Cards: %d, %d, %d, %d",
                                index, itemId, itemType, identified, amount,
                                cards[0], cards[1], cards[2], cards[3]);

                if (msg->getId() == SMSG_PLAYER_INVENTORY)
                {
                    inventory->setItem(index, itemId, amount, false);

                    Item *item = inventory->getItem(index);

                    // Trick because arrows are not considered equipment
                    if (item && arrow & 0x8000)
                        item->setEquipment(true);
                }
                else
                    storage->setItem(index, itemId, amount, false);
            }
            break;

        case SMSG_PLAYER_STORAGE_EQUIP:
            msg->readInt16();  // length
            number = (msg->getLength() - 4) / 20;

            for (int loop = 0; loop < number; loop++)
            {
                index = msg->readInt16() - STORAGE_OFFSET;
                itemId = msg->readInt16();
                itemType = msg->readInt8();
                identified = msg->readInt8();
                amount = 1;
                msg->readInt16();    // Equip Point?
                msg->readInt16();    // Another Equip Point?
                msg->readInt8();   // Attribute (broken)
                msg->readInt8();   // Refine level
                for (int i = 0; i < 4; i++)
                    cards[i] = msg->readInt16();

                if (debugInventory)
                    logger->log("Index: %d, ID: %d, Type: %d, Identified: %d, "
                                "Qty: %d, Cards: %d, %d, %d, %d",
                                index, itemId, itemType, identified, amount,
                                cards[0], cards[1], cards[2], cards[3]);

                storage->setItem(index, itemId, amount, false);
            }
            break;

        case SMSG_PLAYER_INVENTORY_ADD:
            index = msg->readInt16() - INVENTORY_OFFSET;
            amount = msg->readInt16();
            itemId = msg->readInt16();
            identified = msg->readInt8();
            msg->readInt8();  // attribute
            msg->readInt8();  // refine

            for (int i = 0; i < 4; i++)
                cards[i] = msg->readInt16();

            equipType = msg->readInt16();
            itemType = msg->readInt8();

            if (msg->readInt8() > 0 && config.getValue("showpickupchat", true))
                chatWindow->chatLog(_("Unable to pick up item."));
            else
            {
                const ItemInfo &itemInfo = ItemDB::get(itemId);
                const std::string amountStr =
                    (amount > 1) ? toString(amount) : "a";

                if (config.getValue("showpickupchat", true))
                {
                    chatWindow->chatLog(strprintf(_("You picked up %s [%s]."),
                                        amountStr.c_str(),
                                        itemInfo.getName().c_str()));
                }

                if (config.getValue("showpickupparticle", false))
                    player_node->pickedUp(itemInfo.getName());

                if (Item *item = inventory->getItem(index))
                {
                    item->setId(itemId);
                    item->increaseQuantity(amount);
                }
                else
                    inventory->setItem(index, itemId, amount, equipType != 0);
            }
            break;

        case SMSG_PLAYER_INVENTORY_REMOVE:
            index = msg->readInt16() - INVENTORY_OFFSET;
            amount = msg->readInt16();
            if (Item *item = inventory->getItem(index))
            {
                item->increaseQuantity(-amount);

                if (item->getQuantity() == 0)
                    inventory->removeItemAt(index);
            }
            break;

        case SMSG_PLAYER_INVENTORY_USE:
            index = msg->readInt16() - INVENTORY_OFFSET;
            msg->readInt16(); // item id
            msg->readInt32();  // id
            amount = msg->readInt16();
            msg->readInt8();  // type

            if (Item *item = inventory->getItem(index))
                item->setQuantity(amount);

            break;

        case SMSG_ITEM_USE_RESPONSE:
            index = msg->readInt16() - INVENTORY_OFFSET;
            amount = msg->readInt16();

            if (msg->readInt8() == 0)
                chatWindow->chatLog(_("Failed to use item."));
            else if (Item *item = inventory->getItem(index))
                item->setQuantity(amount);

            break;

        case SMSG_PLAYER_STORAGE_STATUS:
            /*
             * This is the closest we get to an "Open Storage" packet from the
             * server. It always comes after the two SMSG_PLAYER_STORAGE_...
             * packets that update storage contents..
             */
            player_node->setInStorage(true);
            msg->readInt16(); // Storage capacity
            msg->readInt16(); // Used count
            break;

        case SMSG_PLAYER_STORAGE_ADD:
            /*
             * Move an item into storage
             */
            index = msg->readInt16() - STORAGE_OFFSET;
            amount = msg->readInt32();
            itemId = msg->readInt16();
            identified = msg->readInt8();
            msg->readInt8();  // attribute
            msg->readInt8();  // refine
            for (int i = 0; i < 4; i++)
                cards[i] = msg->readInt16();

            if (Item *item = storage->getItem(index))
            {
                item->setId(itemId);
                item->increaseQuantity(amount);
            }
            else
                storage->setItem(index, itemId, amount, false);

            break;

        case SMSG_PLAYER_STORAGE_REMOVE:
            /*
             * Move an item out of storage
             */
            index = msg->readInt16() - STORAGE_OFFSET;
            amount = msg->readInt16();
            if (Item *item = storage->getItem(index))
            {
                item->increaseQuantity(-amount);

                if (item->getQuantity() == 0)
                    storage->removeItemAt(index);
            }
            break;

        case SMSG_PLAYER_STORAGE_CLOSE:
            /*
             * Storage access has been closed
             */
            storage->clear();
            player_node->setInStorage(false);
            break;
    }
}
