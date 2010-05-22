/*
 *  Download wrapper for libcurl
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

#ifndef _UPDATEDOWNLOAD_H
#define _UPDATEDOWNLOAD_H

#include <cstdio>
#include <string>
#include <fstream>

/**
 * Whether to use a previously-downloaded version of an GenericVerifier.
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

typedef void CURL;

class GenericVerifier
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
    GenericVerifier(std::string name, std::string url, std::string fullPath,
                    CachePolicy cachePolicy);

    virtual ~GenericVerifier() {};

    /**
     * Returns true if the file passes whatever tests this GenericVerifier has
     * for spotting corrupt files.
     * (Checksums etc).
     *
     * The basic GenericVerifier has no checksum.
     */
    virtual bool verify(FILE* file) const { return true; }

    /**
     * Returns true if the file passes whatever tests this GenericVerifier has
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

/**
 * GenericVerifier that uses an Adler32 checksum to verify the downloaded file.
 */
class Adler32Verifier : public GenericVerifier
{
public:
    /**
     * Details of a file to download.
     *
     *@param name Name of the resource, used for display and logging
     *@param url Where to download it from
     *@param fullPath Where to download it to
     *@param checksum Adler32 checksum
     *@param cachePolicy @see{CachePolicy}
     */
    Adler32Verifier(std::string name, std::string url, std::string fullPath,
                    CachePolicy cachePolicy, unsigned long checksum);

    virtual bool verify(FILE* file) const;

private:
    unsigned long mChecksum;
};

class DownloadListener
{
public:
    virtual ~DownloadListener() {}

    /**
     * Reports the number of bytes downloaded, and the
     * total number of bytes to download.
     * If the webserver didn't give a size, the size will be zero.
     *
     * The callee should return 0 to continue downloading,
     * or -1 to abort.
     */
    virtual int downloadProgress(GenericVerifier* resource, double downloaded,
                                 double size) = 0;
};

class DownloadWrapper
{
public:
    /**
     * Constructor.
     *
     *@param listener Listener for status callbacks.
     */
    DownloadWrapper(DownloadListener *listener);

    virtual ~DownloadWrapper();

    /**
     * Perform a synchronous download, blocking the caller until
     * it completes.  During the download, there will be callbacks
     * via the listener.
     *
     *@return true if the download succeeded, false if it failed.
     */
    bool downloadSynchronous(GenericVerifier* resource);

    void cancelDownload();

private:
    /**
     * A libcurl callback for progress updates.
     */
    static int updateProgress(void *ptr, double dt, double dn, double ut,
                              double un);

    bool mCanceled;

    DownloadListener *mListener;

    CURL *mCurl;

    /** Buffer for libcurl's human readable error messages. */
    char *mCurlError;

    /**
     * What's being downloaded.
     * (null unless downloadSynchronous is running).
     */
    GenericVerifier* mResource;
};

#endif
