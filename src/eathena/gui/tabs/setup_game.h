/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *  Copyright (C) 2009  Aethyra Development Team
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

#ifndef GUI_SETUP_GAME_H
#define GUI_SETUP_GAME_H

#include <guichan/actionlistener.hpp>

#include "../../../bindings/guichan/guichanfwd.h"

#include "../../../bindings/guichan/widgets/setuptabcontainer.h"

class Setup_Game : public SetupTabContainer, public gcn::ActionListener
{
    public:
        Setup_Game();

        void apply();
        void cancel();

        void action(const gcn::ActionEvent &event);

        void fontChanged();
    private:
        void setSpeechModeLabel(const int value);
        void setOverlayDetailLabel(const int value);
        void setParticleDetailLabel(const int value);
        void changeParticleDetailLevel(const int value);

        bool mNameEnabled;
        bool mPickupChatEnabled;
        bool mPickupParticleEnabled;
        int mSpeechMode;

        gcn::Label *speechLabel;
        gcn::Label *overlayDetailLabel;
        gcn::Label *particleDetailLabel;

        gcn::CheckBox *mNameCheckBox;

        gcn::Slider *mSpeechSlider;
        gcn::Label *mSpeechModeLabel;

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

extern Setup_Game *setupGame;

#endif
