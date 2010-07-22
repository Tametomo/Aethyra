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

#include <string>

typedef void CURL;

class DownloadVerifier;

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
    virtual int downloadProgress(DownloadVerifier* resource, double downloaded,
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
    bool downloadSynchronous(DownloadVerifier* resource);

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
    DownloadVerifier* mResource;
};

#endif
