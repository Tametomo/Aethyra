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

#include <iostream>
#include <SDL.h>
#include <SDL_thread.h>

#include "../../engine.h"
#include "../../main.h"

#include "../../core/configuration.h"
#include "../../core/log.h"
#include "../../core/resourcemanager.h"

#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

#include "downloadupdates.h"

namespace
{
        /**
         * Load the given file into a vector of strings.
         */
        std::vector<std::string> loadTextFile(const std::string &fileName)
        {
            std::vector<std::string> lines;
            std::ifstream fin(fileName.c_str());

            if (!fin)
            {
                logger->log("Couldn't load text file: %s", fileName.c_str());
                return lines;
            }

            std::string line;

            while (getline(fin, line))
                lines.push_back(line);

            return lines;
        }
};


DownloadUpdates::DownloadUpdates(const std::string &updateHost,
        UpdaterListener* listener) :
    mThread(NULL),
    mUpdateHost(updateHost),
    mUpdatesDir(""),
    mUserCancel(false),
    mListener(listener)
{
    setUpdatesDir(mUpdateHost);
}

DownloadUpdates::~DownloadUpdates()
{
    if (mThread)
    {
        mUserCancel = true;
        SDL_WaitThread(mThread, NULL);
    }

    typedef std::vector<UpdateResource*>::const_iterator CI;
    for (CI itr = mResources.begin() ; itr != mResources.end() ; itr++)
    {
        delete *itr;
    }
}

void DownloadUpdates::download()
{
    mThread = SDL_CreateThread(DownloadUpdates::downloadThread, this);

    if (!mThread)
    {
        logger->log("Unable to create mThread");
    }
}

void DownloadUpdates::userCancelled()
{
    // This triggers a cancel on the current download next time libcurl calls
    // the status fuction (which happens at least once every second).
    mUserCancel = true;
}

void DownloadUpdates::addUpdatesToResman()
{
    // If the user cancelled in UPDATE_NEWS or UPDATE_LIST, mResources hasn't been
    // populated yet.  But we might have all the updates already.
    if (mResources.empty())
        parseResourcesFile();

    ResourceManager *resman = ResourceManager::getInstance();

    typedef std::vector<UpdateResource*>::const_iterator CI;
    for (CI itr = mResources.begin() ; itr != mResources.end() ; itr++)
    {
        if ((*itr)->isSaneToDownload())
        {
            resman->addToSearchPath((*itr)->getFullPath(), false);
        }
        else
        {
            //TODO warn user - this is reachable if the user cancels and an
            //update after the one being downloaded failed the check.
        }
    }
}

void DownloadUpdates::setUpdatesDir(std::string &updateHost)
{
    //TODO The updateHost is a server-supplied string - with a directory
    //traversal attack from the directory-on-host part, it can create an
    //arbitary directory.

    std::stringstream updates;

    // If updatesHost is currently empty, fill it from config file
    if (updateHost.empty())
    {
        updateHost = config.getValue("updatehost",
                                     "http://www.aethyra.org/updates");
    }

    // Remove any trailing slash at the end of the update host
    if (updateHost.at(updateHost.size() - 1) == '/')
        updateHost.resize(updateHost.size() - 1);

    // Parse out any "http://" or "ftp://", and set the updates directory
    size_t pos;
    pos = updateHost.find("://");
    if (pos != updateHost.npos)
    {
        if (pos + 3 < updateHost.length())
        {
            updates << "updates/" << updateHost.substr(pos + 3);
            mUpdatesDir = updates.str();
        }
        else
        {
            logger->log("Error: Invalid update host: %s", updateHost.c_str());
            errorMessage = _("Invalid update host: ") + updateHost;
            state = ERROR_STATE;
        }
    }
    else
    {
        logger->log("Warning: no protocol was specified for the update host");
        updates << "updates/" << updateHost;
        mUpdatesDir = updates.str();
    }

    ResourceManager *resman = ResourceManager::getInstance();

    // Verify that the updates directory exists. Create if necessary.
    if (!resman->isDirectory("/" + mUpdatesDir))
    {
        if (!resman->mkdir("/" + mUpdatesDir))
        {
#if defined WIN32
            std::string newDir = engine->getHomeDir() + "\\" + mUpdatesDir;
            std::string::size_type loc = newDir.find("/", 0);

            while (loc != std::string::npos)
            {
                newDir.replace(loc, 1, "\\");
                loc = newDir.find("/", loc);
            }

            if (!CreateDirectory(newDir.c_str(), 0) &&
                GetLastError() != ERROR_ALREADY_EXISTS)
            {
                logger->log("Error: %s can't be made, but doesn't exist!",
                            newDir.c_str());
#else
            logger->log("Error: %s/%s can't be made, but doesn't exist!",
                        engine->getHomeDir().c_str(), mUpdatesDir.c_str());
#endif
            errorMessage = _("Error creating updates directory!");
            state = ERROR_STATE;
#if defined WIN32
            }
#endif
        }
    }
}

std::string DownloadUpdates::getUpdatesDirFullPath()
{
    return engine->getHomeDir() + "/" + mUpdatesDir + "/";
}

