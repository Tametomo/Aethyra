/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra derived from original code
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

#include "slotselection.h"

#include "../emoteshortcut.h"
#include "../itemshortcut.h"

#include "../bindings/guichan/layout.h"

#include "../bindings/guichan/widgets/button.h"
#include "../bindings/guichan/widgets/label.h"
#include "../bindings/guichan/widgets/slider.h"

#include "../utils/gettext.h"
#include "../utils/stringutils.h"

SlotSelectionWindow::SlotSelectionWindow(int use, Window *parent, int id):
    Window("", true, parent),
    mUsage(use),
    mId(id)
{
    setCloseButton(true);

    switch (use)
    {
        case ITEM_SHORTCUT:
            setCaption(_("Select item shortcut slot to use."));
            mMaxSlot = SHORTCUT_ITEMS;
            break;
        case EMOTE_SHORTCUT:
            setCaption(_("Select emote shortcut slot to use."));
            mMaxSlot = SHORTCUT_EMOTES;
            break;
    }

    // Integer field
    mSlotLabel = new Label(strprintf("%d / %d", 1, mMaxSlot));
    mSlotLabel->setAlignment(gcn::Graphics::CENTER);

    // Slider
    mSlotSlide = new Slider(1.0, mMaxSlot);
    mSlotSlide->setStepLength(1.0);
    mSlotSlide->setHeight(10);
    mSlotSlide->setActionEventId("Slide");
    mSlotSlide->addActionListener(this);

    // Buttons
    Button *okButton = new Button(_("Ok"), "Ok", this);
    Button *cancelButton = new Button(_("Cancel"), "Cancel", this);

    // Set positions
    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mSlotSlide, 5);
    place(5, 0, mSlotLabel, 2);
    place = getPlacer(0, 1);
    place(4, 0, cancelButton);
    place(5, 0, okButton);

    reflowLayout(225, 0);

    setLocationRelativeTo(getParentWindow());
    setVisible(true);

    mSlotSlide->requestFocus();
}

void SlotSelectionWindow::resetAmount()
{
    mSlotSlide->setValue(1);
    mSlotLabel->setCaption(strprintf("%d / %d", 1, mMaxSlot));
}

void SlotSelectionWindow::action(const gcn::ActionEvent &event)
{
    int amount = mSlotSlide->getValue();

    if (event.getId() == "Cancel")
        close();
    else if (event.getId() == "Ok")
    {
        switch (mUsage)
        {
            case ITEM_SHORTCUT:
                itemShortcut->setItems(amount - 1, mId);
                break;
            case EMOTE_SHORTCUT:
                emoteShortcut->setEmotes(amount - 1, mId);
                break;
        }
        close();
    }

    mSlotLabel->setCaption(strprintf("%d / %d", amount, mMaxSlot));
    mSlotSlide->setValue(amount);
}

void SlotSelectionWindow::close()
{
    scheduleDelete();
}
