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

#include "slider.h"

#include "../graphics.h"
#include "../protectedfocuslistener.h"

#include "../sdl/sdlinput.h"

#include "../../../configlistener.h"
#include "../../../configuration.h"

#include "../../../resources/image.h"
#include "../../../resources/resourcemanager.h"

int Slider::mInstances = 0;
Image *Slider::hStart, *Slider::hMid, *Slider::hEnd, *Slider::hGrip;
Image *Slider::vStart, *Slider::vMid, *Slider::vEnd, *Slider::vGrip;
Image *Slider::hStartHi, *Slider::hMidHi, *Slider::hEndHi, *Slider::hGripHi;
Image *Slider::vStartHi, *Slider::vMidHi, *Slider::vEndHi, *Slider::vGripHi;

float Slider::mAlpha = 1.0;
SliderConfigListener *Slider::mConfigListener = NULL;

class SliderConfigListener : public ConfigListener
{
    public:
        SliderConfigListener(Slider *slider):
            mSlider(slider)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "guialpha")
            {
                mSlider->mAlpha = config.getValue("guialpha", 0.8);

                mSlider->hStart->setAlpha(mSlider->mAlpha);
                mSlider->hStartHi->setAlpha(mSlider->mAlpha);
                mSlider->hMid->setAlpha(mSlider->mAlpha);
                mSlider->hMidHi->setAlpha(mSlider->mAlpha);
                mSlider->hEnd->setAlpha(mSlider->mAlpha);
                mSlider->hEndHi->setAlpha(mSlider->mAlpha);
                mSlider->hGrip->setAlpha(mSlider->mAlpha);
                mSlider->hGripHi->setAlpha(mSlider->mAlpha);

                mSlider->vStart->setAlpha(mSlider->mAlpha);
                mSlider->vStartHi->setAlpha(mSlider->mAlpha);
                mSlider->vMid->setAlpha(mSlider->mAlpha);
                mSlider->vMidHi->setAlpha(mSlider->mAlpha);
                mSlider->vEnd->setAlpha(mSlider->mAlpha);
                mSlider->vEndHi->setAlpha(mSlider->mAlpha);
                mSlider->vGrip->setAlpha(mSlider->mAlpha);
                mSlider->vGripHi->setAlpha(mSlider->mAlpha);
            }
        }
    private:
        Slider *mSlider;
};

Slider::Slider(double scaleEnd):
    gcn::Slider(scaleEnd)
{
    init();
}

Slider::Slider(double scaleStart, double scaleEnd):
    gcn::Slider(scaleStart, scaleEnd)
{
    init();
}

Slider::~Slider()
{
    mInstances--;

    if (mInstances == 0)
    {
        config.removeListener("guialpha", mConfigListener);
        delete mConfigListener;

        delete hStart;
        delete hStartHi;
        delete hMid;
        delete hMidHi;
        delete hEnd;
        delete hEndHi;
        delete hGrip;
        delete hGripHi;
        delete vStart;
        delete vStartHi;
        delete vMid;
        delete vMidHi;
        delete vEnd;
        delete vEndHi;
        delete vGrip;
        delete vGripHi;
    }

    if (mFocusHandler && mFocusHandler->isFocused(this))
        mFocusHandler->focusNone();

    removeFocusListener(mProtFocusListener);
    delete mProtFocusListener;
}

