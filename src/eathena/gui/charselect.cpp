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
#include "charselect.h"

#include "../net/logindata.h"
#include "../net/messageout.h"

#include "../../main.h"

#include "../../bindings/guichan/layout.h"

#include "../../bindings/guichan/dialogs/confirmdialog.h"
#include "../../bindings/guichan/dialogs/okdialog.h"

#include "../../bindings/guichan/widgets/beingbox.h"
#include "../../bindings/guichan/widgets/button.h"
#include "../../bindings/guichan/widgets/label.h"

#include "../../core/configuration.h"

#include "../../core/map/sprite/localplayer.h"

#include "../../core/utils/dtor.h"
#include "../../core/utils/gettext.h"
#include "../../core/utils/lockedarray.h"
#include "../../core/utils/stringutils.h"

CharSelectDialog *charSelectDialog = NULL;

/**
 * Listener for confirming character deletion.
 */
class CharDeleteConfirm : public ConfirmDialog
{
    public:
        CharDeleteConfirm(CharSelectDialog *master);
        void action(const gcn::ActionEvent &event);
    private:
        CharSelectDialog *master;
};

CharDeleteConfirm::CharDeleteConfirm(CharSelectDialog *m):
    ConfirmDialog(_("Confirm Character Delete"),
                  _("Are you sure you want to delete this character?"), m),
    master(m)
{
}

void CharDeleteConfirm::action(const gcn::ActionEvent &event)
{
    //ConfirmDialog::action(event);
    if (event.getId() == "yes")
        master->attemptCharDelete();

    ConfirmDialog::action(event);
}

CharSelectDialog::CharSelectDialog(LockedArray<LocalPlayer*> *charInfo,
                                   Gender gender):
    Window(_("Select Character")),
    mCharInfo(charInfo),
    mGender(gender),
    mCharSelected(false)
{
    charSelectDialog = this;

    // Control that shows the Player
    mBeingBox = new BeingBox();
    mBeingBox->setWidth(74);

    mNameLabel = new Label(strprintf(_("Name: %s"), ""));
    mLevelLabel = new Label(strprintf(_("Level: %d"), 0));
    mJobLevelLabel = new Label(strprintf(_("Job Level: %d"), 0));
    mMoneyLabel = new Label(strprintf(_("Money: %d"), 0));

    mPreviousButton = new Button(_("Previous"), "previous", this);
    mNextButton = new Button(_("Next"), "next", this);
    mNewDelCharButton = new Button(_("New"), "newdel", this);
    mSelectButton = new Button(_("OK"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    selectByName(config.getValue("lastCharacter", ""));
    updatePlayerInfo();
}

CharSelectDialog::~CharSelectDialog()
{
    mCharInfo->clear();

    charSelectDialog = NULL;
}

void CharSelectDialog::fontChanged()
{
    Window::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mBeingBox, 1, 6).setPadding(3);
    place(1, 0, mNewDelCharButton);
    place(1, 1, mNameLabel, 5);
    place(1, 2, mLevelLabel, 5);
    place(1, 3, mJobLevelLabel, 5);
    place(1, 4, mMoneyLabel, 5);
    place.getCell().matchColWidth(1, 4);
    place = getPlacer(0, 2);
    place(0, 0, mPreviousButton);
    place(1, 0, mNextButton);
    place(4, 0, mCancelButton);
    place(5, 0, mSelectButton);

    reflowLayout(250, 0);

    restoreFocus();
}

void CharSelectDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "ok" && loginData.slots > 0)
    {
        // Start game
        mNewDelCharButton->setEnabled(false);
        mSelectButton->setEnabled(false);
        mPreviousButton->setEnabled(false);
        mNextButton->setEnabled(false);
        mCancelButton->setEnabled(false);
        mCharSelected = true;
        attemptCharSelect();
    }
    else if (event.getId() == "cancel")
        state = EXIT_STATE;
    else if (event.getId() == "newdel")
    {
        // Check for a character
        if (mCharInfo->getEntry())
        {
            new CharDeleteConfirm(this);
        }
        // Start new character dialog
        else if (loginData.slots < MAX_PLAYER_SLOTS && !charCreateDialog)
        {
            charCreateDialog = new CharCreateDialog(this, mCharInfo->getPos(),
                                                    mGender);
        }
    }
    else if (event.getId() == "previous")
    {
        mCharInfo->prev();
        updatePlayerInfo();
    }
    else if (event.getId() == "next")
    {
        mCharInfo->next();
        updatePlayerInfo();
    }
}

void CharSelectDialog::updatePlayerInfo()
{
    LocalPlayer *pi = mCharInfo->getEntry();

    if (pi)
    {
        mNameLabel->setCaption(strprintf(_("Name: %s"), pi->getName().c_str()));
        mLevelLabel->setCaption(strprintf(_("Level: %d"), pi->mLevel));
        mJobLevelLabel->setCaption(strprintf(_("Job Level: %d"), pi->mJobLevel));
        mMoneyLabel->setCaption(strprintf(_("Gold: %d"), pi->mGp));
        if (!mCharSelected)
        {
            mNewDelCharButton->setCaption(_("Delete"));
            mNewDelCharButton->adjustSize();
            fontChanged();
            mSelectButton->setEnabled(true);
            mSelectButton->requestFocus();
        }
    }
    else
    {
        mNameLabel->setCaption(strprintf(_("Name: %s"), ""));
        mLevelLabel->setCaption(strprintf(_("Level: %d"), 0));
        mJobLevelLabel->setCaption(strprintf(_("Job Level: %d"), 0));
        mMoneyLabel->setCaption(strprintf(_("Money: %s"), ""));
        mNewDelCharButton->setCaption(_("New"));
        mNewDelCharButton->adjustSize();
        fontChanged();
        mSelectButton->setEnabled(false);
    }

    mBeingBox->setBeing(pi);
}

void CharSelectDialog::attemptCharDelete()
{
    // Request character deletion
    MessageOut outMsg(0x0068);
    outMsg.writeInt32(mCharInfo->getEntry()->mCharId);
    outMsg.writeString("a@a.com", 40);
    mCharInfo->lock();
}

void CharSelectDialog::attemptCharSelect()
{
    // Request character selection
    MessageOut outMsg(0x0066);
    outMsg.writeInt8(mCharInfo->getPos());
    mCharInfo->lock();
}

void CharSelectDialog::requestFocus()
{
    Window::requestFocus();

    if (mSelectButton->isEnabled())
        mSelectButton->requestFocus();
    else
        mNewDelCharButton->requestFocus();
}

bool CharSelectDialog::selectByName(const std::string &name)
{
    if (mCharInfo->isLocked())
        return false;

    unsigned int oldPos = mCharInfo->getPos();

    mCharInfo->select(0);
    do
    {
        LocalPlayer *player = mCharInfo->getEntry();

        if (player && player->getName() == name)
            return true;

        mCharInfo->next();
    } while (mCharInfo->getPos());

    mCharInfo->select(oldPos);

    return false;
}

