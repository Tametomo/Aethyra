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

#include "charserver.h"

#include "../main.h"

#include "../bindings/guichan/layout.h"

#include "../bindings/guichan/widgets/button.h"
#include "../bindings/guichan/widgets/listbox.h"
#include "../bindings/guichan/widgets/scrollarea.h"

#include "../net/logindata.h"
#include "../net/serverinfo.h"

#include "../utils/gettext.h"
#include "../utils/stringutils.h"

extern SERVER_INFO **server_info;

/**
 * The list model for the server list.
 */
class ServerListModel : public gcn::ListModel
{
    public:
        virtual ~ServerListModel() {};

        int getNumberOfElements();
        std::string getElementAt(int i);
};

ServerSelectDialog::ServerSelectDialog(LoginData *loginData, int nextState):
    Window(_("Select Server")),
    mLoginData(loginData),
    mNextState(nextState)
{
    mServerListModel = new ServerListModel();
    mServerList = new ListBox(mServerListModel);
    mServerList->setActionEventId("ok");

    ScrollArea *mScrollArea = new ScrollArea(mServerList);
    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mOkButton = new Button(_("OK"), "ok", this);
    Button *mCancelButton = new Button(_("Cancel"), "cancel", this);

    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mScrollArea, 8, 5).setPadding(3);
    place(4, 5, mCancelButton);
    place(5, 5, mOkButton);
    reflowLayout(200, 100);

    if (n_server == 0)
        // Disable Ok button
        mOkButton->setEnabled(false);
    else
        // Select first server
        mServerList->setSelected(1);

    setLocationRelativeTo(getParent());
    setVisible(true);
    mOkButton->requestFocus();
}

ServerSelectDialog::~ServerSelectDialog()
{
    delete mServerListModel;
}

void ServerSelectDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok")
    {
        mOkButton->setEnabled(false);
        const SERVER_INFO *si = server_info[mServerList->getSelected()];
        mLoginData->hostname = ipToString(si->address);
        mLoginData->port = si->port;
        mLoginData->updateHost = si->updateHost;
        state = mNextState;
    }
    else if (event.getId() == "cancel")
        state = LOGIN_STATE;
}

int ServerListModel::getNumberOfElements()
{
    return n_server;
}

std::string ServerListModel::getElementAt(int i)
{
    const SERVER_INFO *si = server_info[i];
    return si->name + " (" + toString(si->online_users) + ")";
}
