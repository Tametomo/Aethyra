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

#ifndef HELP_H
#define HELP_H

#include "../handlers/linkhandler.h"

#include "../widgets/window.h"

class RichTextBox;

/**
 * The help dialog.
 */
class HelpDialog : public Window, public LinkHandler
{
    public:
        /**
         * Constructor.
         */
        HelpDialog();

        /**
         * Handles link action.
         */
        void handleLink(const std::string& link);

        /**
         * Loads help in the dialog.
         */
        void loadHelp(const std::string &helpFile);

        /**
         * Focuses on the close button on gaining focus.
         */
        void requestFocus();

        /**
         * Loads help index file on close.
         */
        void close();

        void fontChanged();
    private:
        void loadFile(const std::string &file);

        RichTextBox* mRichTextBox;
        gcn::ScrollArea *mScrollArea;
        gcn::Button *mOkButton;
};

extern HelpDialog *helpDialog;

#endif
