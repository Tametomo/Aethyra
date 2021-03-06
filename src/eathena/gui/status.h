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

#ifndef STATUS_H
#define STATUS_H

#include "../../bindings/guichan/widgets/window.h"

class LocalPlayer;
class ProgressBar;

/**
 * The player status dialog.
 *
 * \ingroup Interface
 */
class StatusWindow : public Window
{
    public:
        /**
         * Constructor.
         */
        StatusWindow(LocalPlayer *player);

         /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Draw this window
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Updates this dialog with values from PLAYER_INFO *char_info
         */
        void update();

        /**
         * Allows for progress bars to reset on visibility. This is done more
         * for a "bling" reason, not a necessary reason.
         */
        void widgetShown(const gcn::Event& event);

        void fontChanged();

        /**
         * Overridden from the requestFocus inherited from the Window class to
         * do nothing, since there are no focusable widgets in this Window.
         */
        void requestFocus() {};
    private:
        LocalPlayer *mPlayer;

        /**
         * Status Part
         */
        gcn::Label *mLvlLabel, *mJobLvlLabel;
        gcn::Label *mGpLabel;
        gcn::Label *mHpLabel, *mMpLabel, *mXpLabel, *mJobLabel;

        ProgressBar *mHpBar, *mMpBar;
        ProgressBar *mXpBar, *mJobBar;

        /**
         * Derived Statistics captions
         */
        gcn::Label *mStatsAttackLabel, *mStatsDefenseLabel;
        gcn::Label *mStatsMagicAttackLabel, *mStatsMagicDefenseLabel;
        gcn::Label *mStatsAccuracyLabel, *mStatsEvadeLabel;
        gcn::Label *mStatsReflexLabel;

        gcn::Label *mStatsAttackPoints, *mStatsDefensePoints;
        gcn::Label *mStatsMagicAttackPoints, *mStatsMagicDefensePoints;
        gcn::Label *mStatsAccuracyPoints, *mStatsEvadePoints;
        gcn::Label *mStatsReflexPoints;

        /**
         * Stats captions.
         */
        gcn::Label *mStatsTitleLabel;
        gcn::Label *mStatsTotalLabel;
        gcn::Label *mStatsCostLabel;

        gcn::Label *mStatsLabel[6];
        gcn::Label *mPointsLabel[6];
        gcn::Label *mStatsDisplayLabel[6];
        gcn::Label *mRemainingStatsPointsLabel;

        /**
         * Stats buttons.
         */
        gcn::Button *mStatsButton[6];
};

extern StatusWindow *statusWindow;

#endif
