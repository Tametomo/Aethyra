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

#include "charserverhandler.h"
#include "logindata.h"
#include "messagein.h"
#include "messageout.h"
#include "network.h"
#include "protocol.h"

#include "../game.h"

#include "../gui/charselect.h"

#include "../../main.h"

#include "../../bindings/guichan/dialogs/okdialog.h"

#include "../../core/log.h"

#include "../../core/map/sprite/localplayer.h"

#include "../../core/utils/dtor.h"
#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

CharServerHandler::CharServerHandler()
{
    static const Uint16 _messages[] = {
        SMSG_CONNECTION_PROBLEM,
        0x006b,
        0x006c,
        0x006d,
        0x006e,
        0x006f,
        0x0070,
        0x0071,
        0
    };
    handledMessages = _messages;
}

void CharServerHandler::handleMessage(MessageIn *msg)
{
    int slot, flags, code;
    LocalPlayer *tempPlayer;

    logger->log("CharServerHandler: Packet ID: %x, Length: %d",
            msg->getId(), msg->getLength());
    switch (msg->getId())
    {
        case SMSG_CONNECTION_PROBLEM:
            code = msg->readInt8();
            logger->log("Connection problem: %i", code);

            switch (code)
            {
                case 0:
                    errorMessage = _("Authentication failed.");
                    break;
                case 1:
                    errorMessage = _("Map server(s) offline.");
                    break;
                case 2:
                    errorMessage = _("This account is already logged in.");
                    break;
                case 3:
                    errorMessage = _("Speed hack detected.");
                    break;
                case 8:
                    errorMessage = _("Duplicated login.");
                    break;
                default:
                    errorMessage = _("Unknown connection error.");
                    break;
            }
            state = ERROR_STATE;
            break;

        case 0x006b:
            msg->skip(2); // Length word
            flags = msg->readInt32(); // Aethyra extensions flags
            logger->log("Server flags are: %x", flags);
            msg->skip(16); // Unused

            // Derive number of characters from message length
            loginData.slots = (msg->getLength() - 24) / 106;

            for (int i = 0; i < loginData.slots; i++)
            {
                tempPlayer = readPlayerData(*msg, slot);
                mCharInfo->select(slot);
                mCharInfo->setEntry(tempPlayer);
                logger->log("CharServer: Player: %s (%d)",
                tempPlayer->getName().c_str(), slot);
            }

            state = CHAR_SELECT_STATE;
            break;

        case 0x006c:
            switch (msg->readInt8())
            {
                case 0:
                    errorMessage = _("Access denied.");
                    break;
                case 1:
                    errorMessage = _("Cannot use this ID.");
                    break;
                default:
                    errorMessage = _("Unknown failure to select character.");
                    break;
            }
            mCharInfo->unlock();
            break;

        case 0x006d:
            tempPlayer = readPlayerData(*msg, slot);
            mCharInfo->unlock();
            mCharInfo->select(slot);
            mCharInfo->setEntry(tempPlayer);
            loginData.slots++;
            if (charSelectDialog)
                charSelectDialog->updatePlayerInfo();

            // Close the character create dialog
            if (charCreateDialog)
                destroy(charCreateDialog);

            break;

        case 0x006e:
            new OkDialog(_("Error"), _("Failed to create character. Most "
                                       "likely the name is already taken."));
            if (charCreateDialog)
                charCreateDialog->unlock();
            break;

        case 0x006f:
            delete mCharInfo->getEntry();
            mCharInfo->setEntry(0);
            mCharInfo->unlock();
            loginData.slots--;
            if (charSelectDialog)
                charSelectDialog->updatePlayerInfo();
            new OkDialog(_("Info"), _("Character deleted."));
            break;

        case 0x0070:
            mCharInfo->unlock();
            new OkDialog(_("Error"), _("Failed to delete character."));
            break;

        case 0x0071:
            player_node = mCharInfo->getEntry();
            slot = mCharInfo->getPos();
            msg->skip(4); // CharID, must be the same as player_node->charID
            map_path = msg->readString(16);
            loginData.hostname = ipToString(msg->readInt32());
            loginData.port = msg->readInt16();
            mCharInfo->unlock();
            mCharInfo->select(0);
            // Clear unselected players infos
            do
            {
                LocalPlayer *tmp = mCharInfo->getEntry();
                if (tmp != player_node)
                {
                    destroy(tmp);
                    mCharInfo->setEntry(0);
                }
                mCharInfo->next();
            } while (mCharInfo->getPos());

            mCharInfo->select(slot);
            state = CONNECTING_STATE;
            break;
    }
}

