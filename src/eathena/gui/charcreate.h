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

#ifndef _CHAR_CREATE_H
#define _CHAR_CREATE_H

#include <guichan/actionlistener.hpp>

#include "../../bindings/guichan/widgets/window.h"

#include "../../core/map/sprite/being.h"

class BeingBox;
class Player;

/**
 * Character creation dialog.
 *
 * \ingroup GUI
 */
class CharCreateDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        CharCreateDialog(Window *parent, int slot, Gender gender);

        /**
         * Destructor.
         */
        ~CharCreateDialog();

        virtual void close();

        void action(const gcn::ActionEvent &event);

        /**
         * Unlocks the dialog, enabling the create character button again.
         */
        void unlock();

        void fontChanged();
    private:
        /**
         * Returns the name of the character to create.
         */
        std::string getName();

        /**
         * Communicate character creation to the server.
         */
        void attemptCharCreate();

        gcn::TextField *mNameField;
        gcn::Label *mNameLabel;
        gcn::Button *mNextHairColorButton;
        gcn::Button *mPrevHairColorButton;
        gcn::Label *mHairColorLabel;
        gcn::Button *mNextHairStyleButton;
        gcn::Button *mPrevHairStyleButton;
        gcn::Label *mHairStyleLabel;
        gcn::Button *mCreateButton;
        gcn::Button *mCancelButton;

        Player *mPlayer;
        BeingBox *mBeingBox;

        int mSlot;
};

extern CharCreateDialog *charCreateDialog;

#endif
