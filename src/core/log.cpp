/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
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
#include <sstream>
#include <stdarg.h>
#include <stdlib.h>

#include <sys/time.h>

#ifdef WIN32
#include <windows.h>
#elif __APPLE__
#include <Carbon/Carbon.h>
#endif

#include "log.h"

#include "../bindings/guichan/graphics.h"

#include "../eathena/statemanager.h"

#include "../eathena/gui/chat.h"

Logger::Logger():
    mLogToStandardOut(false),
    mLogToChatWindow(false)
{
}

Logger::~Logger()
{
    if (mLogFile.is_open())
        mLogFile.close();
}

void Logger::setLogFile(const std::string &logFilename)
{
    mLogFile.open(logFilename.c_str(), std::ios_base::trunc);

    if (!mLogFile.is_open())
        std::cout << "Warning: error while opening " << logFilename <<
                     " for writing.\n";
}

void Logger::log(const char *log_text, ...)
{
    if (!mLogFile.is_open())
        return;

    char* buf = new char[1024];
    va_list ap;

    // Use a temporary buffer to fill in the variables
    va_start(ap, log_text);
    vsprintf(buf, log_text, ap);
    va_end(ap);

    // Get the current system time
    timeval tv;
    gettimeofday(&tv, NULL);

    // Print the log entry
    std::stringstream timeStr;
    timeStr << "["
        << ((((tv.tv_sec / 60) / 60) % 24 < 10) ? "0" : "")
        << (int)(((tv.tv_sec / 60) / 60) % 24)
        << ":"
        << (((tv.tv_sec / 60) % 60 < 10) ? "0" : "")
        << (int)((tv.tv_sec / 60) % 60)
        << ":"
        << ((tv.tv_sec % 60 < 10) ? "0" : "")
        << (int)(tv.tv_sec % 60)
        << "."
        << (((tv.tv_usec / 10000) % 100) < 10 ? "0" : "")
        << (int)((tv.tv_usec / 10000) % 100)
        << "] ";

    mLogFile << timeStr.str() << buf << std::endl;

    if (mLogToStandardOut)
        std::cout << timeStr.str() << buf << std::endl;

    if (chatWindow && mLogToChatWindow)
        chatWindow->chatLog(buf, Palette::LOGGER);

    // Delete temporary buffer
    delete[] buf;
}

void Logger::error(const std::string &error_text)
{
    log("Error: %s", error_text.c_str());

    if (graphics && graphics->initialized())
        stateManager->handleException(error_text.c_str(), LOGOUT_STATE);
    else
    {
#ifdef WIN32
        MessageBox(NULL, error_text.c_str(), "Error", MB_ICONERROR | MB_OK);
#elif defined __APPLE__
        Str255 msg;
        CFStringRef error;
        error = CFStringCreateWithCString(NULL, error_text.c_str(),
                                          kCFStringEncodingMacRoman);
        CFStringGetPascalString(error, msg, 255, kCFStringEncodingMacRoman);
        StandardAlert(kAlertStopAlert, "\pError", (ConstStr255Param) msg, NULL,
                      NULL);
#else
        std::cerr << "Error: " << error_text.c_str() << std::endl;
#endif
        exit(1);
    }
}