LocalPlayer *CharServerHandler::readPlayerData(MessageIn &msg, int &slot)
{
    LocalPlayer *tempPlayer = new LocalPlayer(loginData.account_ID, 0, NULL);
    tempPlayer->setGender((loginData.sex == 0) ? GENDER_FEMALE : GENDER_MALE);

    tempPlayer->mCharId = msg.readInt32();
    tempPlayer->setXp(msg.readInt32());
    tempPlayer->mGp = msg.readInt32();
    tempPlayer->mJobXp = msg.readInt32();
    tempPlayer->mJobLevel = msg.readInt32();
    tempPlayer->setSprite(Being::SHOE_SPRITE, msg.readInt16());
    tempPlayer->setSprite(Being::GLOVES_SPRITE, msg.readInt16());
    tempPlayer->setSprite(Being::CAPE_SPRITE, msg.readInt16());
    tempPlayer->setSprite(Being::MISC1_SPRITE, msg.readInt16());
    msg.readInt32();                       // option
    msg.readInt32();                       // karma
    msg.readInt32();                       // manner
    msg.skip(2);                           // unknown
    tempPlayer->mHp = msg.readInt16();
    tempPlayer->mMaxHp = msg.readInt16();
    tempPlayer->mMp = msg.readInt16();
    tempPlayer->mMaxMp = msg.readInt16();
    tempPlayer->setWalkSpeed(msg.readInt16());  // speed
    msg.readInt16();                            // class
    int hairStyle = msg.readInt16();
    tempPlayer->setSprite(Being::WEAPON_SPRITE, msg.readInt16());
    tempPlayer->mLevel = msg.readInt16();
    msg.readInt16();                       // skill point
    tempPlayer->setSprite(Being::BOTTOMCLOTHES_SPRITE, msg.readInt16()); // head bottom
    tempPlayer->setSprite(Being::SHIELD_SPRITE, msg.readInt16());
    tempPlayer->setSprite(Being::HAT_SPRITE, msg.readInt16()); // head option top
    tempPlayer->setSprite(Being::TOPCLOTHES_SPRITE, msg.readInt16()); // head option mid
    int hairColor = msg.readInt16();
    tempPlayer->setHairStyle(hairStyle, hairColor);
    tempPlayer->setSprite(Being::MISC2_SPRITE, msg.readInt16());
    tempPlayer->setName(msg.readString(24));

    for (int i = 0; i < 6; i++)
        tempPlayer->mAttr[i] = msg.readInt8();

    slot = msg.readInt8(); // character slot
    msg.readInt8();        // unknown

    return tempPlayer;
}

void CharServerHandler::login(LockedArray<LocalPlayer*> *charInfo)
{
    logger->log("Sending server client version and getting character data...");
    network->disconnect();
    network->connect(loginData.hostname, loginData.port);
    network->registerHandler(this);
    mCharInfo = charInfo;

    // Send login infos
    MessageOut outMsg(0x0065);
    outMsg.writeInt32(loginData.account_ID);
    outMsg.writeInt32(loginData.session_ID1);
    outMsg.writeInt32(loginData.session_ID2);
    // [Fate] The next word is unused by the old char server, so we squeeze in
    //        tmw client version information
    outMsg.writeInt16(CLIENT_PROTOCOL_VERSION);
    outMsg.writeInt8(loginData.sex);

    // We get 4 useless bytes before the real answer comes in
    network->skip(4);
}

