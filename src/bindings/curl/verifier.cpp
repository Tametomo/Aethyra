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

#include "verifier.h"

#include "../../core/log.h"

DownloadVerifier::DownloadVerifier(std::string name, std::string url,
                                   std::string fullPath, CachePolicy cachePolicy) :
    mCachePolicy(cachePolicy),
    mName(name),
    mUrl(url),
    mFullPath(fullPath)
{
}

bool DownloadVerifier::verify()
{
    FILE *newFile = fopen(mFullPath.c_str(), "rb");
    bool verifies = verify(newFile);
    fclose(newFile);
    return verifies;
}

bool DownloadVerifier::isSaneToDownload() const
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
        if ((pos > 0) && (pos < (mFullPath.size() - 3)) &&
            (std::string::npos == mFullPath.find("..", pos + 1)) &&
            isxdigit(mFullPath[pos - 1]) && isxdigit(mFullPath[pos + 2]))
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

