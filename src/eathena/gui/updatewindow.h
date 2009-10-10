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

#ifndef _UPDATERWINDOW_H
#define _UPDATERWINDOW_H

#include <guichan/actionlistener.hpp>

#include <string>
#include <vector>

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
class UpdaterWindow : public Window, public gcn::ActionListener
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

    private:
        /**
         * Set's progress bar status
         */
        void setProgress(float p);

        /**
         * Set's label above progress
         */
        void setLabel(const std::string &);

        /**
         * Parse the update host and determine the updates directory
         * Then verify that the directory exists (creating if needed).
         */
        void setUpdatesDir(std::string &updateHost);

        /**
         * Enables play button
         */
        void enable();

        /**
         * Parse and display the contents of news.txt.  Assumes that news.txt file
         * has already been downloaded.
         */
        void loadNews();

        /**
         * Reads the file "{Updates Directory}/resources2.txt" and attempts to load
         * each update mentioned in it.  The files need to have been downloaded
         * already - this just passes filenames to ResourceManager.
         */
        void addUpdatesToResman();

        /**
         * Asynchronously downloads a single file (identified by mCurrentFile etc).
         */
        void download();

        /**
         * The thread function that download the files.
         */
        static int downloadThread(void *ptr);

        /**
         * A libcurl callback for progress updates.
         */
        static int updateProgress(void *ptr, double dt, double dn, double ut,
                                  double un);

        /**
         * State machine for the download.  Assuming success, they occur in
         * sequential order down to UPDATE_COMPLETE.
         *
         * UPDATE_ERROR is the error state (including the user pressing "cancel"),
         * which moves on to UPDATE_FINISH next time logic() is called.
         *
         * The "play" button is only active in UPDATE_COMPLETE.
         */
        enum DownloadStatus
        {
            UPDATE_NEWS,       /**< Download news.txt file. */
            UPDATE_LIST,       /**< Download resources2.txt. */
            UPDATE_RESOURCES,  /**< Download .zip files named in resources2.txt. */
            UPDATE_FINISH,     /**< All downloads complete. */
            UPDATE_COMPLETE,   /**< Waiting for user to press "play". */
            UPDATE_ERROR       /**< Error condition. */
        };

        int updateState;

        /** A thread that use libcurl to download updates. */
        SDL_Thread *mThread;

        /** Status of the current download. */
        DownloadStatus mDownloadStatus;

        /** Host where we get the updated files. */
        std::string mUpdateHost;

        /** Place where the updates are stored (absolute path). */
        std::string mUpdatesDir;

        /** The file currently downloading. */
        std::string mCurrentFile;

        /** The new label caption to be set in the logic method. */
        std::string mNewLabelCaption;

        /** The mutex used to guard access to mNewLabelCaption. */
        Mutex mLabelMutex;

        /** The Adler32 checksum of the file currently downloading. */
        unsigned long mCurrentChecksum;

        /** Flag that show if current download is complete. */
        bool mDownloadComplete;

        /** Flag that show if the user has canceled the update. */
        bool mUserCancel;

        /** Buffer to handler human readable error provided by curl. */
        char *mCurlError;

        /** List of files to download. */
        std::vector<std::string> mLines;

        /** Index of the file to be downloaded. */
        unsigned int mLineIndex;

        gcn::Label *mLabel;           /**< Progress bar caption. */
        Button *mStateButton;          /**< Button to start playing/cancel. */
        ProgressBar *mProgressBar;    /**< Update progress bar. */
        BrowserBox *mBrowserBox;      /**< Box to display news. */
        ScrollArea *mScrollArea;      /**< Used to scroll news box. */
};

#endif
