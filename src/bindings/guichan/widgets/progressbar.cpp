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

#include <guichan/color.hpp>
#include <guichan/font.hpp>

#include "progressbar.h"

#include "../graphics.h"
#include "../gui.h"
#include "../palette.h"
#include "../textrenderer.h"

#include "../../../configlistener.h"
#include "../../../configuration.h"

#include "../../../resources/image.h"
#include "../../../resources/resourcemanager.h"

ImageRect ProgressBar::mBorder;

int ProgressBar::mInstances = 0;
float ProgressBar::mAlpha = 1.0;
ProgressBarConfigListener *ProgressBar::mConfigListener = NULL;

class ProgressBarConfigListener : public ConfigListener
{
    public:
        ProgressBarConfigListener(ProgressBar *pb):
            mProgressBar(pb)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "guialpha")
            {
                mProgressBar->mAlpha = config.getValue("guialpha", 0.8);

                for (int a = 0; a < 9; a++)
                {
                    mProgressBar->mBorder.grid[a]->setAlpha(mProgressBar->mAlpha);
                }
            }
        }
    private:
        ProgressBar *mProgressBar;
};

ProgressBar::ProgressBar(float progress, int width, int height,
                         gcn::Color color):
    gcn::Widget(),
    mProgress(0.0f),
    mProgressToGo(0.0f),
    mSmoothProgress(true),
    mCurrentColor(0),
    mColor(color),
    mColorToGo(color),
    mSmoothColorChange(true)
{
    mColors.push_back(color);

    setProgress(progress);
    setSize(width, height);

    if (mInstances == 0)
    {
        mAlpha = config.getValue("guialpha", 0.8);

        ResourceManager *resman = ResourceManager::getInstance();
        Image *dBorders = resman->getImage("graphics/gui/vscroll_grey.png");
        Image *dInsides = resman->getImage("graphics/gui/buttonhi.png");

        mBorder.grid[0] = dBorders->getSubImage(0, 0, 4, 4);
        mBorder.grid[1] = dBorders->getSubImage(4, 0, 3, 4);
        mBorder.grid[2] = dBorders->getSubImage(7, 0, 4, 4);
        mBorder.grid[3] = dBorders->getSubImage(0, 4, 4, 10);
        mBorder.grid[4] = dInsides->getSubImage(2, 2, 22, 22);
        mBorder.grid[5] = dBorders->getSubImage(7, 4, 4, 10);
        mBorder.grid[6] = dBorders->getSubImage(0, 15, 4, 4);
        mBorder.grid[7] = dBorders->getSubImage(4, 15, 3, 4);
        mBorder.grid[8] = dBorders->getSubImage(7, 15, 4, 4);

        for (int i = 0; i < 9; i++)
        {
            mBorder.grid[i]->setAlpha(mAlpha);
        }

        dInsides->decRef();
        dBorders->decRef();

        mConfigListener = new ProgressBarConfigListener(this);
        config.addListener("guialpha", mConfigListener);
    }

    mLastUpdate = tick_time;

    mInstances++;
}

ProgressBar::~ProgressBar()
{
    mInstances--;

    if (mInstances == 0)
    {
        config.removeListener("guialpha", mConfigListener);
        delete mConfigListener;

        for (int i = 0; i < 9; i++)
            delete mBorder.grid[i];
    }
}

void ProgressBar::logic()
{
    if (!isVisible() || !getParent()->isVisible())
    {
        mLastUpdate = tick_time;
        return;
    }

    // Make the gradients and smooth progress appear consistent regardless of
    // the framerate.
    const int updateTicks = get_elapsed_time(mLastUpdate) / 10;

    if (updateTicks)
        mLastUpdate = tick_time;
    else
        return;

    const size_t index = (size_t) (mProgress * mColors.size());

    if (mCurrentColor != index && index < mColors.size())
    {
        mCurrentColor = index;
        mColorToGo = mColors[index];
    }

    if (mSmoothColorChange && mColorToGo != mColor)
    {
        // Smoothly changing the color for a nicer effect.
        if (mColorToGo.r > mColor.r)
            mColor.r = std::min(mColorToGo.r, mColor.r + updateTicks);
        if (mColorToGo.g > mColor.g)
            mColor.g = std::min(mColorToGo.g, mColor.g + updateTicks);
        if (mColorToGo.b > mColor.b)
            mColor.b = std::min(mColorToGo.b, mColor.b + updateTicks);
        if (mColorToGo.r < mColor.r)
            mColor.r = std::max(mColorToGo.r, mColor.r - updateTicks);
        if (mColorToGo.g < mColor.g)
            mColor.g = std::max(mColorToGo.g, mColor.g - updateTicks);
        if (mColorToGo.b < mColor.b)
            mColor.b = std::max(mColorToGo.b, mColor.b - updateTicks);
    }
    else
    {
        mColor = mColorToGo;
    }

    if (mSmoothProgress && mProgressToGo != mProgress)
    {
        // Smoothly showing the progressbar changes.
        if (mProgressToGo > mProgress)
            mProgress = std::min(1.0f, mProgress + (0.005f * updateTicks));
        if (mProgressToGo < mProgress)
            mProgress = std::max(0.0f, mProgress - (0.005f * updateTicks));
    }
    else
    {
        mProgress = mProgressToGo;
    }
}

void ProgressBar::draw(gcn::Graphics *graphics)
{
    static_cast<Graphics*>(graphics)->
        drawImageRect(0, 0, getWidth(), getHeight(), mBorder);

    mColor.a = (int) (mAlpha * 255);

    // The bar
    if (mProgress > 0)
    {
        graphics->setColor(mColor);
        graphics->fillRectangle(gcn::Rectangle(4, 4,
                               (int) (mProgress * (getWidth() - 8)),
                                getHeight() - 8));
    }

    // The label
    if (!mText.empty())
    {
        gcn::Font* boldFont = gui->getBoldFont();

        const int textX = getWidth() / 2;
        const int textY = (getHeight() - boldFont->getHeight()) / 2;

        TextRenderer::renderText(graphics, mText, textX, textY,
                                 gcn::Graphics::CENTER,
                                 guiPalette->getColor(Palette::PROGRESS_BAR,
                                 mColor.a), boldFont, true, false, mColor.a);
    }
}

void ProgressBar::setProgress(float progress)
{
    if (progress < 0.0f)
        mProgressToGo = 0.0;
    else if (progress > 1.0f)
        mProgressToGo = 1.0;
    else
        mProgressToGo = progress;
}

void ProgressBar::addColor(const gcn::Color& color)
{
    mColors.push_back(color);
}

void ProgressBar::addColor(Uint8 red, Uint8 green, Uint8 blue)
{
    mColors.push_back(gcn::Color(red, green, blue));
}

void ProgressBar::reset()
{
    mProgress = 0;
    mColor = mColorToGo = mColors[0];
}

