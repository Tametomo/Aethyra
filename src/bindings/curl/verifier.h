/*
 *  Abstract file verifier to be used with download management.
 *
 *  Copyright (C) 2009  Aethyra Development Team
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

#ifndef DOWNLOAD_VERIFIER_H
#define DOWNLOAD_VERIFIER_H

#include <fstream>
#include <string>

/**
 * Whether to use a previously-downloaded version of an DownloadVerifier.
 *
 * If all of the following are true:
 * * a file with this name already exists
 * * the policy is CACHE_OK
 * * verify() succeeds on that file
 * then DownloadWrapper leaves the existing file and reports success.
 *
 * If either of these are true:
 * * the policy is CACHE_REFRESH
 * * verify() fails (on an existing file)
 * then DownloadWrapper requests any intermediate web caches
 * to refresh their cache too.
 */
enum CachePolicy
{
    CACHE_OK,        /**< Use already-downloaded version. */
    CACHE_REFRESH    /**< Re-download existing files. */
};

class DownloadVerifier
{
public:
    /**
     * Details of a file to download.
     *
     *@param name Name of the resource, used for display and logging
     *@param url Where to download it from
     *@param fullPath Where to download it to
     *@param cachePolicy @see{CachePolicy}
     */
    DownloadVerifier(std::string name, std::string url, std::string fullPath,
                     CachePolicy cachePolicy);

    virtual ~DownloadVerifier() {};

    /**
     * Returns true if the file passes whatever tests this DownloadVerifier has
     * for spotting corrupt files.
     * (Checksums etc).
     *
     * The basic DownloadVerifier has no checksum.
     */
    virtual bool verify(FILE* file) const { return true; }

    /**
     * Returns true if the file passes whatever tests this DownloadVerifier has
     * for spotting corrupt files.
     * (Checksums etc).
     *
     * This version is for use when the file to be tested isn't already open.
     */
    bool verify();

    /**
     * Whether or not the file currently can be accessed or not.
     */
    bool fileExists()
    {
        std::ifstream testFile(mFullPath.c_str());
        return testFile.is_open();
    }

    /**
     * Run some sanity checks on the URL and local filename.
     *
     * This is intended to be called (and handled specially) by users of the
     * DownloadWrapper, not just as an assert in the DownloadWrapper itself.
     */
    bool isSaneToDownload() const;

    const std::string getName() const { return mName; }
    const std::string getUrl() const { return mUrl; }
    const std::string getFullPath() const { return mFullPath; }

    CachePolicy getCachePolicy() const { return mCachePolicy; }

protected:
    /**
     * Use an HTTP "Cache-Contol: no-cache" header.
     */
    CachePolicy mCachePolicy;

private:
    /** Display name */
    std::string mName;

    /** Download from this URL. */
    std::string mUrl;

    /** Download to this file. */
    std::string mFullPath;
};

#endif
