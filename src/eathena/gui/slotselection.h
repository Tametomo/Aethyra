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

#ifndef SLOT_SELECTION_WINDOW_H
#define ITEM_AMOUNT_WINDOW_H

#include <guichan/actionlistener.hpp>

#include "../../bindings/guichan/widgets/window.h"

#define ITEM_SHORTCUT  1
#define EMOTE_SHORTCUT 2

/**
 * Window used for selecting which slot to place a shortcut in on the shortcut
 * window.
 *
 * \ingroup Interface
 */
class SlotSelectionWindow : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        SlotSelectionWindow(int use, Window *parent, int id);

        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Reselects the first slot.
         */
        void reset();

        /**
         * Schedules the slot selection window for deletion.
         */
        void close();

        void fontChanged();
    private:
        gcn::Label *mSlotLabel;  /**< Slot caption. */
        gcn::Slider *mSlotSlide; /**< Slider containing the selected slot */

        gcn::Button *mOkButton;
        gcn::Button *mCancelButton;

        int mMaxSlot;
        int mUsage;              /**< What kind of shortcut container */
        int mId;                 /**< Item or emote # */
};

#endif /* ITEM_AMOUNT_WINDOW_H */
