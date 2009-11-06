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

#include "setup_game.h"

#include "../../../bindings/guichan/graphics.h"
#include "../../../bindings/guichan/layouthelper.h"

#include "../../../bindings/guichan/widgets/checkbox.h"
#include "../../../bindings/guichan/widgets/label.h"
#include "../../../bindings/guichan/widgets/slider.h"
#include "../../../bindings/guichan/widgets/textfield.h"

#include "../../../core/configuration.h"
#include "../../../core/log.h"

#include "../../../core/image/particle/particle.h"

#include "../../../core/map/sprite/localplayer.h"

#include "../../../core/utils/gettext.h"
#include "../../../core/utils/stringutils.h"

Setup_Game::Setup_Game():
    mNameEnabled(config.getValue("showownname", false)),
    mPickupChatEnabled(config.getValue("showpickupchat", true)),
    mPickupParticleEnabled(config.getValue("showpickupparticle", false)),
    mSpeechMode(config.getValue("speech", 3)),
    mNameCheckBox(new CheckBox(_("Show name"), mNameEnabled)),
    mSpeechSlider(new Slider(0, 3)),
    mSpeechModeLabel(new Label("")),
    mOverlayDetail(config.getValue("OverlayDetail", 2)),
    mOverlayDetailSlider(new Slider(0, 2)),
    mOverlayDetailLabel(new Label("")),
    mParticleDetail(3 - config.getValue("particleEmitterSkip", 1)),
    mParticleDetailSlider(new Slider(-1, 3)),
    mParticleDetailLabel(new Label("")),
    mPickupNotifyLabel(new Label(_("Show pickup notification"))),
    mPickupChatCheckBox(new CheckBox(_("in chat"), mPickupChatEnabled)),
    mPickupParticleCheckBox(new CheckBox(_("as particle"),
                            mPickupParticleEnabled))
{
    setName(_("Game"));

    mPickupChatCheckBox->setActionEventId("pickupchat");
    mPickupParticleCheckBox->setActionEventId("pickupparticle");
    mNameCheckBox->setActionEventId("showownname");
    mSpeechSlider->setActionEventId("speech");
    mOverlayDetailSlider->setActionEventId("overlaydetailslider");
    mParticleDetailSlider->setActionEventId("particledetailslider");

    speechLabel = new Label(_("Overhead text"));
    overlayDetailLabel = new Label(_("Ambient FX"));
    particleDetailLabel = new Label(_("Particle detail"));

    mPickupChatCheckBox->addActionListener(this);
    mPickupParticleCheckBox->addActionListener(this);
    mNameCheckBox->addActionListener(this);
    mSpeechSlider->addActionListener(this);
    mOverlayDetailSlider->addActionListener(this);
    mParticleDetailSlider->addActionListener(this);

    setSpeechModeLabel(mSpeechMode);
    mSpeechSlider->setValue(mSpeechMode);

    setOverlayDetailLabel(mOverlayDetail);
    mOverlayDetailSlider->setValue(mOverlayDetail);

    setParticleDetailLabel(mParticleDetail);
    mParticleDetailSlider->setValue(mParticleDetail);

    mSpeechSlider->setWidth(90);
    mOverlayDetailSlider->setWidth(90);
    mParticleDetailSlider->setWidth(90);

    mSpeechSlider->setStepLength(1.0);
    mOverlayDetailSlider->setStepLength(1.0);
    mParticleDetailSlider->setStepLength(1.0);

    fontChanged();

    setDimension(gcn::Rectangle(0, 0, 340, 280));
}

void Setup_Game::fontChanged()
{
    SetupTabContainer::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, mPickupNotifyLabel, 2);
    place(2, 0, mNameCheckBox, 3);
    place(0, 1, mPickupChatCheckBox, 1);
    place(1, 1, mPickupParticleCheckBox, 2);

    place(0, 2, mSpeechSlider);
    place(0, 3, mOverlayDetailSlider);
    place(0, 4, mParticleDetailSlider);

    place(1, 2, speechLabel, 2);
    place(1, 3, overlayDetailLabel, 2);
    place(1, 4, particleDetailLabel, 2);

    place(3, 2, mSpeechModeLabel, 3).setPadding(2);
    place(3, 3, mOverlayDetailLabel, 3).setPadding(2);
    place(3, 4, mParticleDetailLabel, 3).setPadding(2);

    h.reflowLayout(340, 280);
    restoreFocus();
}

