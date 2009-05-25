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

#include "npcintegerdialog.h"

#include "../bindings/guichan/layout.h"

#include "../bindings/guichan/widgets/button.h"
#include "../bindings/guichan/widgets/inttextfield.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../resources/sprite/npc.h"

#include "../utils/gettext.h"

NpcIntegerDialog::NpcIntegerDialog():
    Window(_("NPC Input"))
{
    setWindowName("NPCInput");
    saveVisibility(false);

    mValueField = new IntTextField(0, "ok", this);

    okButton = new Button(_("OK"), "ok", this);
    cancelButton = new Button(_("Cancel"), "cancel", this);
    resetButton = new Button(_("Reset"), "reset", this);

    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mValueField, 3);
    place.getCell().matchColWidth(1, 0);
    place = getPlacer(0, 1);
    place(0, 0, resetButton);
    place(2, 0, cancelButton);
    place(3, 0, okButton);

    setDefaultSize(175, 75, ImageRect::CENTER);

    loadWindowState();
}

void NpcIntegerDialog::setRange(const int min, const int max)
{
    mValueField->setRange(min, max);
}

int NpcIntegerDialog::getValue()
{
    return mValueField->getValue();
}

void NpcIntegerDialog::reset()
{
    mValueField->reset();
}

void NpcIntegerDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "reset")
    {
        mValueField->reset();
        return;
    }
    else if (event.getId() == "cancel")
        mValueField->reset();

    close();
}

void NpcIntegerDialog::requestFocus()
{
    mValueField->requestFocus();
}

void NpcIntegerDialog::close()
{
    Window::close();
    NPC::mTalking = false;

    if (!current_npc)
        return;

    MessageOut outMsg(CMSG_NPC_INT_RESPONSE);
    outMsg.writeInt32(current_npc);
    outMsg.writeInt32(mValueField->getValue());

    current_npc = 0;
    mValueField->reset();
}
