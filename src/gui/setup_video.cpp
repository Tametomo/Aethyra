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

#include <string>

#include <guichan/key.hpp>

#include "ok_dialog.h"
#include "setup_video.h"

#include "../beingmanager.h"
#include "../configuration.h"
#include "../engine.h"
#include "../localplayer.h"
#include "../log.h"
#include "../main.h"
#include "../map.h"
#include "../particle.h"

#include "../bindings/guichan/graphics.h"
#include "../bindings/guichan/gui.h"
#include "../bindings/guichan/layouthelper.h"

#include "../bindings/guichan/models/modelistmodel.h"

#include "../bindings/guichan/widgets/checkbox.h"
#include "../bindings/guichan/widgets/label.h"
#include "../bindings/guichan/widgets/listbox.h"
#include "../bindings/guichan/widgets/scrollarea.h"
#include "../bindings/guichan/widgets/slider.h"
#include "../bindings/guichan/widgets/textfield.h"

#include "../utils/gettext.h"
#include "../utils/stringutils.h"

Setup_Video::Setup_Video():
    mFullScreenEnabled(config.getValue("screen", false)),
    mOpenGLEnabled(config.getValue("opengl", false)),
    mCustomCursorEnabled(config.getValue("customcursor", true)),
    mNameEnabled(config.getValue("showownname", false)),
    mPickupChatEnabled(config.getValue("showpickupchat", true)),
    mPickupParticleEnabled(config.getValue("showpickupparticle", false)),
    mOpacity(config.getValue("guialpha", 0.8)),
    mFps((int) config.getValue("fpslimit", 0)),
    mSpeechMode((int) config.getValue("speech", 3)),
    mModeListModel(new ModeListModel),
    mModeList(new ListBox(mModeListModel, "videomode", this)),
    mFsCheckBox(new CheckBox(_("Full screen"), mFullScreenEnabled)),
    mOpenGLCheckBox(new CheckBox(_("OpenGL"), mOpenGLEnabled)),
    mCustomCursorCheckBox(new CheckBox(_("Custom cursor"), mCustomCursorEnabled)),
    mNameCheckBox(new CheckBox(_("Show name"), mNameEnabled)),
    mSpeechSlider(new Slider(0, 3)),
    mSpeechModeLabel(new Label("")),
    mAlphaSlider(new Slider(0.2, 1.0)),
    mFpsCheckBox(new CheckBox(_("FPS Limit:"))),
    mFpsSlider(new Slider(10, 120)),
    mFpsField(new TextField),
    mOverlayDetail((int) config.getValue("OverlayDetail", 2)),
    mOverlayDetailSlider(new Slider(0, 2)),
    mOverlayDetailLabel(new Label("")),
    mParticleDetail(3 - (int) config.getValue("particleEmitterSkip", 1)),
    mParticleDetailSlider(new Slider(-1, 3)),
    mParticleDetailLabel(new Label("")),
    mPickupNotifyLabel(new Label(_("Show pickup notification"))),
    mPickupChatCheckBox(new CheckBox(_("in chat"), mPickupChatEnabled)),
    mPickupParticleCheckBox(new CheckBox(_("as particle"),
                           mPickupParticleEnabled))
{
    setName(_("Video"));

    ScrollArea *scrollArea = new ScrollArea(mModeList);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setWidth(90);

    speechLabel = new Label(_("Overhead text"));
    alphaLabel = new Label(_("Gui opacity"));
    overlayDetailLabel = new Label(_("Ambient FX"));
    particleDetailLabel = new Label(_("Particle Detail"));
    mFpsLabel = new Label("");

    mModeList->setEnabled(true);

#ifndef USE_OPENGL
    mOpenGLCheckBox->setEnabled(false);
#endif

    mAlphaSlider->setValue(mOpacity);

    mFpsField->setText(toString(mFps));
    mFpsField->setWidth(30);
    mFpsSlider->setValue(mFps);
    mFpsSlider->setEnabled(mFps > 0);
    mFpsCheckBox->setSelected(mFps > 0);

    mCustomCursorCheckBox->setActionEventId("customcursor");
    mPickupChatCheckBox->setActionEventId("pickupchat");
    mPickupParticleCheckBox->setActionEventId("pickupparticle");
    mNameCheckBox->setActionEventId("showownname");
    mAlphaSlider->setActionEventId("guialpha");
    mFpsCheckBox->setActionEventId("fpslimitcheckbox");
    mSpeechSlider->setActionEventId("speech");
    mFpsSlider->setActionEventId("fpslimitslider");
    mOverlayDetailSlider->setActionEventId("overlaydetailslider");
    mOverlayDetailLabel->setActionEventId("OverlayDetailLabel");
    mParticleDetailSlider->setActionEventId("particledetailslider");
    mParticleDetailLabel->setActionEventId("particledetailfield");

    mCustomCursorCheckBox->addActionListener(this);
    mPickupChatCheckBox->addActionListener(this);
    mPickupParticleCheckBox->addActionListener(this);
    mNameCheckBox->addActionListener(this);
    mAlphaSlider->addActionListener(this);
    mFpsCheckBox->addActionListener(this);
    mSpeechSlider->addActionListener(this);
    mFpsSlider->addActionListener(this);
    mOverlayDetailSlider->addActionListener(this);
    mParticleDetailSlider->addActionListener(this);

    setSpeechModeLabel(mSpeechMode);
    mSpeechSlider->setValue(mSpeechMode);

    setOverlayDetailLabel(mOverlayDetail);
    mOverlayDetailSlider->setValue(mOverlayDetail);

    setParticleDetailLabel(mParticleDetail);
    mParticleDetailSlider->setValue(mParticleDetail);

    // Do the layout
    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, scrollArea, 1, 5).setPadding(2);
    place(1, 0, mFsCheckBox, 2);
    place(3, 0, mOpenGLCheckBox, 1);
    place(1, 1, mCustomCursorCheckBox, 3);
    place(1, 2, mNameCheckBox, 3);
    place(1, 3, mPickupNotifyLabel, 3);
    place(1, 4, mPickupChatCheckBox, 1);
    place(2, 4, mPickupParticleCheckBox, 2);

    place(0, 6, mAlphaSlider);
    place(0, 7, mFpsSlider);
    place(0, 8, mSpeechSlider);
    place(0, 9, mOverlayDetailSlider);
    place(0, 10, mParticleDetailSlider);

    place(1, 6, alphaLabel, 3);
    place(1, 7, mFpsCheckBox).setPadding(3);
    place(1, 8, speechLabel);
    place(1, 9, overlayDetailLabel);
    place(1, 10, particleDetailLabel);

    place(2, 7, mFpsField).setPadding(1);
    place(2, 8, mSpeechModeLabel, 3).setPadding(2);
    place(2, 9, mOverlayDetailLabel, 3).setPadding(2);
    place(2, 10, mParticleDetailLabel, 3).setPadding(2);

    place(3, 7, mFpsLabel);

    setDimension(gcn::Rectangle(0, 0, 325, 280));
}

