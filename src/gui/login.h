/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef WIN32
  #pragma warning (disable:4312)
#endif

#ifndef _LOGIN_H
#define _LOGIN_H 

#include "../main.h"
#include "../log.h"
#include "../net/network.h"
#include "gui.h"
#include "window.h"
#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif

/**
 * The login dialog.
 *
 * \ingroup GUI
 */
class LoginDialog : public Window, public gcn::ActionListener {
    public:
        LoginDialog();
        ~LoginDialog();

        /**
         * Initializes the dialog. Should be called after adding it to the GUI.
         */
        void init();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId);

    private:
        gcn::Label *userLabel;
        gcn::Label *passLabel;
        gcn::TextField *userField;
        gcn::TextField *passField;
        gcn::CheckBox *keepCheck;
        gcn::Button *okButton;
        gcn::Button *cancelButton;
};

void login();
void server_login(const std::string& user, const std::string& pass);

#endif
