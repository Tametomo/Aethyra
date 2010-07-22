/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra.
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

#ifndef ADLER32_H
#define ADLER32_H

#include <cstdio>

#include "../curl/verifier.h"

/**
 * DownloadVerifier that uses an Adler32 checksum to verify the downloaded file.
 */
class Adler32Verifier : public DownloadVerifier
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
        /**
         * Calculates the Alder-32 checksum for the given file.
         */
        static unsigned long fadler32(FILE *file);

        unsigned long mChecksum;
};

#endif // ADLER32_H
