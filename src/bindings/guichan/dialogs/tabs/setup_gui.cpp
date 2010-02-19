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

#include <string>

#include "setup_gui.h"

#include "../okdialog.h"

#include "../../graphics.h"
#include "../../gui.h"
#include "../../layouthelper.h"

#include "../../models/modelistmodel.h"

#include "../../widgets/checkbox.h"
#include "../../widgets/desktop.h"
#include "../../widgets/label.h"
#include "../../widgets/listbox.h"
#include "../../widgets/scrollarea.h"
#include "../../widgets/slider.h"
#include "../../widgets/textfield.h"

#include "../../../../core/configuration.h"
#include "../../../../core/log.h"

#include "../../../../core/image/particle/particle.h"

#include "../../../../core/map/sprite/localplayer.h"

#include "../../../../core/utils/gettext.h"
#include "../../../../core/utils/stringutils.h"

#ifdef HAVE_CONFIG_H
#include "../../../../../config.h"
#endif

Setup_Gui::Setup_Gui():
    mFullScreenEnabled(config.getValue("screen", false)),
    mOpenGLEnabled(config.getValue("opengl", USE_OPENGL)),
    mCustomCursorEnabled(config.getValue("customcursor", true)),
    mOpacity(config.getValue("guialpha", 0.8)),
    mMouseOpacity(config.getValue("mousealpha", 0.7)),
    mFps(config.getValue("fpslimit", 0)),
    mScreenWidth(graphics->getWidth()),
    mScreenHeight(graphics->getHeight()),
    mModeListModel(new ModeListModel()),
    mModeList(new ListBox(mModeListModel, "videomode", this)),
    mFsCheckBox(new CheckBox(_("Full screen"), mFullScreenEnabled)),
    mOpenGLCheckBox(new CheckBox(_("OpenGL"), mOpenGLEnabled)),
    mCustomCursorCheckBox(new CheckBox(_("Custom cursor"), mCustomCursorEnabled)),
    mAlphaSlider(new Slider(0.2, 1.0)),
    mMouseAlphaSlider(new Slider(0.2, 1.0)),
    mFpsCheckBox(new CheckBox(_("FPS Limit:"))),
    mFpsSlider(new Slider(5, 200)),
    mFontSize(config.getValue("fontSize", 11)),
    mFontSizeSlider(new Slider(8, 14)),
    mFontSizeLabel(new Label())
{
    setName(_("GUI"));

    mScrollArea = new ScrollArea(mModeList);
    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mScrollArea->setWidth(90);

    mAlphaLabel = new Label(_("Gui opacity"));
    mMouseAlphaLabel = new Label(_("Mouse opacity"));
    mFpsLabel = new Label();
    mFontLabel = new Label(_("Font size"));

    mModeList->setEnabled(true);

#ifndef USE_OPENGL
    mOpenGLCheckBox->setEnabled(false);
#endif

    mAlphaSlider->setValue(mOpacity);
    mMouseAlphaSlider->setValue(mMouseOpacity);

    mFpsLabel->setCaption(toString(mFps));
    mFpsLabel->adjustSize();
    mFpsSlider->setValue(mFps);
    mFpsSlider->setEnabled(mFps > 0);
    mFpsCheckBox->setSelected(mFps > 0);

    mCustomCursorCheckBox->setActionEventId("customcursor");
    mAlphaSlider->setActionEventId("guialpha");
    mMouseAlphaSlider->setActionEventId("mousealpha");
    mFpsCheckBox->setActionEventId("fpslimitcheckbox");
    mFpsSlider->setActionEventId("fpslimitslider");
    mFontSizeSlider->setActionEventId("fontsizeslider");

    mCustomCursorCheckBox->addActionListener(this);
    mAlphaSlider->addActionListener(this);
    mMouseAlphaSlider->addActionListener(this);
    mFpsCheckBox->addActionListener(this);
    mFpsSlider->addActionListener(this);
    mFontSizeSlider->addActionListener(this);

    mFontSize = config.getValue("fontSize", 11);
    mFontSizeLabel->setCaption(strprintf(_("%d Point"), mFontSize));
    mFontSizeSlider->setValue(mFontSize);

    mFpsSlider->setStepLength(1.0);
    mFontSizeSlider->setStepLength(1.0);

    fontChanged();

    setDimension(gcn::Rectangle(0, 0, 325, 200));
}

Setup_Gui::~Setup_Gui()
{
    delete mModeListModel;
}

void Setup_Gui::fontChanged()
{
    SetupTabContainer::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, mScrollArea, 1, 6).setPadding(2);
    place(1, 0, mFsCheckBox, 2);
    place(3, 0, mOpenGLCheckBox, 1);
    place(1, 1, mCustomCursorCheckBox, 3);

    place(0, 6, mAlphaSlider);
    place(0, 7, mMouseAlphaSlider);
    place(0, 8, mFpsSlider);
    place(0, 9, mFontSizeSlider);

    place(1, 6, mAlphaLabel, 3).setPadding(2);
    place(1, 7, mMouseAlphaLabel, 3).setPadding(2);
    place(1, 8, mFpsCheckBox).setPadding(3);
    place(1, 9, mFontLabel);

    place(2, 8, mFpsLabel).setPadding(1);
    place(2, 9, mFontSizeLabel, 3).setPadding(2);

    h.reflowLayout(325, 200);

    restoreFocus();
}

