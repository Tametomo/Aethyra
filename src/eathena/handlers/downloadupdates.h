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

#include "../../bindings/zlib/adler32.h"

#include "../../core/utils/mutex.h"

/**
 * Callbacks to the GUI.
 * Likely to be in a different thread than the GUIChan one.
 */
class UpdaterListener
{
public:
    virtual ~UpdaterListener() {}

    virtual void downloadTextUpdate(const std::vector<std::string>& news) = 0;
    virtual void downloadProgress(float totalProgress, 
                                  const std::string& currentFile,
                                  float fileProgress) = 0;
    virtual void downloadComplete() = 0;
    virtual void downloadFailed() = 0;
    virtual void verifyingFiles() = 0;
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
    DownloadUpdates(const std::string &updateHost, UpdaterListener* listener);

    virtual ~DownloadUpdates();

    /**
     * Spawn the asynchronous download thread.
     */
    void download();

    // from DownloadListener
    int downloadProgress(DownloadVerifier* resource, double downloaded,
                         double size);

    /**
     * The user has press the cancel button (or whatever
     * represents this in the UI).
     */
    void userCancelled();

    /**
     * Reads the file "{Updates Directory}/resources2.txt" and attempts to load
     * each update mentioned in it.  The files need to have been downloaded
     * already - this just passes filenames to ResourceManager.
     */
    bool addUpdatesToResman();

private:
    enum VerificationStatus {
        CHECK_STATUS_UNKNOWN = -1,
        CHECK_FAILURE,
        CHECK_UNSUCCESSFUL,
        CHECK_SUCCESSFUL
    };

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
    bool parseResourcesFile();

    /**
     * The thread function that download the files.
     */
    static int downloadThread(void *ptr);
    int downloadThreadWithThis();

    /**
     * Checks whether all updates pass file verification.
     */
    VerificationStatus verifyUpdates();

    Mutex mMutex;

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
    std::vector<DownloadVerifier*> mResources;

    /** Lines to write to the updater listener */
    std::vector<std::string> mLines;

    UpdaterListener* mListener;
};

#endif
