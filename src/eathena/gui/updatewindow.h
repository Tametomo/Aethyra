/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *  Copyright (C) 2009  The Aethyra Development Team
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

#ifndef _UPDATERWINDOW_H
#define _UPDATERWINDOW_H

#include <guichan/actionlistener.hpp>

#include <string>
#include <vector>

#include "../../bindings/update/downloadupdates.h"

#include "../../bindings/guichan/widgets/window.h"

#include "../../core/utils/mutex.h"

class BrowserBox;
class Button;
class ProgressBar;
class ScrollArea;

struct SDL_Thread;

/**
 * Update progress window GUI, for downloading from the update server.
 *
 * Main user interface while StateManager::getState() == UPDATE_STATE.
 * When the player presses "play", sets the game state to LOADDATA_STATE.
 *
 * Currently also the logic for the process of updating, and the controller for
 * the download.
 *
 * Currently also responsible for adding all the update files to
 * ResourceManager's search path.
 *
 * \ingroup GUI
 */
class UpdaterWindow : public Window, public gcn::ActionListener,
        public UpdaterListener
{
    public:
        /**
         * Constructor.
         *
         * @param updateHost Host where to get the updated files.
         */
        UpdaterWindow(const std::string &updateHost);

        /**
         * Destructor
         */
        ~UpdaterWindow();

        void action(const gcn::ActionEvent &event);

        void logic();

        /**
         * Selects the cancel button when no updates are loaded, or the play
         * button when the updates are already loaded.
         */
        void requestFocus();

        void fontChanged();

        //from UpdaterListener
        void downloadTextUpdate(const std::vector<std::string>& news);
        void downloadProgress(float totalProgress,
                const std::string& currentFile, float fileProgress);
        void downloadComplete();


    private:
        /**
         * Enables play button
         */
        void enable();

        /**
         * The new label caption to be set in the logic method.
         * This is for the downloadProgress callback, which happens in
         * a different thread to the UI thread.
         */
        std::string mNewLabelCaption;
        /**
         * New value for the progress bar.
         *
         * This is a unit with mNewLabelCaption - they both update
         * at the same time.
         */
        float mNewProgress;

        /**
         * News to display to the user.
         * Not synchronous to mNewLabelCaption, but is affected by the same two
         * threads, and uses the same mutex.
         */
        std::vector<std::string> mNewNews;

        /** The mutex used to guard access to mNewLabelCaption etc. */
        Mutex mLabelMutex;

        gcn::Label *mLabel;           /**< Progress bar caption. */
        Button *mStateButton;          /**< Button to start playing/cancel. */
        ProgressBar *mProgressBar;    /**< Update progress bar. */
        BrowserBox *mBrowserBox;      /**< Box to display news. */
        ScrollArea *mScrollArea;      /**< Used to scroll news box. */

        /**
         * The model that this UI is the view for.
         */
        DownloadUpdates* mLogic;
};

#endif
