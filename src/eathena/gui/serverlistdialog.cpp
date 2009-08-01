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

#include "serverlistdialog.h"

#include "../net/logindata.h"
#include "../net/serverinfo.h"

#include "../../main.h"

#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

extern SERVER_INFO **server_info;

ServerListDialog::ServerListDialog(LoginData *loginData, int nextState):
    ListDialog("ServerSelect"),
    mLoginData(loginData),
    mNextState(nextState)
{
    setWindowName(_("Select Server"));
    saveVisibility(false);

    loadWindowState();
}

void ServerListDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        const SERVER_INFO *si = server_info[getSelected()];

        mLoginData->hostname = ipToString(si->address);
        mLoginData->port = si->port;
        mLoginData->updateHost = si->updateHost;

        state = mNextState;
    }
    else if (event.getId() == "cancel")
        state = LOGIN_STATE;
}

void ServerListDialog::widgetShown(const gcn::Event& event)
{
    for (int i = 0; i < n_server; i++)
    {
        const SERVER_INFO *si = server_info[i];
        addOption(si->name + " (" + toString(si->online_users) + ")");
    }

    ListDialog::widgetShown(event);
}
