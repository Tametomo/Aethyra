/*
 *  Aethyra
 *  Copyright 2008  Lloyd Bryant <lloyd_bryant@netzero.net>
 *
 *  This file is part of Aethyra.
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

#include "party.h"

#include "gui/chat.h"

#include "net/messageout.h"
#include "net/protocol.h"

#include "../bindings/guichan/dialogs/confirmdialog.h"

#include "../core/map/sprite/being.h"
#include "../core/map/sprite/localplayer.h"

#include "../core/utils/dtor.h"
#include "../core/utils/gettext.h"
#include "../core/utils/stringutils.h"

Party::Party(ChatWindow *chat) :
    mChat(chat),
    mInviteListener(&mInParty)
{
}

void Party::respond(const std::string &command, const std::string &args)
{
    if (command == "new" || command == "create")
        create(args);
    else if (command == "leave")
        leave(args);
    else if (command == "settings")
    {
        mChat->chatLog(_("Not yet implemented!"));
        /*
        MessageOut outMsg(CMSG_PARTY_SETTINGS);
        outMsg.writeInt16(0); // Experience
        outMsg.writeInt16(0); // Item
        */
    }
    else
        mChat->chatLog(_("Party command not known."));
}

void Party::create(const std::string &party)
{
    if (party.empty())
        mChat->chatLog(_("Party name is missing."));
    else
    {
        MessageOut outMsg(CMSG_PARTY_CREATE);
        outMsg.writeString(party.substr(0, 23), 24);
        mCreating = true;
    }
}

void Party::leave(const std::string &args)
{
    MessageOut outMsg(CMSG_PARTY_LEAVE);
    mChat->chatLog(_("Left party."));
    mInParty = false;
}

void Party::createResponse(bool ok)
{
    if (ok)
    {
        mChat->chatLog(_("Party successfully created."));
        mInParty = true;
    }
    else
        mChat->chatLog(_("Could not create party."));
}

void Party::inviteResponse(const std::string &nick, int status) const
{
    switch (status)
    {
        case 0:
            mChat->chatLog(strprintf(_("%s is already a member of a party."),
                           nick.c_str()));
            break;
        case 1:
            mChat->chatLog(strprintf(_("%s refused your invitation."),
                           nick.c_str()));
            break;
        case 2:
            mChat->chatLog(strprintf(_("%s is now a member of your party."),
                           nick.c_str()));
            break;
    }
}

void Party::invitedAsk(const std::string &nick, int gender,
                       const std::string &partyName)
{
    mPartyName = partyName; /* Quick and nasty - needs redoing */

    if (nick.empty())
    {
        mChat->chatLog(_("You can\'t have a blank party name!"));
        return;
    }

    mCreating = false;
    ConfirmDialog *dlg = new ConfirmDialog(_("Invite to party"), strprintf(_(
                                             "%s invites you to join the %s "
                                             "party, do you accept?"),
                                             nick.c_str(), partyName.c_str()));
    dlg->addActionListener(&mInviteListener);
}

void Party::InviteListener::action(const gcn::ActionEvent &event)
{
    MessageOut outMsg(CMSG_PARTY_INVITED);
    outMsg.writeInt32(player_node->getId());
    const bool accept = event.getId() == "yes";
    outMsg.writeInt32(accept ? 1 : 0);
    *mInParty = *mInParty || accept;
}

void Party::leftResponse(const std::string &nick) const
{
    mChat->chatLog(strprintf(_("%s has left your party."), nick.c_str()));
}

void Party::receiveChat(Being *being, const std::string &msg) const
{
    if (!being)
        return;

    if (being->getType() != Being::PLAYER)
    {
        mChat->chatLog(_("Party chat received, but being is not a player"));
        return;
    }
    being->setSpeech(msg, SPEECH_TIME);
    mChat->chatLog(being->getName() + " : " + msg, Palette::PARTY);
}

void Party::help(const std::string &msg) const
{
    if (msg.empty())
    {
        mChat->chatLog(_("Command: /party <command> <args>"));
        mChat->chatLog(_("where <command> can be one of:"));
        mChat->chatLog("   /new");
        mChat->chatLog("   /create");
        mChat->chatLog("   /prefix");
        mChat->chatLog("   /leave");
        mChat->chatLog(_("This command implements the partying function."));
        mChat->chatLog(_("Type /help party <command> for further help."));
    }
    else if (msg == "new" || msg == "create")
    {
        mChat->chatLog(_("Command: /party new <party-name>"));
        mChat->chatLog(_("Command: /party create <party-name>"));
        mChat->chatLog(_("These commands create a new party <party-name>."));
    }
    else if (msg == "prefix")
    {
        mChat->chatLog(_("Command: /party prefix <prefix-char>"));
        mChat->chatLog(_("This command sets the party prefix character."));
        mChat->chatLog(_("Any message preceded by <prefix-char> is sent to "
                         "the party instead of everyone."));
        mChat->chatLog(_("Command: /party prefix"));
        mChat->chatLog(_("This command reports the current party prefix "
                         "character."));
    }
    //if (msg == "settings")
    //if (msg == "info")
    else if (msg == "leave")
    {
        mChat->chatLog(_("Command: /party leave"));
        mChat->chatLog(_("This command causes the player to leave the party."));
    }
    else
    {
        mChat->chatLog(_("Unknown /party command."));
        mChat->chatLog(_("Type /help party for a list of options."));
    }
}