void Setup_Game::apply()
{
    // We sync old and new values at apply time
    mNameEnabled = config.getValue("showownname", false);
    mPickupChatEnabled = config.getValue("showpickupchat", true);
    mPickupParticleEnabled = config.getValue("showpickupparticle", false);
    mSpeechMode = config.getValue("speech", 3);
    mOverlayDetail = config.getValue("OverlayDetail", 2);
    mParticleDetail = 3 - config.getValue("particleEmitterSkip", 1);
    config.setValue("particleeffects", mParticleDetail != -1);
}

void Setup_Game::cancel()
{
    mSpeechSlider->setValue(mSpeechMode);
    mNameCheckBox->setSelected(mNameEnabled);
    mOverlayDetailSlider->setValue(mOverlayDetail);
    mParticleDetailSlider->setValue(mParticleDetail);

    int val = (int) mSpeechSlider->getValue();
    setSpeechModeLabel(val);

    val = (int) mOverlayDetailSlider->getValue();
    setOverlayDetailLabel(val);

    changeParticleDetailLevel(mParticleDetail);

    config.setValue("showownname", mNameEnabled ? true : false);
    config.setValue("showpickupchat", mPickupChatEnabled ? true : false);
    config.setValue("showpickupparticle", mPickupParticleEnabled ?
                    true : false);
    config.setValue("speech", mSpeechMode);

    if (player_node)
        player_node->mUpdateName = true;
}

void Setup_Game::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "pickupchat")
    {
        config.setValue("showpickupchat", mPickupChatCheckBox->isSelected() ?
                                              true : false);
    }
    else if (event.getId() == "pickupparticle")
    {
        config.setValue("showpickupparticle",
                        mPickupParticleCheckBox->isSelected() ? true : false);
    }
    else if (event.getId() == "speech")
    {
        const int val = (int) mSpeechSlider->getValue();
        setSpeechModeLabel(val);        

        mSpeechSlider->setValue(val);
        config.setValue("speech", val);
    }
    else if (event.getId() == "showownname")
    {
        // Notify the local player that settings have changed for the name
        // and requires an update
        if (player_node)
            player_node->mUpdateName = true;

        config.setValue("showownname", mNameCheckBox->isSelected() ? true :
                                                                     false);
    }
    else if (event.getId() == "overlaydetailslider")
    {
        const int val = (int) mOverlayDetailSlider->getValue();
        setOverlayDetailLabel(val);
        config.setValue("OverlayDetail", val);
    }
    else if (event.getId() == "particledetailslider")
    {
        const int val = (int) mParticleDetailSlider->getValue();
        changeParticleDetailLevel(val);
    }
}

void Setup_Game::setSpeechModeLabel(const int value)
{
    switch (value)
    {
        case 0:
            mSpeechModeLabel->setCaption(_("No text"));
            break;
        case 1:
            mSpeechModeLabel->setCaption(_("Text"));
            break;
        case 2:
            mSpeechModeLabel->setCaption(_("Bubbles, no names"));
            break;
        case 3:
            mSpeechModeLabel->setCaption(_("Bubbles with names"));
            break;
    }

    mSpeechModeLabel->adjustSize();
}

void Setup_Game::setOverlayDetailLabel(const int value)
{
    switch (value)
    {
        case 0:
            mOverlayDetailLabel->setCaption(_("off"));
            break;
        case 1:
            mOverlayDetailLabel->setCaption(_("low"));
            break;
        case 2:
            mOverlayDetailLabel->setCaption(_("high"));
            break;
    }

    mOverlayDetailLabel->adjustSize();
}

void Setup_Game::setParticleDetailLabel(const int value)
{
    switch (value)
    {
        case -1:
            mParticleDetailLabel->setCaption(_("off"));
            break;                
        case 0:
            mParticleDetailLabel->setCaption(_("low"));
            break;
        case 1:
            mParticleDetailLabel->setCaption(_("medium"));
            break;
        case 2:
            mParticleDetailLabel->setCaption(_("high"));
            break;
        case 3:
            mParticleDetailLabel->setCaption(_("max"));
            break;
    }

    mParticleDetailLabel->adjustSize();
}

void Setup_Game::changeParticleDetailLevel(const int value)
{
    setParticleDetailLabel(value);
    config.setValue("particleeffects", value != -1);
    config.setValue("particleEmitterSkip", 3 - value);

    if (particleEngine)
        particleEngine->changeParticleDetailLevel(value);
}

