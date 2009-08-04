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

#include "npclistdialog.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../../core/map/sprite/npc.h"

#include "../../core/utils/gettext.h"

NpcListDialog::NpcListDialog():
    ListDialog("NPC")
{
    setWindowName(_("NPC"));
    saveVisibility(false);

    loadWindowState();
}

void NpcListDialog::parseItems(const std::string &itemString)
{
    std::istringstream iss(itemString);
    std::string tmp;

    while (getline(iss, tmp, ':'))
        addOption(tmp);
}

void NpcListDialog::reset()
{
    NPC::mTalking = false;
    ListDialog::reset();
}

void NpcListDialog::action(const gcn::ActionEvent &event)
{
    mChoice = 0xff;

    if (event.getId() == "ok")
    {
        // Send the selected index back to the server
        const int selectedIndex = getSelected();

        if (selectedIndex > -1)
            mChoice = selectedIndex + 1;
    }

    close();
}

void NpcListDialog::widgetShown(const gcn::Event& event)
{
    ListDialog::widgetShown(event);
    loadWindowState();
}

void NpcListDialog::close()
{
    ListDialog::close();
    saveWindowState();

    if (!current_npc)
        return;

    MessageOut outMsg(CMSG_NPC_LIST_CHOICE);
    outMsg.writeInt32(current_npc);
    outMsg.writeInt8(mChoice);

    current_npc = 0;
}
