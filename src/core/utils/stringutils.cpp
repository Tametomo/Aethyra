/*
 *  Aethyra
 *  Copyright (C) 2007  The Mana World Development Team
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

#include "stringutils.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>

std::string &trim(std::string &str, char token)
{
    std::string::size_type pos = str.find_last_not_of(token);
    if (pos != std::string::npos)
    {
        str.erase(pos + 1);
        pos = str.find_first_not_of(token);

        if (pos != std::string::npos)
            str.erase(0, pos);
    }
    else
    {
        // There is nothing else but whitespace in the string
        str.clear();
    }
    return str;
}

std::string &toLower(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}

std::string &normalize(std::string &str)
{
    return toLower(trim(str));
}

std::string getToken(const std::string &str)
{
    std::string token = str;
    trim(token);

    std::string::size_type pos = str.find("\"", 1);
    if (pos != std::string::npos && token.substr(0, 1) == "\"")
    {
        token = token.substr(1, pos - 1);
        trim(token);
    }
    else
    {
        const std::string::size_type space = str.find(" ");
        if (space != std::string::npos)
            token = token.substr(0, space);
    }

    return token;
}

std::string stripToken(const std::string &str)
{
    std::string token = str;
    trim(token);

    std::string::size_type pos = str.find("\"", 1);
    if (pos != std::string::npos && token.substr(0, 1) == "\"")
    {
        token = token.substr(pos);
        trim(token);
    }
    else
    {
        const std::string::size_type space = str.find(" ");
        if (space != std::string::npos)
            token = token.substr(space);
    }

    return token;
}

unsigned int atox(const std::string &str)
{
    unsigned int value;
    sscanf(str.c_str(), "0x%06x", &value);

    return value;
}

const char *ipToString(int address)
{
    static char asciiIP[16];

    sprintf(asciiIP, "%i.%i.%i.%i",
            (unsigned char)(address),
            (unsigned char)(address >> 8),
            (unsigned char)(address >> 16),
            (unsigned char)(address >> 24));

    return asciiIP;
}

std::string strprintf(char const *format, ...)
{
    char buf[256];
    va_list(args);
    va_start(args, format);
    int nb = vsnprintf(buf, 256, format, args);
    va_end(args);
    if (nb < 256)
    {
        return buf;
    }
    // The static size was not big enough, try again with a dynamic allocation.
    ++nb;
    char *buf2 = new char[nb];
    va_start(args, format);
    vsnprintf(buf2, nb, format, args);
    va_end(args);
    std::string res(buf2);
    delete [] buf2;
    return res;
}

int getStringTruthValue(const std::string &text)
{
    std::string a = text;
    a = getToken(normalize(a));
    if (a.compare("t") == 0 || a.compare("true") == 0 || a.compare("1") == 0 ||
        a.compare("on") == 0 || a.compare("yes") == 0 || a.compare("y") == 0)
        return 1;
    if (a.compare("f") == 0 || a.compare("false") == 0 || a.compare("0") == 0 ||
        a.compare("off") == 0 || a.compare("no") == 0 || a.compare("n") == 0)
        return 0;
    else
        return -1;
}

