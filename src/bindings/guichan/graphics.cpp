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

#include <cassert>
#include <fstream>
#include <physfs.h>
#include <sstream>
#include <SDL.h>

#include "graphics.h"

#include "../../core/log.h"

#include "../../core/image/imageloader.h"
#include "../../core/image/imagewriter.h"

#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

#include "../../eathena/gui/chat.h"

#ifdef HAVE_CONFIG_H
#include "../../../config.h"
#endif

Graphics::Graphics():
    mTarget(NULL),
    mWidth(0),
    mHeight(0),
    mBpp(0),
    mAlpha(false),
    mFullscreen(false),
    mHWAccel(false)
{
}

bool Graphics::resizeVideoMode(int w, int h)
{
    bool success = setVideoMode(w, h, mBpp, mFullscreen, mHWAccel);

    if (success)
    {
        gcn::Graphics::popClipArea();
        _beginDraw();
    }

    return success;
}

bool Graphics::setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel)
{
    logger->log("Setting video mode %dx%d %s", w, h, fs ? "fullscreen" :
                "windowed");

    logger->log("Bits per pixel: %d", bpp);

    mWidth = w;
    mHeight = h;
    mBpp = bpp;
    mFullscreen = fs;
    mHWAccel = hwaccel;

    return true;
}

bool Graphics::setFullscreen(bool fs)
{
    if (mFullscreen == fs)
        return true;

#ifdef X11
    logger->log("Switching to %s mode", fs ? "fullscreen" : "windowed");
    return SDL_WM_ToggleFullScreen(mTarget);
#else
    return setVideoMode(mWidth, mHeight, mBpp, fs, mHWAccel);
#endif
}

void Graphics::drawImageRect(int x, int y, int w, int h, Image *topLeft,
                             Image *topRight, Image *bottomLeft,
                             Image *bottomRight, Image *top, Image *right,
                             Image *bottom, Image *left, Image *center)
{
    pushClipArea(gcn::Rectangle(x, y, w, h));

    // Draw the center area
    drawImagePattern(center, topLeft->getWidth(), topLeft->getHeight(),
                     w - topLeft->getWidth() - topRight->getWidth(),
                     h - topLeft->getHeight() - bottomLeft->getHeight());

    // Draw the sides
    drawImagePattern(top, left->getWidth(), 0, w - left->getWidth() - 
                     right->getWidth(), top->getHeight());
    drawImagePattern(bottom, left->getWidth(), h - bottom->getHeight(),
                     w - left->getWidth() - right->getWidth(),
                     bottom->getHeight());
    drawImagePattern(left, 0, top->getHeight(), left->getWidth(), 
                     h - top->getHeight() - bottom->getHeight());
    drawImagePattern(right, w - right->getWidth(), top->getHeight(),
                     right->getWidth(), h - top->getHeight() - 
                     bottom->getHeight());

    // Draw the corners
    drawImage(topLeft, 0, 0);
    drawImage(topRight, w - topRight->getWidth(), 0);
    drawImage(bottomLeft, 0, h - bottomLeft->getHeight());
    drawImage(bottomRight, w - bottomRight->getWidth(),
              h - bottomRight->getHeight());

    popClipArea();
}

void Graphics::drawImageRect(int x, int y, int w, int h,
                             const ImageRect &imgRect)
{
    drawImageRect(x, y, w, h, imgRect.grid[0], imgRect.grid[2], imgRect.grid[6],
                  imgRect.grid[8], imgRect.grid[1], imgRect.grid[5],
                  imgRect.grid[7], imgRect.grid[3], imgRect.grid[4]);
}

int Graphics::getWidth()
{
    return mWidth;
}

int Graphics::getHeight()
{
    return mHeight;
}

void saveScreenshot()
{
    static unsigned int screenshotCount = 0;

    SDL_Surface *screenshot = graphics->getScreenshot();

    // Search for an unused screenshot name
    std::stringstream filenameSuffix;
    std::stringstream filename;
    std::fstream testExists;
    bool found = false;

    do {
        screenshotCount++;
        filename.str("");
        filenameSuffix.str("");
        filename << PHYSFS_getUserDir();
#if (defined __USE_UNIX98 || defined __FreeBSD__)
        filenameSuffix << ".aethyra/";
#elif defined __APPLE__
        filenameSuffix << "Desktop/";
#endif
        filenameSuffix << "Ae_Screenshot_" << screenshotCount << ".png";
        filename << filenameSuffix.str();
        testExists.open(filename.str().c_str(), std::ios::in);
        found = !testExists.is_open();
        testExists.close();
    } while (!found);

    const bool success = ImageWriter::writePNG(screenshot, filename.str());

    if (success)
    {
        if (chatWindow)
            chatWindow->chatLog(strprintf(_("Screenshot saved to %s"),
                                filename.str().c_str()), BY_LOGGER);
    }
    else
    {
        if (chatWindow)
            chatWindow->chatLog(_("Saving screenshot failed!"), BY_LOGGER);

        logger->log("Error: could not save screenshot.");
    }

    SDL_FreeSurface(screenshot);
}