void Setup_Video::apply()
{
    // Full screen changes
    bool fullscreen = mFsCheckBox->isSelected();
    if (fullscreen != (config.getValue("screen", false) == 1))
    {
        /* The OpenGL test is only necessary on Windows, since switching
         * to/from full screen works fine on Linux. On Windows we'd have to
         * reinitialize the OpenGL state and reload all textures.
         *
         * See http://libsdl.org/cgi/docwiki.cgi/SDL_SetVideoMode
         */

#ifdef WIN32
        // checks for opengl usage
        if (!(config.getValue("opengl", false) == 1))
        {
#endif
            if (!graphics->setFullscreen(fullscreen))
            {
                fullscreen = !fullscreen;
                if (!graphics->setFullscreen(fullscreen))
                {
                    std::stringstream error;
                    error << _("Failed to switch to ") <<
                        (fullscreen ? _("windowed") : _("fullscreen")) <<
                        _("mode and restoration of old mode also failed!") <<
                        std::endl;
                    logger->error(error.str());
                }
            }
#ifdef WIN32
        }
        else
        {
            new OkDialog(_("Switching to full screen"),
                         _("Restart needed for changes to take effect."));
        }
#endif
        config.setValue("screen", fullscreen ? true : false);
    }

    // OpenGL change
    if (mOpenGLCheckBox->isSelected() != mOpenGLEnabled)
    {
        config.setValue("opengl", mOpenGLCheckBox->isSelected() ? true : false);

        // OpenGL can currently only be changed by restarting, notify user.
        new OkDialog(_("Changing OpenGL"),
                     _("Applying change to OpenGL requires restart."));
    }

    // FPS change
    config.setValue("fpslimit", mFps);

    // We sync old and new values at apply time
    mFullScreenEnabled = config.getValue("screen", false);
    mCustomCursorEnabled = config.getValue("customcursor", true);
    mNameEnabled = config.getValue("showownname", false);
    mSpeechMode = (int) config.getValue("speech", 3);
    mOpacity = config.getValue("guialpha", 0.8);
    mOverlayDetail = (int) config.getValue("OverlayDetail", 2);
    mOpenGLEnabled = config.getValue("opengl", false);
    mPickupChatEnabled = config.getValue("showpickupchat", true);
    mPickupParticleEnabled = config.getValue("showpickupparticle", false);
}

