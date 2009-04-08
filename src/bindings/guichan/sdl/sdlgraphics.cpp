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

#include "sdlgraphics.h"

#include "../../../log.h"

#include "../../../resources/image.h"
#include "../../../resources/imageloader.h"

SDLGraphics::SDLGraphics()
{
    mScreen = NULL;
}

SDLGraphics::~SDLGraphics()
{
    _endDraw();
}

bool SDLGraphics::setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel)
{
    logger->log("Setting video mode %dx%d %s", w, h, fs ? "fullscreen" :
                "windowed");

    logger->log("Bits per pixel: %d", bpp);

    int displayFlags = SDL_ANYFORMAT;

    mFullscreen = fs;
    mHWAccel = hwaccel;

    if (fs)
        displayFlags |= SDL_FULLSCREEN;

    if (hwaccel)
        displayFlags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
    else
        displayFlags |= SDL_SWSURFACE;

    mScreen = SDL_SetVideoMode(w, h, bpp, displayFlags);

    if (!mScreen)
        return false;

    char videoDriverName[64];

    if (SDL_VideoDriverName(videoDriverName, 64))
        logger->log("Using video driver: %s", videoDriverName);
    else
        logger->log("Using video driver: unknown");

    const SDL_VideoInfo *vi = SDL_GetVideoInfo();

    logger->log("Possible to create hardware surfaces: %s",
            ((vi->hw_available) ? "yes" : "no"));
    logger->log("Window manager available: %s",
            ((vi->wm_available) ? "yes" : "no"));
    logger->log("Accelerated hardware to hardware blits: %s",
            ((vi->blit_hw) ? "yes" : "no"));
    logger->log("Accelerated hardware to hardware colorkey blits: %s",
            ((vi->blit_hw_CC) ? "yes" : "no"));
    logger->log("Accelerated hardware to hardware alpha blits: %s",
            ((vi->blit_hw_A) ? "yes" : "no"));
    logger->log("Accelerated software to hardware blits: %s",
            ((vi->blit_sw) ? "yes" : "no"));
    logger->log("Accelerated software to hardware colorkey blits: %s",
            ((vi->blit_sw_CC) ? "yes" : "no"));
    logger->log("Accelerated software to hardware alpha blits: %s",
            ((vi->blit_sw_A) ? "yes" : "no"));
    logger->log("Accelerated color fills: %s",
            ((vi->blit_fill) ? "yes" : "no"));
    logger->log("Available video memory: %d", vi->video_mem);

    setTarget(mScreen);

    return true;
}

void SDLGraphics::drawImage(gcn::Image const *image, int srcX, int srcY,
                            int dstX, int dstY, int width, int height)
{
    ProxyImage const *srcImage = dynamic_cast< ProxyImage const * >(image);
    assert(srcImage);
    drawImage(srcImage->getImage(), srcX, srcY, dstX, dstY, width, height, true);
}

bool SDLGraphics::drawImage(Image *image, int x, int y)
{
    return drawImage(image, 0, 0, x, y, image->getWidth(), image->getHeight());
}

bool SDLGraphics::drawImage(Image *image, int srcX, int srcY, int dstX, int dstY,
                           int width, int height, bool)
{
    // Check that preconditions for blitting are met.
    if (!mScreen || !image || !image->mImage) return false;

    dstX += mClipStack.top().xOffset;
    dstY += mClipStack.top().yOffset;

    srcX += image->mBounds.x;
    srcY += image->mBounds.y;

    SDL_Rect dstRect;
    SDL_Rect srcRect;
    dstRect.x = dstX; dstRect.y = dstY;
    srcRect.x = srcX; srcRect.y = srcY;
    srcRect.w = width;
    srcRect.h = height;

    return !(SDL_BlitSurface(image->mImage, &srcRect, mScreen, &dstRect) < 0);
}

void SDLGraphics::drawImagePattern(Image *image, int x, int y, int w, int h)
{
    // Check that preconditions for blitting are met.
    if (!mScreen || !image || !image->mImage) return;

    const int iw = image->getWidth();
    const int ih = image->getHeight();
 
    if (iw == 0 || ih == 0) return;                         

    for (int py = 0; py < h; py += ih)     // Y position on pattern plane
    {
        int dh = (py + ih >= h) ? h - py : ih;
        int srcY = image->mBounds.y;
        int dstY = y + py + mClipStack.top().yOffset;

        for (int px = 0; px < w; px += iw) // X position on pattern plane  
        {
            int dw = (px + iw >= w) ? w - px : iw;
            int srcX = image->mBounds.x;
            int dstX = x + px + mClipStack.top().xOffset;

            SDL_Rect dstRect;
            SDL_Rect srcRect;
            dstRect.x = dstX; dstRect.y = dstY;
            srcRect.x = srcX; srcRect.y = srcY;
            srcRect.w = dw;   srcRect.h = dh;

            SDL_BlitSurface(image->mImage, &srcRect, mScreen, &dstRect);
        }
    }
}

void SDLGraphics::updateScreen()
{
    SDL_Flip(mScreen);
}

SDL_Surface* SDLGraphics::getScreenshot()
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int rmask = 0xff000000;
    int gmask = 0x00ff0000;
    int bmask = 0x0000ff00;
#else
    int rmask = 0x000000ff;
    int gmask = 0x0000ff00;
    int bmask = 0x00ff0000;
#endif
    int amask = 0x00000000;

    SDL_Surface *screenshot = SDL_CreateRGBSurface(SDL_SWSURFACE, mScreen->w,
                                                   mScreen->h, 24, rmask, gmask,
                                                   bmask, amask);

    SDL_BlitSurface(mScreen, NULL, screenshot, NULL);

    return screenshot;
}
