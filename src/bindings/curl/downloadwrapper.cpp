/*
 *  Download wrapper for libcurl
 *
 *  Copyright (C) 2005  The Mana World Development Team
 *  Copyright (C) 2009  Aethyra Development Team
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

#include <cctype>
#include <limits>

#include <curl/curl.h>

#include "downloadwrapper.h"
#include "verifier.h"

#include "../../core/log.h"

DownloadWrapper::DownloadWrapper(DownloadListener *listener) :
    mCanceled(false),
    mListener(listener),
    mCurl(curl_easy_init()),
    mCurlError(new char[CURL_ERROR_SIZE]),
    mResource(NULL)
{
    mCurlError[0] = 0;
}

DownloadWrapper::~DownloadWrapper()
{
    curl_easy_cleanup(mCurl);
    delete[] mCurlError;
}

int DownloadWrapper::updateProgress(void *ptr, double dt, double dn, double ut,
                                    double un)
{
    DownloadWrapper *self = reinterpret_cast<DownloadWrapper *>(ptr);
    return self->mListener->downloadProgress(self->mResource, dn, dt);
}

bool DownloadWrapper::downloadSynchronous(DownloadVerifier* resource)
{
    if (!resource || !resource->isSaneToDownload())
        return false;

    mResource = resource;
    CachePolicy policy = resource->getCachePolicy();
    std::string downloadPath = resource->getFullPath();
    const std::string temporaryPath = downloadPath + ".temp";

    // Check if the file already exists
    {
        FILE* newfile = fopen(resource->getFullPath().c_str(), "rb");
        bool checkForTempFile = false;
        if (newfile)
        {
            if (policy == CACHE_OK)
            {
                if (resource->verify(newfile))
                {
                    logger->log("%s already here and verified",
                                resource->getName().c_str());

                    // Obtain file size and make a download progress callback
                    fseek(newfile, 0, SEEK_END);
                    long fileSize = ftell(newfile);
                    (void) mListener->downloadProgress(resource, fileSize,
                                                       fileSize);

                    fclose(newfile);
                    return true; // No need to continue with the rest of the
                                 // download thread
                }
                else
                {
                    logger->log("%s already here, but doesn't verify",
                                resource->getName().c_str());
                    policy = CACHE_REFRESH;
                    checkForTempFile = true;
                }
            }
            else
            {
                policy = CACHE_REFRESH;
                checkForTempFile = true;
            }
            fclose(newfile);    // must close before deleting for Windows
        }

        // Ensure that if a temporary download file exists, that it's deleted
        // before we attempt to use it. This would only exist if the client
        // was forced to quit before it was allowed to clean up after itself.
        if (checkForTempFile)
        {
            newfile = fopen(temporaryPath.c_str(), "rb");
            if (newfile)
            {
                ::remove(temporaryPath.c_str());
                fclose(newfile);
            }

            downloadPath = temporaryPath;
        }
    }

    int attempts = 0;
    bool downloadComplete = false;
    CURLcode res;

    while (attempts < 3 && !downloadComplete && !mCanceled)
    {
#ifdef WIN32
        curl_easy_cleanup(mCurl);
        mCurl = curl_easy_init();
#endif
        if (mCurl)
        {
            logger->log("Downloading: %s", resource->getUrl().c_str());

            FILE *outfile = fopen(downloadPath.c_str(), "w+b");

            if (!outfile)
                break;  // No point taking 3 attempts here

            curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, outfile);

#ifdef PACKAGE_VERSION
            curl_easy_setopt(mCurl, CURLOPT_USERAGENT, "Aethyra/" PACKAGE_VERSION);
#else
            curl_easy_setopt(mCurl, CURLOPT_USERAGENT, "Aethyra");
#endif
            curl_easy_setopt(mCurl, CURLOPT_ERRORBUFFER, mCurlError);
            curl_easy_setopt(mCurl, CURLOPT_URL, resource->getUrl().c_str());
            curl_easy_setopt(mCurl, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(mCurl, CURLOPT_PROGRESSFUNCTION,
                             DownloadWrapper::updateProgress);
            curl_easy_setopt(mCurl, CURLOPT_PROGRESSDATA, this);
            curl_easy_setopt(mCurl, CURLOPT_NOSIGNAL, 1);
            curl_easy_setopt(mCurl, CURLOPT_CONNECTTIMEOUT, 15);

            struct curl_slist *pHeaders = NULL;
            if (resource->getCachePolicy() == CACHE_REFRESH)
            {
                // Make sure that files marked as CACHE_REFRESH are always 
                // redownloaded, in order to always get the latest version.
                pHeaders = curl_slist_append(pHeaders, "pragma: no-cache");
                pHeaders = curl_slist_append(pHeaders, "Cache-Control: no-cache");
                curl_easy_setopt(mCurl, CURLOPT_HTTPHEADER, pHeaders);
            }

            if ((res = curl_easy_perform(mCurl)) != 0)
            {
                logger->log("curl error %d : %s host: %s", res,
                            mCurlError, resource->getUrl().c_str());

                fclose(outfile);
                ::remove(downloadPath.c_str());

                if (pHeaders)
                {
                    curl_easy_setopt(mCurl, CURLOPT_HTTPHEADER, NULL);
                    curl_slist_free_all(pHeaders);
                }

                attempts++;
                continue;
            }

            int httpCode = 0;
            curl_easy_getinfo (mCurl, CURLINFO_RESPONSE_CODE, &httpCode);

            // Handle all cases except for finding the download as an
            // unreachable state for now. TODO: Handle other response codes, if
            // appropriate.
            if (httpCode != 202)
            {
                attempts = INT_MAX;
                logger->log("%s is currently unavailable online.",
                            resource->getName().c_str());

                // This is to avoid sending the wrong pointer, should the 
                // resource change before being accessed
                mListener->downloadUnreachable(*resource, httpCode);

                // Ensure we don't leave failed fragments around
                fclose(outfile);
                ::remove(downloadPath.c_str());
                break;
            }

            if (pHeaders)
            {
                curl_easy_setopt(mCurl, CURLOPT_HTTPHEADER, NULL);
                curl_slist_free_all(pHeaders);
            }

            // Check checksum
            if (resource->verify(outfile))
            {
                downloadComplete = true;
                fclose(outfile);

                if (downloadPath.compare(resource->getFullPath()) != 0)
                {
                    // Any existing file with this name is deleted first,
                    // otherwise the rename will fail on Windows.
                    ::remove(resource->getFullPath().c_str());
                    ::rename(downloadPath.c_str(), resource->getFullPath().c_str());
                }
            }
            else
            {
                fclose(outfile);        // must close before deleting for Windows
                ::remove(downloadPath.c_str());
                policy = CACHE_REFRESH; // for intermediate web caches
            }
        }
        attempts++;
    }

    mResource = NULL;

    return downloadComplete;
}

void DownloadWrapper::cancelDownload()
{
    // This triggers a cancel on the current download next time libcurl calls
    // the status fuction (which happens at least once every second).
    mCanceled = true;
}

