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

#include "setup_audio.h"

#include "../okdialog.h"

#include "../../layouthelper.h"

#include "../../widgets/checkbox.h"
#include "../../widgets/label.h"
#include "../../widgets/slider.h"

#include "../../../sdl/sound.h"

#include "../../../../core/configuration.h"
#include "../../../../core/log.h"

#include "../../../../core/map/map.h"

#include "../../../../core/utils/gettext.h"

Setup_Audio::Setup_Audio():
    mMusicVolume((int) config.getValue("musicVolume", defaultMusicVolume)),
    mSfxVolume((int) config.getValue("sfxVolume", defaultSfxVolume)),
    mSoundEnabled(config.getValue("sound", 1)),
    mSoundCheckBox(new CheckBox(_("Sound"), mSoundEnabled)),
    mSfxSlider(new Slider(0, sound.getMaxVolume())),
    mMusicSlider(new Slider(0, sound.getMaxVolume()))
{
    setName(_("Audio"));

    if (config.getValue("sound", 1))
    {
        sound.setSfxVolume((int) config.getValue("sfxVolume", defaultSfxVolume));
        sound.setMusicVolume((int) config.getValue("musicVolume",
                                                   defaultMusicVolume));
    }

    mSfxLabel = new Label(_("Sfx volume"));
    mMusicLabel = new Label(_("Music volume"));

    mSfxSlider->setActionEventId("sfx");
    mMusicSlider->setActionEventId("music");

    mSfxSlider->addActionListener(this);
    mMusicSlider->addActionListener(this);

    mSoundCheckBox->setPosition(10, 10);

    mSfxSlider->setValue(mSfxVolume);
    mMusicSlider->setValue(mMusicVolume);

    mSfxSlider->setWidth(90);
    mMusicSlider->setWidth(90);

    fontChanged();

    setDimension(gcn::Rectangle(0, 0, 325, 280));
}

void Setup_Audio::fontChanged()
{
    SetupTabContainer::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, mSoundCheckBox);
    place(0, 1, mSfxSlider);
    place(1, 1, mSfxLabel);
    place(0, 2, mMusicSlider);
    place(1, 2, mMusicLabel);

    h.reflowLayout(325, 280);

    restoreFocus();
}

void Setup_Audio::apply()
{
    if (mSoundCheckBox->isSelected())
    {
        config.setValue("sound", 1);
        try
        {
            sound.init();
        }
        catch (const char *err)
        {
            new OkDialog("Sound Engine", err);
            logger->log("Warning: %s", err);
        }
    }
    else
    {
        config.setValue("sound", 0);
        sound.close();
    }

    mSoundEnabled = config.getValue("sound", 1);
    mSfxVolume = (int) config.getValue("sfxVolume", defaultSfxVolume);
    mMusicVolume = (int) config.getValue("musicVolume", defaultMusicVolume);
}

void Setup_Audio::cancel()
{
    mSoundCheckBox->setSelected(mSoundEnabled);

    sound.setSfxVolume(mSfxVolume);
    mSfxSlider->setValue(mSfxVolume);

    sound.setMusicVolume(mMusicVolume);
    mMusicSlider->setValue(mMusicVolume);

    config.setValue("sound", mSoundEnabled ? 1 : 0);
    config.setValue("sfxVolume", mSfxVolume);
    config.setValue("musicVolume", mMusicVolume);
}

void Setup_Audio::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "sfx")
    {
        config.setValue("sfxVolume", (int) mSfxSlider->getValue());
        sound.setSfxVolume((int) mSfxSlider->getValue());
    }
    else if (event.getId() == "music")
    {
        config.setValue("musicVolume", (int) mMusicSlider->getValue());
        sound.setMusicVolume((int) mMusicSlider->getValue());
    }
}
