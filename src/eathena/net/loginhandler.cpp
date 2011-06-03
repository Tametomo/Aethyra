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

#include "logindata.h"
#include "loginhandler.h"
#include "messagein.h"
#include "messageout.h"
#include "network.h"
#include "protocol.h"
#include "serverinfo.h"

#include "../statemanager.h"

#include "../../core/log.h"

#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

SERVER_INFO **server_info;

LoginHandler::LoginHandler()
{
    static const uint16_t _messages[] = {
        SMSG_CONNECTION_PROBLEM,
        SMSG_UPDATE_HOST,
        0x0069,
        0x006a,
        0
    };
    handledMessages = _messages;
}

void LoginHandler::handleMessage(MessageIn *msg)
{
    int code;
    std::string error;

    switch (msg->getId())
    {
        case SMSG_CONNECTION_PROBLEM:
            code = msg->readInt8();
            logger->log("Connection problem: %i", code);

            switch (code)
            {
                case 0:
                    error = _("Authentication failed");
                    break;
                case 1:
                    error = _("No servers available");
                    break;
                case 2:
                    error = _("This account is already logged in");
                    break;
                default:
                    error = _("Unknown connection error");
                    break;
            }
            stateManager->handleException(error, START_STATE);
            break;

        case SMSG_UPDATE_HOST:
             int len;

             len = msg->readInt16() - 4;
             mUpdateHost = msg->readString(len);

             logger->log("Received update host \"%s\" from login server",
                         mUpdateHost.c_str());
             break;

        case 0x0069:
            // Skip the length word
            msg->skip(2);

            loginData.servers = (msg->getLength() - 47) / 32;
            server_info =
                (SERVER_INFO**) malloc(sizeof(SERVER_INFO*) * loginData.servers);

            loginData.session_ID1 = msg->readInt32();
            loginData.account_ID = msg->readInt32();
            loginData.session_ID2 = msg->readInt32();
            msg->skip(30);                           // unknown
            loginData.sex = msg->readInt8();

            for (int i = 0; i < loginData.servers; i++)
            {
                server_info[i] = new SERVER_INFO();

                server_info[i]->address = msg->readInt32();
                server_info[i]->port = msg->readInt16();
                server_info[i]->name = msg->readString(20);
                server_info[i]->online_users = msg->readInt32();
                server_info[i]->updateHost = mUpdateHost;
                msg->skip(2);                        // unknown

                logger->log("Network: Server: %s (%s:%d)",
                            server_info[i]->name.c_str(),
                            ipToString(server_info[i]->address),
                            server_info[i]->port);
            }
            stateManager->setState(SERVER_SELECT_STATE);
            break;

        case 0x006a:
            code = msg->readInt8();
            logger->log("Login::error code: %i", code);

            switch (code)
            {
                case 0:
                    error = _("Unregistered ID");
                    break;
                case 1:
                    error = _("Wrong password");
                    break;
                case 2:
                    error = _("Account expired");
                    break;
                case 3:
                    error = _("Rejected from server");
                    break;
                case 4:
                    error = _("You have been permanently banned from "
                              "the game. Please contact the GM Team.");
                    break;
                case 6:
                    error = strprintf(_("You have been temporarily banned from "
                                        "the game until %s.\nPlease contact the"
                                        " GM team via the forums."),
                                        msg->readString(20).c_str());
                    break;
                case 9:
                    error = _("This user name is already taken");
                    break;
                default:
                    error = _("Unknown error");
                    break;
            }
            stateManager->handleException(error, LOGIN_STATE);
            break;
    }
}

void LoginHandler::login()
{
    logger->log("Trying to connect to account server...");
    logger->log("Username is %s", loginData.username.c_str());
    network->connect(loginData.hostname, loginData.port);
    network->registerHandler(this);

    // Send login infos
    MessageOut outMsg(0x0064);
    outMsg.writeInt32(0); // client version
    outMsg.writeString(loginData.username, 24);
    outMsg.writeString(loginData.password, 24);

    /*
     * eAthena calls the last byte "client version 2", but it isn't used at
     * at all. We're retasking it, with bit 0 to indicate whether the client
     * can handle the 0x63 "update host" packet. Clients prior to 0.0.25 send
     * 0 here.
     */
    outMsg.writeInt8(0x01);

    // Clear the password, avoids auto login when returning to login
    loginData.password = "";

    // Remove _M or _F from username after a login for registration purpose
    if (loginData.registerLogin)
    {
        loginData.username =
            loginData.username.substr(0, loginData.username.length() - 2);
    }
}
