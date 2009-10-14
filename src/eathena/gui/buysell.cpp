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

#include "buysell.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../../bindings/guichan/widgets/button.h"

#include "../../core/map/sprite/npc.h"

#include "../../core/utils/gettext.h"

static const char *buttonNames[] = {
    N_("Buy"),
    N_("Sell"),
    N_("Cancel"),
    0
};

BuySellDialog::BuySellDialog():
    Window(_("Shop"))
{
    setWindowName("BuySell");
    saveVisibility(false);

    for (const char **curBtn = buttonNames; *curBtn; curBtn++)
    {
        Button *btn = new Button(gettext(*curBtn), *curBtn, this);
        mButtons.push_back(btn);
    }

    fontChanged();
    loadWindowState();
}

void BuySellDialog::fontChanged()
{
    Window::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    int x = 10, y = 10;
    for (size_t i = 0; i < mButtons.size(); i++)
    {
        mButtons[i]->setPosition(x, y);
        add(mButtons[i]);
        x += mButtons[i]->getWidth() + 10;
    }

    setDefaultSize(x + getPadding(), (2 * y + mButtons[0]->getHeight() +
                   getTitleBarHeight()), ImageRect::CENTER);

    setContentSize(x, (2 * y + mButtons[0]->getHeight()));
}

void BuySellDialog::logic()
{
    Window::logic();

    if (isVisible() && !current_npc)
        setVisible(false);
}

void BuySellDialog::requestFocus()
{
    if (mButtons.size() > 0)
        mButtons[0]->requestFocus();
}

void BuySellDialog::action(const gcn::ActionEvent &event)
{
    setVisible(false);
    int action = 0;

    NPC::mTalking = false;

    if (event.getId() == "Buy")
        action = 0;
    else if (event.getId() == "Sell")
        action = 1;
    else if (event.getId() == "Cancel")
    {
        current_npc = 0;
        return;
    }

    MessageOut outMsg(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(current_npc);
    outMsg.writeInt8(action);
}
