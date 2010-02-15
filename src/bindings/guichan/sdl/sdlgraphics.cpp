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

#include <guichan/exception.hpp>

#include <SDL/SDL_gfxPrimitives.h>

#include "sdlgraphics.h"

#include "../../../core/log.h"

#include "../../../core/image/image.h"
#include "../../../core/image/imageloader.h"

SDLGraphics::SDLGraphics()
{
    mTarget = NULL;
}

SDLGraphics::~SDLGraphics()
{
    _endDraw();
}

void SDLGraphics::_beginDraw()
{
    gcn::Rectangle area;
    area.x = 0;
    area.y = 0;
    area.width = mTarget->w;
    area.height = mTarget->h;
    pushClipArea(area);
}

void SDLGraphics::_endDraw()
{
    popClipArea();
}

bool SDLGraphics::pushClipArea(gcn::Rectangle area)
{
    SDL_Rect rect;
    bool result = Graphics::pushClipArea(area);

    const gcn::ClipRectangle& carea = mClipStack.top();
    rect.x = carea.x;
    rect.y = carea.y;
    rect.w = carea.width;
    rect.h = carea.height;

    SDL_SetClipRect(mTarget, &rect);

    return result;
}

void SDLGraphics::popClipArea()
{
    Graphics::popClipArea();

    if (mClipStack.empty())
        return;

    const gcn::ClipRectangle& carea = mClipStack.top();
    SDL_Rect rect;
    rect.x = carea.x;
    rect.y = carea.y;
    rect.w = carea.width;
    rect.h = carea.height;

    SDL_SetClipRect(mTarget, &rect);
}

void SDLGraphics::drawPoint(int x, int y)
{
    if (mClipStack.empty())
        throw GCN_EXCEPTION("Clip stack is empty, perhaps you called a draw "
                            "funtion outside of _beginDraw() and _endDraw()?");

    const gcn::ClipRectangle& top = mClipStack.top();
        
    x += top.xOffset;
    y += top.yOffset;

    pixelRGBA(mTarget, x, y, mColor.r, mColor.g, mColor.b, mColor.a);
}

void SDLGraphics::drawLine(int x1, int y1, int x2, int y2)
{
    if (mClipStack.empty())
        throw GCN_EXCEPTION("Clip stack is empty, perhaps you called a draw "
                            "funtion outside of _beginDraw() and _endDraw()?");

    const gcn::ClipRectangle& top = mClipStack.top();

    x1 += top.xOffset;
    y1 += top.yOffset;
    x2 += top.xOffset;
    y2 += top.yOffset;

    if (x1 == x2)
        vlineRGBA(mTarget, x1, y1, y2, mColor.r, mColor.g, mColor.b, mColor.a);
    else if (y1 == y2)
        hlineRGBA(mTarget, x1, x2, y2, mColor.r, mColor.g, mColor.b, mColor.a);
    else
        lineRGBA(mTarget, x1, y1, x2, y2, mColor.r, mColor.g, mColor.b, mColor.a);
}

void SDLGraphics::drawRectangle(const gcn::Rectangle& rectangle)
{
    if (mClipStack.empty())
        throw GCN_EXCEPTION("Clip stack is empty, perhaps you called a draw "
                            "funtion outside of _beginDraw() and _endDraw()?");

    const gcn::ClipRectangle& top = mClipStack.top();
        
    gcn::Rectangle area = rectangle;
    area.x += top.xOffset;
    area.y += top.yOffset;

    if(!area.isIntersecting(top))
        return;

    rectangleRGBA(mTarget, area.x, area.y, area.x + area.width,
                  area.y + area.height, mColor.r, mColor.g, mColor.b, mColor.a);
}

void SDLGraphics::fillRectangle(const gcn::Rectangle& rectangle)
{
    if (mClipStack.empty())
        throw GCN_EXCEPTION("Clip stack is empty, perhaps you called a draw "
                            "funtion outside of _beginDraw() and _endDraw()?");

    const gcn::ClipRectangle& top = mClipStack.top();
        
    gcn::Rectangle area = rectangle;
    area.x += top.xOffset;
    area.y += top.yOffset;

    if(!area.isIntersecting(top))
        return;

    boxRGBA(mTarget, area.x, area.y, area.x + area.width,
            area.y + area.height, mColor.r, mColor.g, mColor.b, mColor.a);
}

void SDLGraphics::setColor(const gcn::Color& color)
{
    mColor = color;
    mAlpha = color.a != 255;
}

bool SDLGraphics::setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel)
{
    Graphics::setVideoMode(w, h, bpp, fs, hwaccel);

    int displayFlags = SDL_ANYFORMAT;

    if (fs)
        displayFlags |= SDL_FULLSCREEN;

    if (hwaccel)
        displayFlags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
    else
        displayFlags |= SDL_SWSURFACE;

    SDL_Surface* target = SDL_SetVideoMode(w, h, bpp, displayFlags);

    if (!target)
        return false;

    setTarget(target);

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

    return true;
}

void SDLGraphics::drawImage(const gcn::Image *image, int srcX, int srcY,
                            int dstX, int dstY, int width, int height)
{
    const ProxyImage *srcImage = dynamic_cast<const ProxyImage* >(image);
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
    if (!mTarget || !image || !image->mImage)
        return false;

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

    return !(SDL_BlitSurface(image->mImage, &srcRect, mTarget, &dstRect) < 0);
}

void SDLGraphics::drawImagePattern(Image *image, int x, int y, int w, int h)
{
    // Check that preconditions for blitting are met.
    if (!mTarget || !image || !image->mImage)
        return;

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

            SDL_BlitSurface(image->mImage, &srcRect, mTarget, &dstRect);
        }
    }
}

void SDLGraphics::updateScreen()
{
    SDL_Flip(mTarget);
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

    SDL_Surface *screenshot = SDL_CreateRGBSurface(SDL_SWSURFACE, mTarget->w,
                                                   mTarget->h, 24, rmask, gmask,
                                                   bmask, amask);

    SDL_BlitSurface(mTarget, NULL, screenshot, NULL);

    return screenshot;
}
