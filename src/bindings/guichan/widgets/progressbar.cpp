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

#include "../../../core/configlistener.h"
#include "../../../core/configuration.h"
#include "../../../core/resourcemanager.h"

#include "../../../core/image/image.h"

#include "../../../core/utils/dtor.h"

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
    mSmoothColorChange(true),
    mThrobber(false)
{
    mColors.push_back(color);

    setProgress(progress);
    setSize(width, height);

    if (mInstances == 0)
    {
        mAlpha = config.getValue("guialpha", 0.8);

        ResourceManager *resman = ResourceManager::getInstance();
        Image *dBorders = resman->getImage("graphics/gui/vscroll_grey.png");
        int gridx[4] = {0, 4, 7, 11};
        int gridy[4] = {0, 4, 15, 19};
        int i = 0;

        for (int y = 0; y < 3; y++)
        {
            for (int x = 0; x < 3; x++)
            {
                mBorder.grid[i] = dBorders->getSubImage(gridx[x], gridy[y],
                                                        gridx[x + 1] - gridx[x],
                                                        gridy[y + 1] - gridy[y]);
                mBorder.grid[i]->setAlpha(config.getValue("guialpha", 0.8));
                i++;
            }
        }

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
        destroy(mConfigListener);

        for_each(mBorder.grid, mBorder.grid + 9, dtor<Image*>());
    }
}

void ProgressBar::adjustSize()
{
    adjustWidth();
    adjustHeight();
}

void ProgressBar::adjustWidth()
{
    setWidth((getFont()->getWidth(mText) + 8) < mBorder.getMinWidth() ?
             mBorder.getMinHeight() : getFont()->getWidth(mText) + 8);
}

void ProgressBar::adjustHeight()
{
    setHeight((getFont()->getHeight() + 8) < mBorder.getMinHeight() ?
              mBorder.getMinHeight() : getFont()->getHeight() + 8);
}

void ProgressBar::fontChanged()
{
    if (!mText.empty())
        adjustHeight();
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

    if ((mSmoothProgress && mProgressToGo != mProgress) || mThrobber)
    {
        // Smoothly showing the progressbar changes.
        if (mProgressToGo > mProgress)
            mProgress = std::min(1.0f, mProgress + (0.005f * updateTicks));
        if (mProgressToGo < mProgress)
            mProgress = std::max(0.0f, mProgress - (0.005f * updateTicks));

        if (mProgress == 1.0f && mThrobber)
            reset();
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
    mProgress = 0.0f;
    mColor = mColorToGo = mColors[0];
}

void ProgressBar::toggleThrobbing(bool throb)
{
    mThrobber = throb;
    reset();
    mProgressToGo = 1.0f;
}

