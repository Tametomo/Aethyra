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

#ifdef WIN32
#include <windows.h>
#endif

#include "downloadupdates.h"

#include "../statemanager.h"

#include "../../engine.h"

#include "../../core/configuration.h"
#include "../../core/log.h"
#include "../../core/resourcemanager.h"

#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

namespace
{
    /**
     * Load the given file into a vector of strings.
     */
    std::vector<std::string> loadTextFile(const std::string &fileName)
    {
        logger->log("Loading text file: %s", fileName.c_str());

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
    mFilesComplete(0),
    mListener(listener)
{
    setUpdatesDir(mUpdateHost);
}

DownloadUpdates::~DownloadUpdates()
{
    if (mThread)
    {
        mUserCancel = true;
        SDL_KillThread(mThread);
    }

    delete_all(mResources);
    mResources.clear();
}

void DownloadUpdates::download()
{
    mThread = SDL_CreateThread(DownloadUpdates::downloadThread, this);

    if (!mThread)
        logger->log("Unable to create mThread");
}

void DownloadUpdates::userCancelled()
{
    // This triggers a cancel on the current download next time libcurl calls
    // the status fuction (which happens at least once every second).
    mUserCancel = true;
}

bool DownloadUpdates::addUpdatesToResman()
{
    mMutex.lock();

    ResourceManager *resman = ResourceManager::getInstance();
    bool success = true;

    typedef std::vector<DownloadVerifier*>::const_iterator CI;
    for (CI itr = mResources.begin() ; itr != mResources.end() ; itr++)
    {
        if ((*itr)->isSaneToDownload())
        {
            // Don't attempt to load a nonexistent file
            if ((*itr)->fileExists())
                success = resman->addToSearchPath((*itr)->getFullPath(), false);
            else
                success = false;
        }

        if (!success)
            break;
    }

    return success;

    mMutex.unlock();
}

DownloadUpdates::VerificationStatus DownloadUpdates::verifyUpdates()
{
    mMutex.lock();

    VerificationStatus status = CHECK_SUCCESSFUL;

    typedef std::vector<DownloadVerifier*>::const_iterator CI;
    for (CI itr = mResources.begin() ; itr != mResources.end() ; itr++)
    {
        // Hack to work around some compilers reducing logic checks to the
        // enumeration check only.
        bool success = (status == CHECK_SUCCESSFUL);

        if ((*itr)->isSaneToDownload())
        {
            // If the file doesn't exist, throw a fatal error, so that the
            // client doesn't proceed.
            if (!(*itr)->fileExists())
            {
                status = CHECK_FAILURE;
                break;
            }
            else if ((*itr)->verify() && success)
                status = CHECK_SUCCESSFUL;
            else
                status = CHECK_UNSUCCESSFUL;
        }
        // The file name could potentially be an exploit. Don't allow the client
        // to proceed.
        else
        {
            status = CHECK_FAILURE;
            break;
        }
    }

    return status;

    mMutex.unlock();
}

void DownloadUpdates::setUpdatesDir(std::string &updateHost)
{
    // FIXME: The updateHost is a server-supplied string - with a directory
    // traversal attack from the directory-on-host part, it can create an
    // arbitary directory.

    std::stringstream updates;

    // If updatesHost is currently empty, fill it from config file
    if (updateHost.empty())
    {
        updateHost = config.getValue("updatehost",
                                     "http://www.aethyraproject.org/updates");
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
            stateManager->handleException(strprintf(_("Invalid update host: %s"),
                                                    updateHost.c_str()), LOGIN_STATE);
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
            stateManager->handleException(_("Error creating updates directory!"),
                                          LOGIN_STATE);
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

bool DownloadUpdates::parseResourcesFile()
{
    mMutex.lock();

    delete_all(mResources);
    mResources.clear();

    std::vector<std::string> lines = loadTextFile(getUpdatesDirFullPath() +
                                                  "resources2.txt");

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

        Adler32Verifier* resource = new Adler32Verifier(file, url, fullPath,
                                                        CACHE_OK, checksum);
        mResources.push_back(resource);
    }

    mMutex.unlock();

    return (mResources.size() > 0);
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

    /* UPDATE_LIST:      Download resources2.txt. */
    {
        std::string file = "resources2.txt";
        std::string url = mUpdateHost + "/" + file;
        std::string fullPath = getUpdatesDirFullPath() + file;
        DownloadVerifier resource(file, url, fullPath, CACHE_REFRESH);

        if (resource.isSaneToDownload())
        {
            success = dw.downloadSynchronous(&resource);
        }
        else
        {
            success = false;
            securityWorries = true;
        }

        // Check to ensure that resources2.text exists, since download wasn't
        // successful.
        if (!success && !securityWorries)
            securityWorries = !resource.fileExists();

        if (!securityWorries)
            success = parseResourcesFile();
    }

    /* UPDATE_NEWS:      Download news.txt file. */
    if (success)
    {
        std::string file = "news.txt";
        std::string url = mUpdateHost + "/" + file;
        std::string fullPath = getUpdatesDirFullPath() + file;
        DownloadVerifier resource(file, url, fullPath, CACHE_REFRESH);
        bool updatedNews = false;

        if (resource.isSaneToDownload())
        {
            // Don't download the news file if user has already canceled.
            if (!mUserCancel)
                updatedNews = dw.downloadSynchronous(&resource);
        }
        else
        {
            securityWorries = true;
        }

        mFilesComplete++;

        if (mListener)
        {
            mLines.clear();

            // Display news to user, with warning at start
            if (updatedNews)
                mLines = loadTextFile(fullPath);
            else
                mLines.push_back(_("##0  No news has been provided on this server."));

            mListener->downloadTextUpdate(mLines);
        }
    }

    /* UPDATE_RESOURCES: Download .zip files named in resources2.txt. */
    if (success && !mUserCancel)
    {
        mFilesComplete++;
        typedef std::vector<DownloadVerifier*>::const_iterator CI;
        for (CI itr = mResources.begin() ; itr != mResources.end() ; itr++)
        {
            if (mUserCancel)
            {
                dw.cancelDownload();

                if (mThread && SDL_GetThreadID(mThread) != 0)
                {
                    SDL_WaitThread(mThread, NULL);
                    mThread = NULL;
                }
                break;
            }

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

    /* UPDATE_FINISH:    All downloads complete. */
    // The downloading has finished (or been cancelled). The final success
    // state will be entirely dependent on whether the updates can be loaded
    // successfully.

    // Download canceled, so file verification needs to take place.
    if (mUserCancel)
    {
        mListener->verifyingFiles();

        VerificationStatus status = verifyUpdates();

        success = (status == CHECK_SUCCESSFUL);

        if (!securityWorries)
            securityWorries = (status == CHECK_FAILURE);
    }

    bool filesVerified = success;

    success = addUpdatesToResman();

    if (mListener)
    {
        if (success)
        {
            if (!filesVerified)
                stateManager->setState(UPDATE_ERROR_STATE);
            else
                mListener->downloadComplete();
        }
        else
        {
            // These blocks are really UI, probably better in updatewindow.cpp
            if (securityWorries)
            {
                // This gives the user a nice prompt informing them that the update
                // downloading has failed, and gives them a chance to see why it failed.
                stateManager->handleException(_("An update failed a security check, "
                                                "see log for details. If this persists,"
                                                " report this issue with your log file "
                                                "on the forums."), QUIT_STATE);
            }
            else if (mFailedResources.size() > 0)
            {
                std::string files;

                for (size_t i = 0; i < mFailedResources.size(); i++)
                {
                    if (i == (mFailedResources.size() - 1) &&
                        mFailedResources.size() > 1)
                    {
                        files.append(_("and"));
                        files.append(" ");
                    }

                    files.append(mFailedResources[i].getName());

                    if (i != (mFailedResources.size() - 1) &&
                        mFailedResources.size() > 1)
                    {
                        files.append(", ");
                    }
                }

                mLines.clear();
                // TODO: This particular line probably should be handled through
                //       gettext's plurals functionality. Either come up with a
                //       better way of phrasing this that won't require using
                //       plurals, or implement plurals in the gettext wrapper.
                mLines.push_back(strprintf(_("##1  The file(s) %s "), files.c_str()));
                mLines.push_back(_("##1  are currently unavailable online."));
                mLines.push_back(_("##1  Please notify the server administrator of"));
                mLines.push_back(_("##1  this issue."));
                mListener->downloadTextUpdate(mLines);
            }
            else
            {
                mLines.clear();
                mLines.push_back(_("##1  The update process is incomplete."));
                mLines.push_back(_("##1  It is strongly recommended that"));
                mLines.push_back(_("##1  you try again later"));
                mListener->downloadTextUpdate(mLines);
            }
            mListener->downloadFailed();
        }
    }

    /* UPDATE_COMPLETE:  Waiting for user to press "play" or "quit". */

    mThread = NULL;
    return 0;
}

void DownloadUpdates::downloadUnreachable(DownloadVerifier& resource,
                                          int httpCode)
{
    mFailedResources.push_back(resource);
}

int DownloadUpdates::downloadProgress(DownloadVerifier* resource,
                                      double downloaded, double size)
{
    // Prevent crashing the client unnecessarily
    if (!resource)
        return -1;

    mMutex.lock();

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
    // NOTE: This loops to 100% twice: once when downloading the resources2.txt
    //       file, since at that point it doesn't know if there will be other
    //       files, and later again when all of the files are completed.
    float totalFiles = (float) (mResources.size() <= 0) ? 1 : (mResources.size() + 2);
    float totalProgress = (mFilesComplete + progress) / totalFiles;

    if (mListener && resource)
        mListener->downloadProgress(totalProgress, resource->getName(), progress);

    mMutex.unlock();

    // If the action was canceled return an error code to stop the mThread
    return (mUserCancel ? -1 : 0);
}
