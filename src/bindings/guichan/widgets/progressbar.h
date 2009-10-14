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

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <string>
#include <vector>

#include <guichan/widget.hpp>

#include <SDL_types.h>

#include "../guichanfwd.h"

class ImageRect;
class ProgressBarConfigListener;

/**
 * A progress bar.
 *
 * \ingroup GUI
 */
class ProgressBar : public gcn::Widget
{
    public:
        friend class ProgressBarConfigListener;

        /**
         * Constructor, initializes the progress with the given value.
         */
        ProgressBar(float progress = 0.0f, int width = 40, int height = 7,
                    gcn::Color color = gcn::Color(150, 150, 150));

        ~ProgressBar();

        /**
         * Performs progress bar logic (fading colors)
         */
        void logic();

        /**
         * Draws the progress bar.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Adds a color to the progress bar. The progress bar will then
         * transition between the added color and all previously added colors.
         */
        void addColor(const gcn::Color &color);

        /**
         * Adds a color to the progress bar. The progress bar will then
         * transition between the added color and all previously added colors.
         *
         * This is an overloaded version added for convenience.
         */
        void addColor(Uint8, Uint8 green, Uint8 blue);

        /**
         * Sets the current progress.
         */
        void setProgress(float progress);

        /**
         * Returns the current progress.
         */
        float getProgress() const { return mProgress; }

        /**
         * Returns the color of the progress bar.
         */
        const gcn::Color &getColor() const { return mColor; }

        /**
         * Sets the text shown on the progress bar.
         */
        void setText(const std::string &text) { mText = text; }

        /**
         * Returns the text shown on the progress bar.
         */
        const std::string &text() const { return mText; }

        /**
         * Set whether the progress is moved smoothly.
         */
        void setSmoothProgress(bool smoothProgress)
        { mSmoothProgress = smoothProgress; }

        /**
         * Set whether the color changing is made smoothly.
         */
        void setSmoothColorChange(bool smoothColorChange)
        { mSmoothColorChange = smoothColorChange; }

        /**
         * Sets the current shown progress to 0, and resets the progress colors
         * as well.
         */
        void reset();

        /**
         * Sets whether the progress bar should behave like a throbber (as in a
         * progress bar that cycles when progress is unknown) or not.
         */
        void toggleThrobbing(bool throb);

        /**
         * Adjusts the size of the ProgressBar to match the caption text's size.
         */
        virtual void adjustSize();

        virtual void adjustWidth();

        virtual void adjustHeight();

        void fontChanged();
    private:
        float mProgress, mProgressToGo;
        bool mSmoothProgress;
        int mLastUpdate;

        std::vector<gcn::Color> mColors;
        size_t mCurrentColor;

        gcn::Color mColor, mColorToGo;
        bool mSmoothColorChange;
        bool mThrobber;      /**< Whether the progress bar should behave like a
                                  throbber (as in a progress bar that cycles
                                  when progress is unknown) or not. */

        std::string mText;

        static ImageRect mBorder;
        static int mInstances;

    protected:
        static ProgressBarConfigListener *mConfigListener;
        static float mAlpha;
};

#endif
