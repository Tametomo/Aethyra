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

#ifndef GUI_SETUP_GUI_H
#define GUI_SETUP_GUI_H

#include <guichan/actionlistener.hpp>

#include "../../guichanfwd.h"

#include "../../widgets/setuptabcontainer.h"

class ModeListModel;

class Setup_Gui : public SetupTabContainer, public gcn::ActionListener
{
    public:
        Setup_Gui();

        void apply();
        void cancel();

        void action(const gcn::ActionEvent &event);

        void fontChanged();
    private:
        bool mFullScreenEnabled;
        bool mOpenGLEnabled;
        bool mCustomCursorEnabled;
        double mOpacity;
        double mMouseOpacity;
        int mFps;
        int mScreenWidth;
        int mScreenHeight;

        ModeListModel *mModeListModel;

        gcn::Label *mAlphaLabel;
        gcn::Label *mMouseAlphaLabel;
        gcn::Label *mFontLabel;

        gcn::ScrollArea *mScrollArea;

        gcn::ListBox *mModeList;
        gcn::CheckBox *mFsCheckBox;
        gcn::CheckBox *mOpenGLCheckBox;
        gcn::CheckBox *mCustomCursorCheckBox;

        gcn::Slider *mAlphaSlider;
        gcn::Slider *mMouseAlphaSlider;
        gcn::CheckBox *mFpsCheckBox;
        gcn::Slider *mFpsSlider;
        gcn::TextField *mFpsField;

        int mFontSize;
        gcn::Slider *mFontSizeSlider;
        gcn::Label *mFontSizeLabel;
};

#endif
