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

#ifdef HAVE_CONFIG_H
#include "../../../../config.h"
#endif

#ifdef USE_OPENGL

#include <cassert>
#include <SDL.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

#include "openglgraphics.h"

#include "../../../core/log.h"

#include "../../../core/image/image.h"

#ifndef GL_TEXTURE_RECTANGLE_ARB
#define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB 0x84F8
#endif

const unsigned int vertexBufSize = 500;

GLuint OpenGLGraphics::mLastImage = NULL;

OpenGLGraphics::OpenGLGraphics():
    mAlpha(false),
    mTexture(false),
    mColorAlpha(false),
    mSync(false)
{
    mFloatTexArray = new GLfloat[vertexBufSize * 4];
    mIntTexArray = new GLint[vertexBufSize * 4];
    mIntVertArray = new GLint[vertexBufSize * 4];
}

OpenGLGraphics::~OpenGLGraphics()
{
    delete[] mFloatTexArray;
    delete[] mIntTexArray;
    delete[] mIntVertArray;
}

void OpenGLGraphics::setSync(bool sync)
{
    mSync = sync;
}

bool OpenGLGraphics::setVideoMode(int w, int h, int bpp, bool fs, bool hwaccel)
{
    Graphics::setVideoMode(w, h, bpp, fs, hwaccel);

    int displayFlags = SDL_ANYFORMAT | SDL_OPENGL;

    if (fs)
        displayFlags |= SDL_FULLSCREEN;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Surface* target = SDL_SetVideoMode(w, h, bpp, displayFlags);

    if (!target)
        return false;

    setTarget(target);

#ifdef __APPLE__
    if (mSync)
    {
        const GLint VBL = 1;
        CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &VBL);
    }
#endif

    // Setup OpenGL
    glViewport(0, 0, w, h);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    int gotDoubleBuffer;
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &gotDoubleBuffer);
    logger->log("Using OpenGL %s double buffering.",
               (gotDoubleBuffer ? "with" : "without"));

    char const *glExtensions = (char const *)glGetString(GL_EXTENSIONS);
    GLint texSize;
    bool rectTex = strstr(glExtensions, "GL_ARB_texture_rectangle");
    if (rectTex)
    {
        Image::mTextureType = GL_TEXTURE_RECTANGLE_ARB;
        glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB, &texSize);
    }
    else
    {
        Image::mTextureType = GL_TEXTURE_2D;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
    }
    Image::mTextureSize = texSize;
    logger->log("OpenGL texture size: %d pixels%s", Image::mTextureSize,
                rectTex ? " (rectangle textures)" : "");

    return true;
}

static inline void drawQuad(Image *image, int srcX, int srcY, int dstX,
                            int dstY, int width, int height)
{
    if (image->getTextureType() == GL_TEXTURE_2D)
    {
        // Find OpenGL normalized texture coordinates.
        const float texX1 = static_cast<float>(srcX) /
                            static_cast<float>(image->getTextureWidth());
        const float texY1 = static_cast<float>(srcY) /
                            static_cast<float>(image->getTextureHeight());
        const float texX2 = static_cast<float>(srcX + width) /
                            static_cast<float>(image->getTextureWidth());
        const float texY2 = static_cast<float>(srcY + height) /
                            static_cast<float>(image->getTextureHeight());

        GLfloat tex[] = { texX1, texY1, texX2, texY1, 
                          texX2, texY2, texX1, texY2 };
        GLint vert[] = { dstX, dstY, dstX + width, dstY,
                         dstX + width, dstY + height, dstX, dstY + height };

        glVertexPointer(2, GL_FLOAT, 0, &vert);
        glTexCoordPointer(2, GL_INT, 0, &tex);
        glDrawArrays(GL_QUADS, 0, 4);
    }
    else
    {
        GLint tex[] = { srcX, srcY, srcX + width, srcY,
                        srcX + width, srcY + height, srcX, srcY + height };
        GLint vert[] = { dstX, dstY, dstX + width, dstY,
                         dstX + width, dstY + height, dstX, dstY + height };

        glVertexPointer(2, GL_INT, 0, &vert);
        glTexCoordPointer(2, GL_INT, 0, &tex);
        glDrawArrays(GL_QUADS, 0, 4);
    }
}

