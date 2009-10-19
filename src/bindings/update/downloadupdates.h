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

#ifndef _DOWNLOADLOGIC_H
#define _DOWNLOADLOGIC_H

#include <string>
#include <vector>

#include "../../bindings/curl/downloadwrapper.h"

#include "../../core/utils/mutex.h"

/**
 * Callbacks to the GUI.
 * Likely to be in a different thread than the GUIChan one.
 */
class UpdaterListener
{
public:
    virtual void downloadTextUpdate(const std::vector<std::string>& news) = 0;
    virtual void downloadProgress(float totalProgress, const std::string& currentFile, float fileProgress) = 0;
    virtual void downloadComplete() = 0;
    virtual void downloadFailed() = 0;
};

/**
 * Handles downloading updates from the update server.
 */
class DownloadUpdates : public DownloadListener
{
public:
    /**
     * Constructor.
     *
     * @param updateHost Host where to get the updated files.
     */
    DownloadUpdates(const std::string &updateHost,
            UpdaterListener* listener);

    ~DownloadUpdates();

    /**
     * Spawn the asynchronous download thread.
     */
    void download();

    // from DownloadListener
    int downloadProgress(UpdateResource* resource,
            double downloaded, double size);

    /**
     * The user has press the cancel button (or whatever
     * represents this in the UI).
     */
    void userCancelled();

private:
    /**
     * Parse the update host and determine the updates directory
     * Then verify that the directory exists (creating if needed).
     */
    void setUpdatesDir(std::string &updateHost);

    /**
     * Get the full path to the updates dir.
     *
     * (Might make that a data memeber.)
     */
    std::string getUpdatesDirFullPath();

    /**
     * Parse the contents of resources2.txt.
     * Populates mResources.
     */
    void parseResourcesFile();

    /**
     * Reads the file "{Updates Directory}/resources2.txt" and attempts to load
     * each update mentioned in it.  The files need to have been downloaded
     * already - this just passes filenames to ResourceManager.
     */
    void addUpdatesToResman();

    int updateState;

    /**
     * The thread function that download the files.
     */
    static int downloadThread(void *ptr);
    int downloadThreadWithThis();

    /** A thread that use libcurl to download updates. */
    SDL_Thread *mThread;

    /** Host where we get the updated files. */
    std::string mUpdateHost;

    /** Place on the server to download from. */
    std::string mUpdatesDir;

    /** Flag that show if the user has canceled the update. */
    bool mUserCancel;

    /** Number of files completely downloaded, for the progress bar. */
    int mFilesComplete;

    /** List of files to download. */
    std::vector<UpdateResource*> mResources;

    UpdaterListener* mListener;
};

#endif
