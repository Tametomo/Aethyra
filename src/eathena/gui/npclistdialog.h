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

#ifndef GUI_NPCLISTDIALOG_H
#define GUI_NPCLISTDIALOG_H

#include "../../bindings/guichan/dialogs/listdialog.h"

/**
 * The npc list dialog.
 *
 * \ingroup Interface
 */
class NpcListDialog : public ListDialog
{
    public:
        /**
         * Constructor.
         *
         * @see ListDialog::ListDialog()
         */
        NpcListDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Fills the options list for an NPC dialog.
         *
         * @param itemString A string with the options separated with colons.
         */
        void parseItems(const std::string &itemString);

        /**
         * Resets the list by removing all items.
         */
        void reset();

        /**
         * Readjust the window dimensions in case the NPC text dialog was
         * resized.
         */
        void widgetShown(const gcn::Event& event);

        /**
         * Overridden close() method which cleans up the NPC List dialog on
         * close or losing visibility.
         */
        void close();

    private:
        int mChoice;
};

extern NpcListDialog *npcListDialog;

#endif // GUI_NPCLISTDIALOG_H
