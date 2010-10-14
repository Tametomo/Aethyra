/*
 *  Aethyra
 *  Copyright (C) 2010  Aethyra Development Team
 *
 *  This file is part of Aethyra.
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

#ifndef SELECTION_DIALOG_H
#define SELECTION_DIALOG_H

#include <vector>

#include "../widgets/window.h"

class RadioButton;
class TextBox;

/**
 * A selection dialog.
 *
 * \ingroup GUI
 */
class SelectionDialog : public Window
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        SelectionDialog(const std::string &title, const std::string &msg,
                        Window *parent = NULL, bool modal = false);

        virtual void close();

        unsigned int getNumRows();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        void fontChanged();

        void reflow();

        /**
         * Adds an option to select from.
         */
        void addOption(std::string key, std::string label);
    private:
        TextBox *mTextBox;
        gcn::Button *mOkButton;
        std::vector<RadioButton*> mRadioButtons;
        std::string mDialogKey;
        static int mInstances;
};

#endif