void Setup_Video::cancel()
{
    mFsCheckBox->setSelected(mFullScreenEnabled);
    mOpenGLCheckBox->setSelected(mOpenGLEnabled);
    mCustomCursorCheckBox->setSelected(mCustomCursorEnabled);
    mSpeechSlider->setValue(mSpeechMode);
    mNameCheckBox->setSelected(mNameEnabled);
    mAlphaSlider->setValue(mOpacity);
    mOverlayDetailSlider->setValue(mOverlayDetail);
    mParticleDetailSlider->setValue(mParticleDetail);

    config.setValue("screen", mFullScreenEnabled ? true : false);
    config.setValue("customcursor", mCustomCursorEnabled ? true : false);
    config.setValue("speech", mSpeechMode);
    config.setValue("showownname", mNameEnabled ? true : false);
    config.setValue("guialpha", mOpacity);
    config.setValue("opengl", mOpenGLEnabled ? true : false);
    config.setValue("showpickupchat", mPickupChatEnabled ? true : false);
    config.setValue("showpickupparticle", mPickupParticleEnabled ?
                    true : false);

    if (player_node)
        player_node->mUpdateName = true;
}

void Setup_Video::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "videomode")
    {
        const std::string mode = mModeListModel->getElementAt(mModeList->getSelected());
        const int width = atoi(mode.substr(0, mode.find("x")).c_str());
        const int height = atoi(mode.substr(mode.find("x") + 1).c_str());

        if (width != graphics->getWidth() || height != graphics->getHeight())
        {
            // TODO: Find out why the drawing area doesn't resize without a restart.
            new OkDialog(_("Screen resolution changed"),
                         _("Restart your client for the change to take effect."));
        }

        config.setValue("screenwidth", width);
        config.setValue("screenheight", height);
    }
    else if (event.getId() == "guialpha")
    {
        config.setValue("guialpha", mAlphaSlider->getValue());
    }
    else if (event.getId() == "customcursor")
    {
        config.setValue("customcursor",
                mCustomCursorCheckBox->isSelected() ? true : false);
    }
    else if (event.getId() == "pickupchat")
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
        int val = (int) mSpeechSlider->getValue();
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
    else if (event.getId() == "fpslimitslider")
    {
        mFps = (int) mFpsSlider->getValue();
        mFpsField->setText(toString(mFps));
    }
    else if (event.getId() == "overlaydetailslider")
    {
        int val = (int) mOverlayDetailSlider->getValue();
        setOverlayDetailLabel(val);
        config.setValue("OverlayDetail", val);
    }
    else if (event.getId() == "particledetailslider")
    {
        int val = (int) mParticleDetailSlider->getValue();
        setParticleDetailLabel(val);
        config.setValue("particleeffects", val != -1);
        config.setValue("particleEmitterSkip", 3 - val);

        if (val < 1 && engine)
        {
            beingManager->loadParticleEffects();
            Map* map = engine->getCurrentMap();

            if (map)
                map->initializeParticleEffects(particleEngine);
        }

        if (val > -1)
            Particle::emitterSkip = 4 - val;
    }
    else if (event.getId() == "fpslimitcheckbox")
    {
        if (mFpsCheckBox->isSelected())
            mFps = (int) mFpsSlider->getValue();
        else
            mFps = 0;

        mFpsField->setText(toString(mFps));
        mFpsSlider->setValue(mFps);
        mFpsSlider->setEnabled(mFps > 0);
    }
}

void Setup_Video::setSpeechModeLabel(int value)
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
}

void Setup_Video::setOverlayDetailLabel(int value)
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
}

void Setup_Video::setParticleDetailLabel(int value)
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
}

void Setup_Video::logic()
{
    if (!isVisible())
        return;

    SetupTabContainer::logic();

    mFpsLabel->setCaption(toString(fps) + " FPS");
}