bool OpenGLGraphics::drawImage(Image *image, int srcX, int srcY, int dstX,
                               int dstY, int width, int height, bool useColor)
{
    if (!image)
        return false;

    srcX += image->mBounds.x;
    srcY += image->mBounds.y;

    if (!useColor)
        glColor4f(1.0f, 1.0f, 1.0f, image->mAlpha);

    bindTexture(Image::mTextureType, image->mGLImage);

    setTexturingAndBlending(true);

    drawQuad(image, srcX, srcY, dstX, dstY, width, height);

    if (!useColor)
        glColor4ub(static_cast<GLubyte>(mColor.r), static_cast<GLubyte>(mColor.g),
                   static_cast<GLubyte>(mColor.b), static_cast<GLubyte>(mColor.a));

    return true;
}

void OpenGLGraphics::drawImagePattern(Image *image, int x, int y, int w, int h)
{
    if (!image)
        return;

    const int iw = image->getWidth();
    const int ih = image->getHeight();

    if (iw == 0 || ih == 0)
        return;

    const int srcX = image->mBounds.x;
    const int srcY = image->mBounds.y;

    const float tw = static_cast<float>(image->getTextureWidth());
    const float th = static_cast<float>(image->getTextureHeight());

    unsigned int vp = 0;
    const unsigned int vLimit = vertexBufSize * 4;

    glColor4f(1.0f, 1.0f, 1.0f, image->mAlpha);

    bindTexture(Image::mTextureType, image->mGLImage);

    setTexturingAndBlending(true);

    // Draw a set of textured rectangles
    for (int py = 0; py < h; py += ih)
    {
        const int height = (py + ih >= h) ? h - py : ih;
        const int dstY = y + py;
        for (int px = 0; px < w; px += iw)
        {
            int width = (px + iw >= w) ? w - px : iw;
            int dstX = x + px;

            for (int i = 0; i < 4; i++)
            {
                const int offsetX = ((((i + 1) % 4) < 2) ? 0 : width);
                const int offsetY = ((i < 2) ? 0 : height);
                const int index = vp + (2 * i);

                if (image->getTextureType() == GL_TEXTURE_2D)
                {
                    const float tx = static_cast<float>(srcX + offsetX) / tw;
                    const float ty = static_cast<float>(srcY + offsetY) / th;

                    mFloatTexArray[index] = tx;
                    mFloatTexArray[index + 1] = ty;
                }
                else
                {
                    mIntTexArray[index] = srcX + offsetX;
                    mIntTexArray[index + 1] = srcY + offsetY;
                }

                mIntVertArray[index] = dstX + offsetX;
                mIntVertArray[index + 1] = dstY + offsetY;
            }

            vp += 8;
            if (vp >= vLimit)
            {
                if (image->getTextureType() == GL_TEXTURE_2D)
                    drawQuadArrayfi(vp);
                else
                    drawQuadArrayii(vp);

                vp = 0;
            }
        }
    }

    if (vp > 0)
    {
        if (image->getTextureType() == GL_TEXTURE_2D)
            drawQuadArrayfi(vp);
        else
            drawQuadArrayii(vp);
    }

     glColor4ub(static_cast<GLubyte>(mColor.r),	static_cast<GLubyte>(mColor.g),
                static_cast<GLubyte>(mColor.b), static_cast<GLubyte>(mColor.a));
}

void OpenGLGraphics::updateScreen()
{
    glFlush();
    glFinish();
    SDL_GL_SwapBuffers();
}

void OpenGLGraphics::_beginDraw()
{
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0.0, (double) mTarget->w, (double) mTarget->h, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    pushClipArea(gcn::Rectangle(0, 0, mTarget->w, mTarget->h));
}

void OpenGLGraphics::_endDraw()
{
}

SDL_Surface* OpenGLGraphics::getScreenshot()
{
    int h = mTarget->h;
    int w = mTarget->w;
    GLint pack = 1;

    SDL_Surface *screenshot = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 24,
                                                   0xff0000, 0x00ff00, 0x0000ff,
                                                   0x000000);

    if (SDL_MUSTLOCK(screenshot))
        SDL_LockSurface(screenshot);

    // Grap the pixel buffer and write it to the SDL surface
    glGetIntegerv(GL_PACK_ALIGNMENT, &pack);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, screenshot->pixels);

    // Flip the screenshot, as OpenGL has 0,0 in bottom left
    unsigned int lineSize = 3 * w;
    GLubyte* buf = (GLubyte*)malloc(lineSize);

    for (int i = 0; i < (h / 2); i++)
    {
        GLubyte *top = (GLubyte*)screenshot->pixels + lineSize * i;
        GLubyte *bot = (GLubyte*)screenshot->pixels + lineSize * (h - 1 - i);

        memcpy(buf, top, lineSize);
        memcpy(top, bot, lineSize);
        memcpy(bot, buf, lineSize);
    }

    free(buf);

    glPixelStorei(GL_PACK_ALIGNMENT, pack);

    if (SDL_MUSTLOCK(screenshot))
        SDL_UnlockSurface(screenshot);

    return screenshot;
}

