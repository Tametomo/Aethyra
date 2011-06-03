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
#include "npchandler.h"
#include "protocol.h"

#include "../beingmanager.h"

#include "../gui/npctext.h"
#include "../gui/npcintegerdialog.h"
#include "../gui/npclistdialog.h"
#include "../gui/npcstringdialog.h"

#include "../../core/map/sprite/npc.h"

NPCHandler::NPCHandler()
{
    static const uint16_t _messages[] = {
        SMSG_NPC_CHOICE,
        SMSG_NPC_MESSAGE,
        SMSG_NPC_NEXT,
        SMSG_NPC_CLOSE,
        SMSG_NPC_INT_INPUT,
        SMSG_NPC_STR_INPUT,
        0
    };
    handledMessages = _messages;
}

void NPCHandler::handleMessage(MessageIn *msg)
{
    int id;

    switch (msg->getId())
    {
        case SMSG_NPC_CHOICE:
            msg->readInt16();  // length
            current_npc = msg->readInt32();
            npcListDialog->parseItems(msg->readString(msg->getLength() - 8));
            npcListDialog->setVisible(true);
            break;

        case SMSG_NPC_MESSAGE:
            msg->readInt16();  // length
            current_npc = msg->readInt32();
            npcTextDialog->addText(msg->readString(msg->getLength() - 8));
            npcTextDialog->setVisible(true);
            break;

         case SMSG_NPC_CLOSE:
            id = msg->readInt32();
            npcTextDialog->close();
            break;

        case SMSG_NPC_NEXT:
            // Next button in NPC dialog, currently unused
            break;

        case SMSG_NPC_INT_INPUT:
            // Request for an integer
            current_npc = msg->readInt32();
            npcIntegerDialog->setRange(0, 2147483647);
            npcIntegerDialog->setVisible(true);
            break;

        case SMSG_NPC_STR_INPUT:
            // Request for a string
            current_npc = msg->readInt32();
            npcStringDialog->setVisible(true);
            break;
    }
}
