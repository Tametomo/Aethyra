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

#include "radiobutton.h"

#include "../graphics.h"

#include "../../../configlistener.h"
#include "../../../configuration.h"

#include "../../../resources/image.h"
#include "../../../resources/resourcemanager.h"

int RadioButton::instances = 0;
float RadioButton::mAlpha = 1.0;
RadioButtonConfigListener *RadioButton::mConfigListener = NULL;

Image *RadioButton::radioNormal;
Image *RadioButton::radioChecked;
Image *RadioButton::radioDisabled;
Image *RadioButton::radioDisabledChecked;

class RadioButtonConfigListener : public ConfigListener
{
    public:
        RadioButtonConfigListener(RadioButton *button):
            mRadioButton(button)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "guialpha")
            {
                mRadioButton->mAlpha = config.getValue("guialpha", 0.8);

                mRadioButton->radioNormal->setAlpha(mRadioButton->mAlpha);
                mRadioButton->radioChecked->setAlpha(mRadioButton->mAlpha);
                mRadioButton->radioDisabled->setAlpha(mRadioButton->mAlpha);
                mRadioButton->radioDisabledChecked->setAlpha(mRadioButton->mAlpha);
            }
        }
    private:
        RadioButton *mRadioButton;
};

RadioButton::RadioButton(const std::string& caption, const std::string& group,
        bool marked):
    gcn::RadioButton(caption, group, marked)
{
    if (instances == 0)
    {
        ResourceManager *resman = ResourceManager::getInstance();
        mAlpha = config.getValue("guialpha", 0.8);

        radioNormal = resman->getImage("graphics/gui/radioout.png");
        radioChecked = resman->getImage("graphics/gui/radioin.png");
        radioDisabled = resman->getImage("graphics/gui/radioout.png");
        radioDisabledChecked = resman->getImage("graphics/gui/radioin.png");

        radioNormal->setAlpha(mAlpha);
        radioChecked->setAlpha(mAlpha);
        radioDisabled->setAlpha(mAlpha);
        radioDisabledChecked->setAlpha(mAlpha);

        mConfigListener = new RadioButtonConfigListener(this);
        config.addListener("guialpha", mConfigListener);
    }

    instances++;
}

RadioButton::~RadioButton()
{
    instances--;

    if (instances == 0)
    {
        config.removeListener("guialpha", mConfigListener);
        delete mConfigListener;

        radioNormal->decRef();
        radioChecked->decRef();
        radioDisabled->decRef();
        radioDisabledChecked->decRef();
    }
}

void RadioButton::drawBox(gcn::Graphics* graphics)
{
    Image *box = NULL;

    if (isSelected())
    {
        if (isEnabled())
            box = radioChecked;
        else
            box = radioDisabledChecked;
    }
    else if (isEnabled())
        box = radioNormal;
    else
        box = radioDisabled;

    if (box)
        static_cast<Graphics*>(graphics)->drawImage(box, 2, 2);
}

void RadioButton::draw(gcn::Graphics* graphics)
{
    graphics->pushClipArea(gcn::Rectangle(1, 1, getWidth() - 1,
                                          getHeight() - 1));

    drawBox(graphics);

    graphics->popClipArea();

    graphics->setFont(getFont());
    graphics->setColor(getForegroundColor());

    int h = getHeight() + getHeight() / 2;
    graphics->drawText(getCaption(), h - 2, 0);
}