void Slider::init()
{
    int x, y, w, h,o1,o2;
    setFrameSize(0);

    // Load resources
    if (mInstances == 0)
    {
        mAlpha = config.getValue("guialpha", 0.8);

        ResourceManager *resman = ResourceManager::getInstance();
        Image *slider = resman->getImage("graphics/gui/slider.png");
        Image *highlight = resman->getImage("graphics/gui/sliderhi.png");

        x = 0; y = 0;
        w = 15; h = 6;
        o1 = 4; o2 = 11;
        hStart = slider->getSubImage(x, y, o1 - x, h);
        hStartHi = highlight->getSubImage(x, y, o1 - x, h);
        hMid = slider->getSubImage(o1, y, o2 - o1, h);
        hMidHi = highlight->getSubImage(o1, y, o2 - o1, h);
        hEnd = slider->getSubImage(o2, y, w - o2 + x, h);
        hEndHi = highlight->getSubImage(o2, y, w - o2 + x, h);

        x = 6; y = 8;
        w = 9; h = 10;
        hGrip = slider->getSubImage(x, y, w, h);
        hGripHi = highlight->getSubImage(x, y, w, h);

        x = 0; y = 6;
        w = 6; h = 21;
        o1 = 10; o2 = 18;
        vStart = slider->getSubImage(x, y, w, o1 - y);
        vStartHi = highlight->getSubImage(x, y, w, o1 - y);
        vMid = slider->getSubImage(x, o1, w, o2 - o1);
        vMidHi = highlight->getSubImage(x, o1, w, o2 - o1);
        vEnd = slider->getSubImage(x, o2, w, h - o2 + y);
        vEndHi = highlight->getSubImage(x, o2, w, h - o2 + y);

        x = 6; y = 8;
        w = 9; h = 10;
        vGrip = slider->getSubImage(x, y, w, h);
        vGripHi = highlight->getSubImage(x, y, w, h);

        slider->decRef();
        highlight->decRef();

        hStart->setAlpha(mAlpha);
        hStartHi->setAlpha(mAlpha);
        hMid->setAlpha(mAlpha);
        hMidHi->setAlpha(mAlpha);
        hEnd->setAlpha(mAlpha);
        hEndHi->setAlpha(mAlpha);
        hGrip->setAlpha(mAlpha);
        hGripHi->setAlpha(mAlpha);

        vStart->setAlpha(mAlpha);
        vStartHi->setAlpha(mAlpha);
        vMid->setAlpha(mAlpha);
        vMidHi->setAlpha(mAlpha);
        vEnd->setAlpha(mAlpha);
        vEndHi->setAlpha(mAlpha);
        vGrip->setAlpha(mAlpha);
        vGripHi->setAlpha(mAlpha);

        mConfigListener = new SliderConfigListener(this);
        config.addListener("guialpha", mConfigListener);
    }

    mProtFocusListener = new ProtectedFocusListener();

    addFocusListener(mProtFocusListener);

    mProtFocusListener->blockKey(SDLK_LEFT);
    mProtFocusListener->blockKey(SDLK_RIGHT);
    mProtFocusListener->blockKey(SDLK_UP);
    mProtFocusListener->blockKey(SDLK_DOWN);

    mInstances++;

    setMarkerLength(hGrip->getWidth());
}

void Slider::draw(gcn::Graphics *graphics)
{
    int w = getWidth();
    int h = getHeight();
    int x = 0;
    int y = (h - hStart->getHeight()) / 2;

    if (isFocused())
        static_cast<Graphics*>(graphics)->drawImage(hStartHi, x, y);
    else
        static_cast<Graphics*>(graphics)->drawImage(hStart, x, y);

    w -= hStart->getWidth() + hEnd->getWidth();
    x += hStart->getWidth();

    if (isFocused())
        static_cast<Graphics*>(graphics)->drawImagePattern(hMidHi, x, y, w,
                                                           hMidHi->getHeight());
    else
        static_cast<Graphics*>(graphics)->drawImagePattern(hMid, x, y, w,
                                                           hMid->getHeight());

    x += w;

    if (isFocused())
        static_cast<Graphics*>(graphics)->drawImage(hEndHi, x, y);
    else
        static_cast<Graphics*>(graphics)->drawImage(hEnd, x, y);

    drawMarker(graphics);
}

void Slider::drawMarker(gcn::Graphics *graphics)
{
    if (isFocused())
        static_cast<Graphics*>(graphics)->drawImage(hGripHi, getMarkerPosition(),
                                                   (getHeight() -
                                                    hGripHi->getHeight()) / 2);
    else
        static_cast<Graphics*>(graphics)->drawImage(hGrip, getMarkerPosition(),
                                                   (getHeight() -
                                                    hGrip->getHeight()) / 2);
}

void Slider::keyPressed(gcn::KeyEvent &keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (getOrientation() == HORIZONTAL)
    {
        if (key.getValue() == Key::RIGHT)
        {
            setValue(getValue() + getStepLength());
            distributeActionEvent();
            keyEvent.consume();
        }
        else if (key.getValue() == Key::LEFT)
        {
            setValue(getValue() - getStepLength());
            distributeActionEvent();
            keyEvent.consume();
        }            
    }
    else
    {
        if (key.getValue() == Key::UP)
        {
            setValue(getValue() + getStepLength());
            distributeActionEvent();
            keyEvent.consume();
        }
        else if (key.getValue() == Key::DOWN)
        {
            setValue(getValue() - getStepLength());
            distributeActionEvent();
            keyEvent.consume();
        }
    }
}