void Setup_Gui::apply()
{
    const std::string mode = mModeListModel->getElementAt(mModeList->getSelected());

    if (mode.find("x") != std::string::npos)
    {
        const int width = atoi(mode.substr(0, mode.find("x")).c_str());
        const int height = atoi(mode.substr(mode.find("x") + 1).c_str());

        gui->resize(width, height);
    }

    // Full screen changes
    bool fullscreen = mFsCheckBox->isSelected();
    if (fullscreen != (config.getValue("screen", false) == 1))
    {
        /* Currently, switching to fullscreen mode in windows with SDL or OpenGL
         * modes will lock up the screen and cause graphical flickers.
         *
         *  TODO: Remove this if this issue ever gets fixed locally, or if it's
         *        found to be a bug upstream in SDL and gets fixed later.
         *
         * See http://libsdl.org/cgi/docwiki.cgi/SDL_SetVideoMode
         */

#ifdef WIN32
        new OkDialog(_("Switching to full screen"),
                     _("Restart needed for changes to take effect."));
#else
        if (!graphics->setFullscreen(fullscreen))
        {
            fullscreen = !fullscreen;
            if (!graphics->setFullscreen(fullscreen))
            {
                std::stringstream error;
                error << strprintf(_("Failed to switch to %s mode and "
                                     "restoration of old mode also "
                                     "failed!"), (fullscreen ? _("windowed") :
                                   _("fullscreen"))) << std::endl;
                logger->error(error.str());
            }
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

    if ((int) mFontSizeSlider->getValue() != mFontSize)
    {
        const int val = (int) mFontSizeSlider->getValue();

        gui->changeFontSize(val);

        config.setValue("fontSize", val);
    }

    mFps = (mFpsCheckBox->isSelected()) ? (int) mFpsSlider->getValue() : 0;

    mFpsSlider->setEnabled(mFps > 0);

    // FPS change
    config.setValue("fpslimit", mFps);

    // We sync old and new values at apply time
    mFullScreenEnabled = config.getValue("screen", false);
    mOpenGLEnabled = config.getValue("opengl", USE_OPENGL);
    mCustomCursorEnabled = config.getValue("customcursor", true);
    mOpacity = config.getValue("guialpha", 0.8);
    mMouseOpacity = config.getValue("mousealpha", 0.7);
    mScreenWidth = config.getValue("screenwidth", 800);
    mScreenHeight = config.getValue("screenheight", 600);
    mFontSize = config.getValue("fontSize", 11);
}

void Setup_Gui::cancel()
{
    gui->resize(mScreenWidth, mScreenHeight);

    mFpsCheckBox->setSelected(mFps > 0);
    mFsCheckBox->setSelected(mFullScreenEnabled);
    mOpenGLCheckBox->setSelected(mOpenGLEnabled);
    mCustomCursorCheckBox->setSelected(mCustomCursorEnabled);
    mFpsSlider->setValue(mFps);
    mAlphaSlider->setValue(mOpacity);
    mMouseAlphaSlider->setValue(mMouseOpacity);

    std::string text;

    text = (mFpsCheckBox->isSelected()) ? toString(mFps) : "";

    mFpsSlider->setEnabled(mFps > 0);
    mFpsLabel->setCaption(text);

    int val = (int) mFontSizeSlider->getValue();
    mFontSizeLabel->setCaption(strprintf(_("%d Point"), val));
    mFontSizeLabel->adjustSize();

    config.setValue("screen", mFullScreenEnabled ? true : false);
    config.setValue("opengl", mOpenGLEnabled ? true : false);
    config.setValue("customcursor", mCustomCursorEnabled ? true : false);
    config.setValue("guialpha", mOpacity);
    config.setValue("mousealpha", mMouseOpacity);
}

void Setup_Gui::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "videomode")
    {
        const std::string mode = mModeListModel->getElementAt(mModeList->getSelected());

        if (mode.find("x") == std::string::npos)
            return;

        const int width = atoi(mode.substr(0, mode.find("x")).c_str());
        const int height = atoi(mode.substr(mode.find("x") + 1).c_str());

        gui->resize(width, height);
    }
    else if (event.getId() == "guialpha")
    {
        config.setValue("guialpha", mAlphaSlider->getValue());
    }
    else if (event.getId() == "mousealpha")
    {
        config.setValue("mousealpha", mMouseAlphaSlider->getValue());
    }
    else if (event.getId() == "customcursor")
    {
        config.setValue("customcursor",
                mCustomCursorCheckBox->isSelected() ? true : false);
    }
    else if (event.getId() == "fpslimitslider")
    {
        const int fps = (int) mFpsSlider->getValue();
        std::string text;

        text = (mFpsCheckBox->isSelected()) ? toString(fps) : "";

        mFpsLabel->setCaption(text);
        mFpsLabel->adjustSize();
    }
    else if (event.getId() == "fontsizeslider")
    {
        const int val = (int) mFontSizeSlider->getValue();
        mFontSizeLabel->setCaption(strprintf(_("%d Point"), val));
        mFontSizeLabel->adjustSize();
    }
    else if (event.getId() == "fpslimitcheckbox")
    {
        std::string text;
        int fps = (int) mFpsSlider->getValue();

        if (mFpsCheckBox->isSelected())
        {
            text = toString(fps);
        }
        else
        {
            fps = 0;
            text = "";
        }

        mFpsSlider->setEnabled(fps > 0);
        mFpsLabel->setCaption(text);
        mFpsLabel->adjustSize();
    }
}