void DownloadUpdates::parseResourcesFile()
{
    typedef std::vector<UpdateResource*>::const_iterator CIR;
    for (CIR itr = mResources.begin() ; itr != mResources.end() ; itr++)
    {
        delete *itr;
    }
    mResources.clear();

    std::vector<std::string> lines =
            loadTextFile(getUpdatesDirFullPath() + "resources2.txt");

    typedef std::vector<std::string>::const_iterator CIS;
    for (CIS itr = lines.begin(); itr != lines.end(); itr++)
    {
        std::stringstream line(*itr);
        std::string file;
        line >> file;
        std::string checksumText;
        line >> checksumText;
        std::stringstream ss(checksumText);
        unsigned long checksum;
        ss >> std::hex >> checksum;

        std::string url = mUpdateHost + "/" + file;
        std::string fullPath = getUpdatesDirFullPath() + file;

        UpdateResourceAdler32* resource = new UpdateResourceAdler32(file, url,
                fullPath, CACHE_OK, checksum);
        mResources.push_back(resource);
    }
}


int DownloadUpdates::downloadThread(void *ptr)
{
    DownloadUpdates *self = reinterpret_cast<DownloadUpdates *>(ptr);
    return self->downloadThreadWithThis();
}


int DownloadUpdates::downloadThreadWithThis()
{
    DownloadWrapper dw(this);
    bool success = true;
    bool securityWorries = false;

    // The comments here have the pre-refactor state names

    /* UPDATE_NEWS:      Download news.txt file. */
    {
        std::string file = "news.txt";
        std::string url = mUpdateHost + "/" + file;
        std::string fullPath = getUpdatesDirFullPath() + file;
        UpdateResource resource(file, url, fullPath, CACHE_REFRESH);
        if (resource.isSaneToDownload())
            success = dw.downloadSynchronous(&resource);
        else
        {
            success = false;
            securityWorries = true;
        }

        if (success)
        {
            // Display news to user, with warning at start
            std::vector<std::string> lines = loadTextFile(fullPath);
            mListener->downloadTextUpdate(lines);
        }
    }

    /* UPDATE_LIST:      Download resources2.txt. */
    if (success)
    {
        mFilesComplete++;
        std::string file = "resources2.txt";
        std::string url = mUpdateHost + "/" + file;
        std::string fullPath = getUpdatesDirFullPath() + file;
        UpdateResource resource(file, url, fullPath, CACHE_REFRESH);
        if (resource.isSaneToDownload())
            success = dw.downloadSynchronous(&resource);
        else
        {
            success = false;
            securityWorries = true;
        }
    }

    /* UPDATE_RESOURCES: Download .zip files named in resources2.txt. */
    if (success)
    {
        mFilesComplete++;
        parseResourcesFile();
        typedef std::vector<UpdateResource*>::const_iterator CI;
        for (CI itr = mResources.begin() ; itr != mResources.end() ; itr++)
        {
            if (mUserCancel)
                break;

            if ((*itr)->isSaneToDownload())
            {
                if (dw.downloadSynchronous(*itr))
                    mFilesComplete++;
                else
                    success = false;
                    //but still try to download the other files
            }
            else
            {
                success = false;
                securityWorries = true;
                break;
            }
        }
    }

    if (securityWorries)
    {
        // This gives the user a nice prompt informing them that the update
        // downloading has failed, and gives them a chance to see why it failed.
        //
        // This will be able to be handled much more gracefully once the
        // refactored StateManager is available.
        std::vector<std::string> lines;
        lines.push_back(
            _("An update failed a security check, see log for details. If this "
              "persists, report this issue with your log file on the forums."));
        mListener->downloadTextUpdate(lines);

        /* UPDATE_COMPLETE:  Wait for user to press "play", then crash. */
        mListener->downloadFailed();
        return 0;
    }

    if (!success)
    {
        // This is really UI, probably better in updatewindow.cpp
        std::vector<std::string> lines;
        lines.push_back(_("##1  The update process is incomplete."));
        lines.push_back(_("##1  It is strongly recommended that"));
        lines.push_back(_("##1  you try again later"));
        //lines.push_back(mCurlError);
        mListener->downloadTextUpdate(lines);

        //continue, as it can still load any files that have downloaded
        //(this (!success) case includes the user pressing "cancel")
    }

    /* UPDATE_FINISH:    All downloads complete. */
    // The downloading has finished (or been cancelled)
    addUpdatesToResman();

    mListener->downloadComplete();
    /* UPDATE_COMPLETE:  Waiting for user to press "play". */

    return 0;
}

int DownloadUpdates::downloadProgress(UpdateResource* resource,
        double downloaded, double size)
{
    float progress = downloaded / size;
    if (progress != progress)
        progress = 0.0f; // check for NaN
    if (progress < 0.0f)
        progress = 0.0f; // no idea how this could ever happen,
                         // but why not check for it anyway.
    if (progress > 1.0f)
        progress = 1.0f;

    // Make the progress bar display overall progress, not just for this file
    // (inaccurate as it assumes all files are the same size)
    // (The +2 is for news.txt and resources2.txt)
    float totalProgress = (mFilesComplete + progress) / (mResources.size() + 2);

    mListener->downloadProgress(totalProgress, resource->getName(), progress);

    // If the action was canceled return an error code to stop the mThread
    if (mUserCancel)
        return -1;

    return 0;
}
