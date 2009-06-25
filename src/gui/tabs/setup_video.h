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

#ifndef GUI_SETUP_VIDEO_H
#define GUI_SETUP_VIDEO_H

#include <guichan/actionlistener.hpp>

#include "setuptabcontainer.h"

#include "../../bindings/guichan/guichanfwd.h"

class ModeListModel;

class Setup_Video : public SetupTabContainer, public gcn::ActionListener
{
    public:
        Setup_Video();

        void apply();
        void cancel();

        void action(const gcn::ActionEvent &event);

    private:
        void setSpeechModeLabel(const int &value);
        void setOverlayDetailLabel(const int &value);
        void setParticleDetailLabel(const int &value);

        void changeParticleDetailLevel(const int &value);

        void changeResolution(const int &width, const int &height);

        bool mFullScreenEnabled;
        bool mOpenGLEnabled;
        bool mCustomCursorEnabled;
        bool mNameEnabled;
        bool mPickupChatEnabled;
        bool mPickupParticleEnabled;
        double mOpacity;
        double mMouseOpacity;
        int mFps;
        int mSpeechMode;
        int mScreenWidth;
        int mScreenHeight;

        ModeListModel *mModeListModel;

        gcn::Label *speechLabel;
        gcn::Label *alphaLabel;
        gcn::Label *mouseAlphaLabel;
        gcn::Label *fontSizeLabel;
        gcn::Label *overlayDetailLabel;
        gcn::Label *particleDetailLabel;

        gcn::ListBox *mModeList;
        gcn::CheckBox *mFsCheckBox;
        gcn::CheckBox *mOpenGLCheckBox;
        gcn::CheckBox *mCustomCursorCheckBox;
        gcn::CheckBox *mNameCheckBox;

        gcn::Slider *mSpeechSlider;
        gcn::Label *mSpeechModeLabel;
        gcn::Slider *mAlphaSlider;
        gcn::Slider *mMouseAlphaSlider;
        gcn::CheckBox *mFpsCheckBox;
        gcn::Slider *mFpsSlider;
        gcn::TextField *mFpsField;

        int mFontSize;
        gcn::Slider *mFontSizeSlider;
        gcn::Label *mFontSizeLabel;

        int mOverlayDetail;
        gcn::Slider *mOverlayDetailSlider;
        gcn::Label *mOverlayDetailLabel;

        int mParticleDetail;
        gcn::Slider *mParticleDetailSlider;
        gcn::Label *mParticleDetailLabel;

        gcn::Label *mPickupNotifyLabel;
        gcn::CheckBox *mPickupChatCheckBox;
        gcn::CheckBox *mPickupParticleCheckBox;
};

#endif
