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

#ifndef _CHAR_SELECT_H
#define _CHAR_SELECT_H

#include "../../bindings/guichan/widgets/window.h"

#include "../../core/map/sprite/being.h"

class BeingBox;
class LocalPlayer;

template<class T>
class LockedArray;

// TODO De-hardcode this value and have it settable depending on the server
//      configuration.
#define MAX_PLAYER_SLOTS 3

/**
 * Character selection dialog.
 *
 * \ingroup Interface
 */
class CharSelectDialog : public Window
{
    public:
        friend class CharDeleteConfirm;
        /**
         * Constructor.
         */
        CharSelectDialog(LockedArray<LocalPlayer*> *charInfo, Gender gender);

        virtual ~CharSelectDialog();

        void action(const gcn::ActionEvent &event);

        void updatePlayerInfo();

        bool selectByName(const std::string &name);

        /**
         * Focuses on the OK button when there is a character to select,
         * otherwise, it selects the new button.
         */
        void requestFocus();

        void fontChanged();

    private:
        LockedArray<LocalPlayer*> *mCharInfo;

        gcn::Button *mSelectButton;
        gcn::Button *mCancelButton;
        gcn::Button *mNewDelCharButton;
        gcn::Button *mPreviousButton;
        gcn::Button *mNextButton;

        gcn::Label *mNameLabel;
        gcn::Label *mLevelLabel;
        gcn::Label *mJobLevelLabel;
        gcn::Label *mMoneyLabel;

        BeingBox *mBeingBox;

        Gender mGender;
        bool mCharSelected;

        /**
         * Communicate character deletion to the server.
         */
        void attemptCharDelete();

        /**
         * Communicate character selection to the server.
         */
        void attemptCharSelect();
};

extern CharSelectDialog *charSelectDialog;

#endif
