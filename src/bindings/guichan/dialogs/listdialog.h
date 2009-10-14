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

#ifndef LISTDIALOG_H
#define LISTDIALOG_H

#include <vector>

#include <guichan/actionlistener.hpp>
#include <guichan/listmodel.hpp>

#include "../widgets/window.h"

/**
 * A dialog box for displaying simple lists.
 *
 * \ingroup Interface
 */
class ListDialog : public Window, public gcn::ActionListener,
                   public gcn::ListModel
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window()
         */
        ListDialog(const std::string &caption);

        /**
         * Called when receiving actions from the widgets.
         */
        virtual void action(const gcn::ActionEvent &event);

        /**
         * Returns the number of items in the choices list.
         */
        int getNumberOfElements();

        /**
         * Returns the name of item number i of the choices list.
         */
        std::string getElementAt(int i);

        /**
         * Gets the currently selected element.
         */
        const int getSelected();

        /**
         * Fills the options list for an NPC dialog.
         *
         * @param itemString A string with the options separated with colons.
         */
        void addOption(const std::string &option);

        /**
         * Resets the list by removing all items.
         */
        virtual void reset();

        /**
         * Called when the dialog is visible to allow for mItemList to request
         * focus.
         */
        void requestFocus();

        /**
         * Selects the first item in the list on shown events.
         */
        virtual void widgetShown(const gcn::Event& event);

        /**
         * Clears the list dialog on hiding
         */
        virtual void widgetHidden(const gcn::Event& event);

        void fontChanged();
    private:
        gcn::ListBox *mOptionList;
        gcn::ScrollArea *mScrollArea;
        gcn::Button *mOkButton;
        gcn::Button *mCancelButton;

        std::vector<std::string> mOptions;
};

#endif // LISTDIALOG_H
