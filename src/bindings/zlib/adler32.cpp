/*
 *  Aethyra
 *  Copyright (C) 2006  The Mana World Development Team
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

#include <cstdlib>
#include <zlib.h>

#include "adler32.h"

#include "../../core/log.h"

#include "../../core/utils/gettext.h"

unsigned long Adler32Verifier::fadler32(FILE *file)
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

Adler32Verifier::Adler32Verifier(std::string name, std::string url,
                                 std::string fullPath, CachePolicy cachePolicy,
                                 unsigned long checksum) :
    DownloadVerifier(name, url, fullPath, cachePolicy),
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
