/*
 *  Download wrapper for libcurl
 *
 *  Copyright (C) 2009  The Mana World Development Team
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

#include <iostream>
#include <zlib.h>
#include <ctype.h>

#include <curl/curl.h>

#include "downloadwrapper.h"

#include "../../engine.h"

#include "../../core/configuration.h"
#include "../../core/log.h"
#include "../../core/utils/gettext.h"

/**
 * Calculates the Alder-32 checksum for the given file.
 */
static unsigned long fadler32(FILE *file)
{
    // Obtain file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    // Calculate Adler-32 checksum
    char *buffer = (char*) malloc(fileSize);
    const size_t read = fread(buffer, 1, fileSize, file);
    unsigned long adler = adler32(0L, Z_NULL, 0);
    adler = adler32(adler, (Bytef*) buffer, read);
    free(buffer);

    return adler;
}

DownloadWrapper::DownloadWrapper(DownloadListener *listener) :
    mListener(listener),
    mCurlError(new char[CURL_ERROR_SIZE])
{
    mCurlError[0] = 0;
}

int DownloadWrapper::updateProgress(void *ptr, double dt, double dn, double ut,
                                    double un)
{
    DownloadWrapper *self = reinterpret_cast<DownloadWrapper *>(ptr);
    return self->mListener->downloadProgress(self->mResource, dn, dt);
}

bool DownloadWrapper::downloadSynchronous(GenericVerifier* resource)
{
    int attempts = 0;
    bool downloadComplete = false;
    CURL *curl;
    CURLcode res;

    if (!resource->isSaneToDownload())
        return false;

    mResource = resource;
    CachePolicy policy = resource->getCachePolicy();

    // Check if the file already exists
    {
        FILE* newfile = fopen(resource->getFullPath().c_str(), "rb");
        if (newfile)
        {
            if (policy == CACHE_OK)
            {
                if (resource->verify(newfile))
                {
                    logger->log("%s already here and verified",
                                resource->getName().c_str());
                    downloadComplete = true;

                    // Obtain file size and make a download progress callback
                    fseek(newfile, 0, SEEK_END);
                    long fileSize = ftell(newfile);
                    (void) mListener->downloadProgress(resource, fileSize,
                                                       fileSize);
                }
                else
                {
                    logger->log("%s already here, but doesn't verify",
                                resource->getName().c_str());
                    policy = CACHE_CORRUPTED;
                }

                fclose(newfile);
            }
            else
            {
                policy = CACHE_REFRESH;
                fclose(newfile);    // must close before deleting for Windows
                ::remove(resource->getFullPath().c_str());
            }
        }
    }

    while (attempts < 3 && !downloadComplete)
    {
        FILE *outfile = NULL;
        const std::string temporaryPath = resource->getFullPath() + ".temp";

        curl = curl_easy_init();

        if (curl)
        {
            logger->log("Downloading: %s", resource->getUrl().c_str());

            if (policy != CACHE_CORRUPTED)
                outfile = fopen(resource->getFullPath().c_str(), "w+b");
            else
                outfile = fopen(temporaryPath.c_str(), "w+b");

            if (!outfile)
                break;  // No point taking 3 attempts here

            curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);

#ifdef PACKAGE_VERSION
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Aethyra/" PACKAGE_VERSION);
#else
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Aethyra");
#endif
            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, mCurlError);
            curl_easy_setopt(curl, CURLOPT_URL, resource->getUrl().c_str());
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION,
                             DownloadWrapper::updateProgress);
            curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15);

            struct curl_slist *pHeaders = NULL;
            if (resource->getCachePolicy() == CACHE_REFRESH)
            {
                // Make sure that files marked as CACHE_REFRESH are always 
                // redownloaded, in order to always get the latest version.
                pHeaders = curl_slist_append(pHeaders, "pragma: no-cache");
                pHeaders = curl_slist_append(pHeaders, "Cache-Control: no-cache");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pHeaders);
            }

            if ((res = curl_easy_perform(curl)) != 0)
            {
                std::cerr << "curl error " << res << ": " << mCurlError
                          << " host: " << resource->getUrl() << std::endl;

                fclose(outfile);
                if (policy == CACHE_CORRUPTED)
                    ::remove(temporaryPath.c_str());
                else
                    ::remove(resource->getFullPath().c_str());
                attempts++;
                continue;   //FIXME this leaks curl
            }

            curl_easy_cleanup(curl);

            if (pHeaders)
                curl_slist_free_all(pHeaders);

            // Check checksum
            if (resource->verify(outfile))
            {
                downloadComplete = true;
                fclose(outfile);

                if (policy == CACHE_CORRUPTED)
                {
                    // Any existing file with this name is deleted first,
                    // otherwise the rename will fail on Windows.
                    ::remove(resource->getFullPath().c_str());
                    ::rename(temporaryPath.c_str(), resource->getFullPath().c_str());
                }
            }
            else
            {
                fclose(outfile);        // must close before deleting for Windows
                if (policy == CACHE_CORRUPTED)
                    ::remove(temporaryPath.c_str());
                else
                {
                    policy = CACHE_REFRESH; // for intermediate web caches
                    ::remove(resource->getFullPath().c_str());
                }
            }
        }
        attempts++;
    }

    mResource = NULL;

    return downloadComplete;
}

DownloadWrapper::~DownloadWrapper()
{
    delete[] mCurlError;
}

GenericVerifier::GenericVerifier(std::string name, std::string url,
                                 std::string fullPath, CachePolicy cachePolicy) :
    mCachePolicy(cachePolicy),
    mName(name),
    mUrl(url),
    mFullPath(fullPath)
{
}

Adler32Verifier::Adler32Verifier(std::string name, std::string url,
                                 std::string fullPath, CachePolicy cachePolicy,
                                 unsigned long checksum) :
    GenericVerifier(name, url, fullPath, cachePolicy),
    mChecksum(checksum)
{
}

bool Adler32Verifier::verify(FILE* file) const
{
    unsigned long adler = fadler32(file);
    if (adler != mChecksum)
    {
        logger->log(_("Checksum for file %s failed: (%lx/%lx)"),
                      getName().c_str(), adler, mChecksum);
        return false;
    }
    return true;
}

bool GenericVerifier::isSaneToDownload() const
{
    // Sanity checks for security issues.  Ideally this would
    // be in a library, but PhysFS doesn't provide it for
    // native filenames.

    // Check for a directory traversal attempt.
    // As a special case, we allow filenames such as
    //   update-548aa36..ac86ea6.zip
    // which contain one ".." between two hex digits
    // (matching Git's commit..commit notation)
    std::string::size_type pos = mFullPath.find("..");
    if (std::string::npos != pos)
    {
        // Contains a "..".  Check the special case
        if ((pos > 0) &&
            (pos < (mFullPath.size() - 3)) &&
            (std::string::npos == mFullPath.find("..", pos + 1)) &&
            isxdigit(mFullPath[pos - 1]) &&
            isxdigit(mFullPath[pos + 2]))
        {
            logger->log("Info: recognised special case '..' in '%s'",
                        mFullPath.c_str());
        }
        else
        {
            logger->log("Error: suspect directory traversal ('..') in '%s'",
                        mFullPath.c_str());
            return false;
        }
    }

    // Check for scp URLs (as in libcurl-tutorial(3) )
    // One of these compare()s needs to return zero.
    const std::string http("http://");
    const std::string  ftp( "ftp://");
    if (mUrl.compare(0, http.size(), http) && mUrl.compare(0, ftp.size(), ftp))
        return false;

    // Haven't spotted anything, so let it through.
    return true;
}
