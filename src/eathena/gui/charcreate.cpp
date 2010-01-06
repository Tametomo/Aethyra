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

#include <string>

#include "charcreate.h"

#include "../db/colordb.h"

#include "../net/messageout.h"

#include "../../bindings/guichan/layout.h"

#include "../../bindings/guichan/dialogs/okdialog.h"

#include "../../bindings/guichan/widgets/beingbox.h"
#include "../../bindings/guichan/widgets/button.h"
#include "../../bindings/guichan/widgets/container.h"
#include "../../bindings/guichan/widgets/label.h"
#include "../../bindings/guichan/widgets/textfield.h"

#include "../../core/map/sprite/player.h"

#include "../../core/utils/dtor.h"
#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

CharCreateDialog *charCreateDialog = NULL;

CharCreateDialog::CharCreateDialog(Window *parent, int slot, Gender gender):
    Window(_("Create Character"), true, parent),
    mSlot(slot)
{
    mPlayer = new Player(0, 0, NULL);
    mPlayer->setGender(gender);

    int numberOfHairColors = ColorDB::size();

    mPlayer->setHairStyle(rand() % mPlayer->getNumOfHairstyles(),
                          rand() % numberOfHairColors);

    mNameField = new TextField("");
    mNameLabel = new Label(_("Name:"));
    mNextHairColorButton = new Button(">", "nextcolor", this);
    mPrevHairColorButton = new Button("<", "prevcolor", this);
    mHairColorLabel = new Label(_("Hair Color:"));
    mNextHairStyleButton = new Button(">", "nextstyle", this);
    mPrevHairStyleButton = new Button("<", "prevstyle", this);
    mHairStyleLabel = new Label(_("Hair Style:"));
    mCreateButton = new Button(_("Create"), "create", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);
    mBeingBox = new BeingBox(mPlayer);

    mBeingBox->setWidth(74);

    mNameField->setActionEventId("create");
    mNameField->addActionListener(this);

    fontChanged();

    setLocationRelativeTo(getParent());
    setVisible(true);

    mNameField->requestFocus();
}

void CharCreateDialog::fontChanged()
{
    Window::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mNameLabel, 1);
    place(1, 0, mNameField, 6);
    place(0, 1, mHairStyleLabel, 1);
    place(1, 1, mPrevHairStyleButton);
    place(2, 1, mBeingBox, 1, 8).setPadding(3);
    place(3, 1, mNextHairStyleButton);
    place(0, 2, mHairColorLabel, 1);
    place(1, 2, mPrevHairColorButton);
    place(3, 2, mNextHairColorButton);
    place.getCell().matchColWidth(0, 2);
    place = getPlacer(0, 2);
    place(4, 0, mCancelButton);
    place(5, 0, mCreateButton);

    reflowLayout(225, 0);

    restoreFocus();
}

CharCreateDialog::~CharCreateDialog()
{
    destroy(mPlayer);
    charCreateDialog = NULL;
}

void CharCreateDialog::action(const gcn::ActionEvent &event)
{
    const int numberOfColors = ColorDB::size();
    const int numberOfHair = mPlayer->getNumOfHairstyles();
    if (event.getId() == "create")
    {
        if (getName().length() >= 4)
        {
            // Attempt to create the character
            mCreateButton->setEnabled(false);
            attemptCharCreate();
        }
        else
        {
            new OkDialog("Error",
                         "Your name needs to be at least 4 characters.", this);
        }
    }
    else if (event.getId() == "cancel")
        close();
    else if (event.getId() == "nextcolor")
        mPlayer->setHairStyle(mPlayer->getHairStyle(),
                             (mPlayer->getHairColor() + 1) % numberOfColors);
    else if (event.getId() == "prevcolor")
        mPlayer->setHairStyle(mPlayer->getHairStyle(), (mPlayer->getHairColor() +
                              numberOfColors - 1) % numberOfColors);
    else if (event.getId() == "nextstyle")
        mPlayer->setHairStyle((mPlayer->getHairStyle() + 1) % numberOfHair,
                              mPlayer->getHairColor());
    else if (event.getId() == "prevstyle")
        mPlayer->setHairStyle((mPlayer->getHairStyle() + numberOfHair - 1) %
                               numberOfHair, mPlayer->getHairColor());
}

void CharCreateDialog::close()
{
    Window::close();
    windowContainer->scheduleDelete(this);
}

std::string CharCreateDialog::getName()
{
    std::string name = mNameField->getText();
    trim(name);
    return name;
}

void CharCreateDialog::unlock()
{
    mCreateButton->setEnabled(true);
}

void CharCreateDialog::attemptCharCreate()
{
    // Send character infos
    MessageOut outMsg(0x0067);
    outMsg.writeString(getName(), 24);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(5);
    outMsg.writeInt8(mSlot);
    outMsg.writeInt16(mPlayer->getHairColor());
    outMsg.writeInt16(mPlayer->getHairStyle());
}