bool OpenGLGraphics::pushClipArea(gcn::Rectangle area)
{
    int transX = 0;
    int transY = 0;

    if (!mClipStack.empty())
    {
        transX = -mClipStack.top().xOffset;
        transY = -mClipStack.top().yOffset;
    }

    bool result = gcn::Graphics::pushClipArea(area);

    transX += mClipStack.top().xOffset;
    transY += mClipStack.top().yOffset;

    glPushMatrix();
    glTranslatef(transX, transY, 0);
    glScissor(mClipStack.top().x,
              mTarget->h - mClipStack.top().y - mClipStack.top().height,
              mClipStack.top().width, mClipStack.top().height);

    return result;
}

void OpenGLGraphics::popClipArea()
{
    gcn::Graphics::popClipArea();

    if (mClipStack.empty())
        return;

    glPopMatrix();
    glScissor(mClipStack.top().x, mTarget->h - mClipStack.top().y - 
              mClipStack.top().height, mClipStack.top().width,
              mClipStack.top().height);
}

void OpenGLGraphics::setColor(const gcn::Color& color)
{
    mColor = color;
    glColor4ub(color.r, color.g, color.b, color.a);

    mColorAlpha = (color.a != 255);
}

void OpenGLGraphics::drawPoint(int x, int y)
{
    setTexturingAndBlending(false);

    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

void OpenGLGraphics::drawLine(int x1, int y1, int x2, int y2)
{
    setTexturingAndBlending(false);

    glBegin(GL_LINES);
    glVertex2f(x1 + 0.5f, y1 + 0.5f);
    glVertex2f(x2 + 0.5f, y2 + 0.5f);
    glEnd();

    glBegin(GL_POINTS);
    glVertex2f(x2 + 0.5f, y2 + 0.5f);
    glEnd();
}

void OpenGLGraphics::drawRectangle(const gcn::Rectangle& rect)
{
    drawRectangle(rect, false);
}

void OpenGLGraphics::fillRectangle(const gcn::Rectangle& rect)
{
    drawRectangle(rect, true);
}

void OpenGLGraphics::setTargetPlane(int width, int height)
{
}

void OpenGLGraphics::setTexturingAndBlending(bool enable)
{
    if (enable)
    {
        if (!mTexture)
        {
            glEnable(Image::mTextureType);
            mTexture = true;
        }

        if (!mAlpha)
        {
            glEnable(GL_BLEND);
            mAlpha = true;
        }
    }
    else
    {
        mLastImage = NULL;
        if (mAlpha && !mColorAlpha)
        {
            glDisable(GL_BLEND);
            mAlpha = false;
        }
        else if (!mAlpha && mColorAlpha)
        {
            glEnable(GL_BLEND);
            mAlpha = true;
        }

        if (mTexture)
        {
            glDisable(Image::mTextureType);
            mTexture = false;
        }
    }
}

void OpenGLGraphics::drawRectangle(const gcn::Rectangle& rect, bool filled)
{
    const float offset = filled ? 0 : 0.5f;

    setTexturingAndBlending(false);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    GLfloat vert[] =
    {
        rect.x + offset, rect.y + offset,
        rect.x + rect.width - offset, rect.y + offset,
        rect.x + rect.width - offset, rect.y + rect.height - offset,
        rect.x + offset, rect.y + rect.height - offset
    };

    glVertexPointer(2, GL_FLOAT, 0, &vert);
    glDrawArrays(filled ? GL_QUADS : GL_LINE_LOOP, 0, 4);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void OpenGLGraphics::bindTexture(GLenum target, GLuint texture)
{
    if (mLastImage != texture)
    {
        mLastImage = texture;
        glBindTexture(target, texture);
    }
}

inline void OpenGLGraphics::drawQuadArrayfi(int size)
{
    glVertexPointer(2, GL_INT, 0, mIntVertArray);
    glTexCoordPointer(2, GL_FLOAT, 0, mFloatTexArray);

    glDrawArrays(GL_QUADS, 0, size / 2);
}

inline void OpenGLGraphics::drawQuadArrayii(int size)
{
    glVertexPointer(2, GL_INT, 0, mIntVertArray);
    glTexCoordPointer(2, GL_INT, 0, mIntTexArray);

    glDrawArrays(GL_QUADS, 0, size / 2);
}


#endif // USE_OPENGL
