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

#include <guichan/focushandler.hpp>

#include "checkbox.h"

#include "../graphics.h"
#include "../palette.h"
#include "../protectedfocuslistener.h"

#include "../../../core/configlistener.h"
#include "../../../core/configuration.h"
#include "../../../core/resourcemanager.h"

#include "../../../core/image/image.h"

int CheckBox::instances = 0;
float CheckBox::mAlpha = 1.0;
CheckBoxConfigListener *CheckBox::mConfigListener = NULL;

Image *CheckBox::checkBoxNormal;
Image *CheckBox::checkBoxNormalHighlight;
Image *CheckBox::checkBoxChecked;
Image *CheckBox::checkBoxCheckedHighlight;
Image *CheckBox::checkBoxDisabled;
Image *CheckBox::checkBoxDisabledChecked;

class CheckBoxConfigListener : public ConfigListener
{
    public:
        CheckBoxConfigListener(CheckBox *cb):
            mCheckBox(cb)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "guialpha")
            {
                mCheckBox->mAlpha = config.getValue("guialpha", 0.8);

                mCheckBox->checkBoxNormal->setAlpha(mCheckBox->mAlpha);
                mCheckBox->checkBoxNormalHighlight->setAlpha(mCheckBox->mAlpha);
                mCheckBox->checkBoxChecked->setAlpha(mCheckBox->mAlpha);
                mCheckBox->checkBoxCheckedHighlight->setAlpha(mCheckBox->mAlpha);
                mCheckBox->checkBoxDisabled->setAlpha(mCheckBox->mAlpha);
                mCheckBox->checkBoxDisabledChecked->setAlpha(mCheckBox->mAlpha);
            }
        }
    private:
        CheckBox *mCheckBox;
};

CheckBox::CheckBox(const std::string& caption, bool selected):
    gcn::CheckBox(caption, selected)
{
    if (instances == 0)
    {
        ResourceManager *resman = ResourceManager::getInstance();
        Image *checkBox = resman->getImage("graphics/gui/checkbox.png");
        checkBoxNormal = checkBox->getSubImage(0, 0, 9, 10);
        checkBoxChecked = checkBox->getSubImage(9, 0, 9, 10);
        checkBoxDisabled = checkBox->getSubImage(18, 0, 9, 10);
        checkBoxDisabledChecked = checkBox->getSubImage(27, 0, 9, 10);
        Image *highlight = resman->getImage("graphics/gui/checkboxhi.png");
        checkBoxNormalHighlight = highlight->getSubImage(0, 0, 9, 10);
        checkBoxCheckedHighlight = highlight->getSubImage(9, 0, 9, 10);

        mAlpha = config.getValue("guialpha", 0.8);

        checkBoxNormal->setAlpha(mAlpha);
        checkBoxNormalHighlight->setAlpha(mAlpha);
        checkBoxChecked->setAlpha(mAlpha);
        checkBoxCheckedHighlight->setAlpha(mAlpha);
        checkBoxDisabled->setAlpha(mAlpha);
        checkBoxDisabledChecked->setAlpha(mAlpha);

        checkBox->decRef();
        highlight->decRef();

        mConfigListener = new CheckBoxConfigListener(this);
        config.addListener("guialpha", mConfigListener);
    }

    instances++;

    mProtFocusListener = new ProtectedFocusListener();

    addFocusListener(mProtFocusListener);

    mProtFocusListener->blockKey(SDLK_SPACE);
    mProtFocusListener->blockKey(SDLK_RETURN);
}

CheckBox::~CheckBox()
{
    instances--;

    if (instances == 0)
    {
        config.removeListener("guialpha", mConfigListener);
        delete mConfigListener;

        delete checkBoxNormal;
        delete checkBoxNormalHighlight;
        delete checkBoxChecked;
        delete checkBoxCheckedHighlight;
        delete checkBoxDisabled;
        delete checkBoxDisabledChecked;
    }

    if (mFocusHandler && mFocusHandler->isFocused(this))
        mFocusHandler->focusNone();

    removeFocusListener(mProtFocusListener);
    delete mProtFocusListener;
}

void CheckBox::draw(gcn::Graphics* graphics)
{
    drawBox(graphics);

    graphics->setFont(getFont());
    graphics->setColor(guiPalette->getColor(Palette::TEXT));

    const int h = getHeight() + getHeight() / 2;

    graphics->drawText(getCaption(), h - 2, 0);
}

void CheckBox::drawBox(gcn::Graphics* graphics)
{
    Image *box;

    if (isSelected())
    {
        if (isEnabled() && isFocused())
            box = checkBoxCheckedHighlight;
        else if (isEnabled() && !isFocused())
            box = checkBoxChecked;
        else
            box = checkBoxDisabledChecked;
    }
    else if (isEnabled() && isFocused())
        box = checkBoxNormalHighlight;
    else if (isEnabled() && !isFocused())
        box = checkBoxNormal;
    else
        box = checkBoxDisabled;

    static_cast<Graphics*>(graphics)->drawImage(box, 2, 2);
}
