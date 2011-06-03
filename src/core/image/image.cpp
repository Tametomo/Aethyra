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

#include <SDL_image.h>
#include <SDL_rotozoom.h>

#include <guichan/color.hpp>

#include "dye.h"
#include "image.h"

#include "../log.h"

#include "../utils/dtor.h"

#ifdef USE_OPENGL
#include "../../bindings/guichan/opengl/openglgraphics.h"

bool Image::mUseOpenGL = false;
int Image::mTextureType = 0;
int Image::mTextureSize = 0;
#endif

Image::Image(SDL_Surface *image, uint8_t* alphas):
    mStoredAlpha(alphas),
#ifdef USE_OPENGL
    mGLImage(0),
#endif
    mImage(image),
    mAlpha(1.0f)
{
    mBounds.x = 0;
    mBounds.y = 0;
    mBounds.w = mImage->w;
    mBounds.h = mImage->h;
}

#ifdef USE_OPENGL
Image::Image(const GLuint &glimage, const int width, const int height,
             const int texWidth, const int texHeight):
    mStoredAlpha(NULL),
    mGLImage(glimage),
    mTexWidth(texWidth),
    mTexHeight(texHeight),
    mImage(0),
    mAlpha(1.0)
{
    mBounds.x = 0;
    mBounds.y = 0;
    mBounds.w = width;
    mBounds.h = height;
}
#endif

Image::~Image()
{
    unload();
}

Resource *Image::load(void *buffer, unsigned bufferSize)
{
    // Load the raw file data from the buffer in an RWops structure
    SDL_RWops *rw = SDL_RWFromMem(buffer, bufferSize);
    SDL_Surface *tmpImage = IMG_Load_RW(rw, 1);

    if (!tmpImage)
    {
        logger->log("Error, image load failed: %s", IMG_GetError());
        return NULL;
    }

    Image *image = load(tmpImage);

    SDL_FreeSurface(tmpImage);
    return image;
}

Resource *Image::load(void *buffer, unsigned bufferSize, const Dye &dye)
{
    SDL_RWops *rw = SDL_RWFromMem(buffer, bufferSize);
    SDL_Surface *tmpImage = IMG_Load_RW(rw, 1);

    if (!tmpImage)
    {
        logger->log("Error, image load failed: %s", IMG_GetError());
        return NULL;
    }

    SDL_PixelFormat rgba;
    rgba.palette = NULL;
    rgba.BitsPerPixel = 32;
    rgba.BytesPerPixel = 4;
    rgba.Rmask = 0xFF000000; rgba.Rloss = 0; rgba.Rshift = 24;
    rgba.Gmask = 0x00FF0000; rgba.Gloss = 0; rgba.Gshift = 16;
    rgba.Bmask = 0x0000FF00; rgba.Bloss = 0; rgba.Bshift = 8;
    rgba.Amask = 0x000000FF; rgba.Aloss = 0; rgba.Ashift = 0;
    rgba.colorkey = 0;
    rgba.alpha = 255;

    SDL_Surface *surf = SDL_ConvertSurface(tmpImage, &rgba, SDL_SWSURFACE);
    SDL_FreeSurface(tmpImage);

    uint32_t *pixels = static_cast<uint32_t* >(surf->pixels);
    for (uint32_t *p_end = pixels + surf->w * surf->h; pixels != p_end; ++pixels)
    {
        int alpha = *pixels & 255;
        if (!alpha)
            continue;
        gcn::Color *v = new gcn::Color();
        v->r = (*pixels >> 24) & 255;
        v->g = (*pixels >> 16) & 255;
        v->b = (*pixels >> 8 ) & 255;
        dye.update(v);
        *pixels = (v->r << 24) | (v->g << 16) | (v->b << 8) | alpha;
        destroy(v);
    }

    Image *image = load(surf);
    SDL_FreeSurface(surf);
    return image;
}

Resource *Image::resize(Image *image, const int width, const int height)
{
    return image->resize(width, height);
}

Image *Image::load(SDL_Surface *tmpImage)
{
#ifdef USE_OPENGL
    if (mUseOpenGL)
    {
        // Flush current error flag.
        glGetError();

        int width = tmpImage->w;
        int height = tmpImage->h;
        int realWidth = powerOfTwo(width);
        int realHeight = powerOfTwo(height);

        if (realWidth < width || realHeight < height)
        {
            logger->log("Warning: image too large, cropping to %dx%d texture!",
                    tmpImage->w, tmpImage->h);
        }

        // Make sure the alpha channel is not used, but copied to destination
        SDL_SetAlpha(tmpImage, 0, SDL_ALPHA_OPAQUE);

        // Determine 32-bit masks based on byte order
        uint32_t rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
#endif

        SDL_Surface *oldImage = tmpImage;
        tmpImage = SDL_CreateRGBSurface(SDL_SWSURFACE, realWidth, realHeight,
                                        32, rmask, gmask, bmask, amask);

        if (!tmpImage)
        {
            logger->log("Error, image convert failed: out of memory");
            return NULL;
        }

        SDL_BlitSurface(oldImage, NULL, tmpImage, NULL);

        GLuint texture;
        glGenTextures(1, &texture);
        OpenGLGraphics::bindTexture(mTextureType, texture);

        if (SDL_MUSTLOCK(tmpImage))
            SDL_LockSurface(tmpImage);

        glTexImage2D(mTextureType, 0, 4, tmpImage->w, tmpImage->h,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, tmpImage->pixels);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexParameteri(mTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(mTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (SDL_MUSTLOCK(tmpImage))
            SDL_UnlockSurface(tmpImage);

        SDL_FreeSurface(tmpImage);

        GLenum error = glGetError();
        if (error)
        {
            std::string errmsg = "Unknown error";
            switch (error)
            {
                case GL_INVALID_ENUM:
                    errmsg = "GL_INVALID_ENUM";
                    break;
                case GL_INVALID_VALUE:
                    errmsg = "GL_INVALID_VALUE";
                    break;
                case GL_INVALID_OPERATION:
                    errmsg = "GL_INVALID_OPERATION";
                    break;
                case GL_STACK_OVERFLOW:
                    errmsg = "GL_STACK_OVERFLOW";
                    break;
                case GL_STACK_UNDERFLOW:
                    errmsg = "GL_STACK_UNDERFLOW";
                    break;
                case GL_OUT_OF_MEMORY:
                    errmsg = "GL_OUT_OF_MEMORY";
                    break;
            }
            logger->log("Error: Image GL import failed: %s", errmsg.c_str());
            return NULL;
        }

        return new Image(texture, width, height, realWidth, realHeight);
    }
#endif

    bool hasAlpha = false;

    uint8_t* imageAlphas = new uint8_t[tmpImage->w * tmpImage->h];
    if (tmpImage->format->BitsPerPixel == 32)
    {
        // Figure out whether the image uses its alpha layer
        for (int i = 0; i < tmpImage->w * tmpImage->h; ++i)
        {
            uint8_t r, g, b, a;
            SDL_GetRGBA(((uint32_t*) tmpImage->pixels)[i],
                          tmpImage->format, &r, &g, &b, &a);

            imageAlphas[i] = a;

            if (a != 255)
                hasAlpha = true;
        }
    }

    SDL_Surface *image;

    // Convert the surface to the current display format
    if (hasAlpha)
        image = SDL_DisplayFormatAlpha(tmpImage);
    else
        image = SDL_DisplayFormat(tmpImage);

    if (!image)
    {
        logger->log("Error: Image convert failed.");
        delete [] imageAlphas;
        return NULL;
    }

    return new Image(image, imageAlphas);
}

void Image::unload()
{
    mLoaded = false;

    if (mImage)
    {
        // Free the image surface.
        SDL_FreeSurface(mImage);
        mImage = NULL;
        delete [] mStoredAlpha;
    }

#ifdef USE_OPENGL
    if (mGLImage)
    {
        glDeleteTextures(1, &mGLImage);
        mGLImage = 0;
    }
#endif
}

Image* Image::resize(const int width, const int height)
{
    // Don't return anything for bad height or width values
    if (width <= 0 || height <= 0)
        return NULL;

    // Don't scale at all if the image would get the same dimensions
    if (width == getWidth() && height == getHeight())
        return this;
    
    if (mImage)
    {
        SDL_Surface* scaledSurface = NULL;
        uint8_t* imageAlphas = NULL;

        const double scaleX = (double) width / (double) getWidth();
        const double scaleY = (double) height / (double) getHeight();

        scaledSurface = zoomSurface(mImage, scaleX, scaleY, 1);

        imageAlphas = new uint8_t[scaledSurface->w * scaledSurface->h];
        if (scaledSurface->format->BitsPerPixel == 32)
        {
            // Recalculate the alpha layers
            for (int i = 0; i < scaledSurface->w * scaledSurface->h; ++i)
            {
                uint8_t r, g, b, a;
                SDL_GetRGBA(((uint32_t*) scaledSurface->pixels)[i],
                              scaledSurface->format, &r, &g, &b, &a);

                imageAlphas[i] = a;
            }
        }

        return new Image(scaledSurface, imageAlphas);
    }

    return this;
}

SubImage *Image::getSubImage(const int x, const int y, const int width,
                             const int height)
{
    // Create a new clipped sub-image
#ifdef USE_OPENGL
    if (mUseOpenGL)
        return new SubImage(this, mGLImage, x, y, width, height,
                            mTexWidth, mTexHeight);
#endif

    return new SubImage(this, mImage, x, y, width, height);
}

void Image::setAlpha(float alpha)
{
    if (mAlpha == alpha)
        return;

    mAlpha = alpha;

#ifdef USE_OPENGL
    if (mImage && !mUseOpenGL)
#else
    if (mImage)
#endif
    {
        if (SDL_MUSTLOCK(mImage))
            SDL_LockSurface(mImage);

        // Set the alpha value this image is drawn at, pixel by pixel
        for (int i = 0; i < mImage->w * mImage->h; i++)
        {
            uint8_t r, g, b, a;
            SDL_GetRGBA(((uint32_t*) mImage->pixels)[i], mImage->format, &r, 
                                                         &g, &b, &a);

            a = (uint8_t) (mStoredAlpha[i] * mAlpha);

            ((uint32_t *)(mImage->pixels))[i] = SDL_MapRGBA(mImage->format, r,
                                                            g, b, a);
        }

        if (SDL_MUSTLOCK(mImage))
            SDL_UnlockSurface(mImage);
    }
}

Image* Image::merge(Image* image, const int x, const int y)
{
    SDL_Surface* surface = new SDL_Surface(*(image->mImage));

    uint32_t surface_pix, cur_pix;
    uint8_t r, g, b, a, p_r, p_g, p_b, p_a;
    double f_a, f_ca, f_pa;
    SDL_PixelFormat *current_fmt = mImage->format;
    SDL_PixelFormat *surface_fmt = surface->format;
    int current_offset, surface_offset;
    int offsetX = 0, offsetY = 0;

    if (SDL_MUSTLOCK(mImage))
    {
        SDL_LockSurface(surface);
        SDL_LockSurface(mImage);
    }

    // for each pixel lines of a source image
    for (offsetY = (y > 0 ? 0 : -y); offsetY < image->getHeight() &&
                    y + offsetY < getHeight(); offsetY++)
    {
        for (offsetX = (x > 0 ? 0 : -x); offsetX < image->getWidth() &&
                        x + offsetX < getWidth(); offsetX++)
        {
            // Computing offset on both images
            current_offset = (y + offsetY) * getWidth() + x + offsetX;
            surface_offset = offsetY * surface->w + offsetX;

            // Retrieving a pixel to merge
            surface_pix = ((uint32_t*) surface->pixels)[surface_offset];
            cur_pix = ((uint32_t*) mImage->pixels)[current_offset];

            // Retreiving each channel of the pixel using pixel format
            r = (uint8_t)(((surface_pix & surface_fmt->Rmask) >> 
                            surface_fmt->Rshift) << surface_fmt->Rloss);
            g = (uint8_t)(((surface_pix & surface_fmt->Gmask) >>
                            surface_fmt->Gshift) << surface_fmt->Gloss);
            b = (uint8_t)(((surface_pix & surface_fmt->Bmask) >>
                            surface_fmt->Bshift) << surface_fmt->Bloss);
            a = (uint8_t)(((surface_pix & surface_fmt->Amask) >>
                            surface_fmt->Ashift) << surface_fmt->Aloss);

            // Retreiving previous alpha value
            p_a = (uint8_t)(((cur_pix & current_fmt->Amask) >>
                            current_fmt->Ashift) << current_fmt->Aloss);

            // new pixel with no alpha or nothing on previous pixel
            if (a == SDL_ALPHA_OPAQUE || (p_a == 0 && a > 0))
                ((uint32_t *)(surface->pixels))[current_offset] = 
                    SDL_MapRGBA(current_fmt, r, g, b, a);
            else if (a > 0) 
            { // alpha is lower => merge color with previous value
                f_a = (double) a / 255.0;
                f_ca = 1.0 - f_a;
                f_pa = (double) p_a / 255.0;
                p_r = (uint8_t)(((cur_pix & current_fmt->Rmask) >> 
                                  current_fmt->Rshift) << current_fmt->Rloss);
                p_g = (uint8_t)(((cur_pix & current_fmt->Gmask) >>
                                  current_fmt->Gshift) << current_fmt->Gloss);
                p_b = (uint8_t)(((cur_pix & current_fmt->Bmask) >>
                                  current_fmt->Bshift) << current_fmt->Bloss);
                r = (uint8_t)((double) p_r * f_ca * f_pa + (double)r * f_a);
                g = (uint8_t)((double) p_g * f_ca * f_pa + (double)g * f_a);
                b = (uint8_t)((double) p_b * f_ca * f_pa + (double)b * f_a);
                a = (a > p_a ? a : p_a);
               ((uint32_t *)(surface->pixels))[current_offset] =
                   SDL_MapRGBA(current_fmt, r, g, b, a);
            }
        }
    }

    if (SDL_MUSTLOCK(mImage))
    {
        SDL_UnlockSurface(surface);
        SDL_UnlockSurface(mImage);
    }

    Image* newImage = new Image(surface);

    return newImage;
}

float Image::getAlpha() const
{
    return mAlpha;
}

#ifdef USE_OPENGL
void Image::setLoadAsOpenGL(const bool useOpenGL)
{
    Image::mUseOpenGL = useOpenGL;
}

int Image::powerOfTwo(const int input)
{
    int value = 1;
    if (mTextureType == GL_TEXTURE_2D)
    {
        while (value < input && value < mTextureSize)
            value <<= 1;
    }
    else
        value = input;

    return value >= mTextureSize ? mTextureSize : value;
}
#endif

//============================================================================
// SubImage Class
//============================================================================

SubImage::SubImage(Image *parent, SDL_Surface *image, const int x, const int y,
                   const int width, const int height):
    Image(image),
    mParent(parent)
{
    mParent->incRef();

    // Set up the rectangle.
    mBounds.x = x;
    mBounds.y = y;
    mBounds.w = width;
    mBounds.h = height;
}

#ifdef USE_OPENGL
SubImage::SubImage(Image *parent, const GLuint &image, const int x, const int y,
                   const int width, const int height, const int texWidth,
                   const int texHeight):
    Image(image, width, height, texWidth, texHeight),
    mParent(parent)
{
    mParent->incRef();

    // Set up the rectangle.
    mBounds.x = x;
    mBounds.y = y;
    mBounds.w = width;
    mBounds.h = height;
}
#endif

SubImage::~SubImage()
{
    // Avoid destruction of the image
    mImage = NULL;
#ifdef USE_OPENGL
    mGLImage = 0;
#endif
    mParent->decRef();
}

SubImage *SubImage::getSubImage(const int x, const int y, const int w,
                                const int h)
{
    return mParent->getSubImage(mBounds.x + x, mBounds.y + y, w, h);
}

void SubImage::setAlpha(float alpha)
{
    if (mAlpha == alpha)
        return;

    mAlpha = alpha;

#ifdef USE_OPENGL
    if (mImage && !mUseOpenGL)
#else
    if (mImage)
#endif
    {
        if (SDL_MUSTLOCK(mImage))
            SDL_LockSurface(mImage);

        // Set the alpha value this image is drawn at, pixel by pixel
        for (int offsetY = 0; offsetY < getHeight() &&
            (offsetY + mBounds.y) < mParent->getHeight(); offsetY++)
        {
            for (int offsetX = 0; offsetX < getWidth() &&
                (offsetX + mBounds.x) < mParent->getWidth(); offsetX++)
            {
                int i = (offsetY + mBounds.y) * mParent->getWidth() + offsetX +
                        mBounds.x;
                uint8_t r, g, b, a;
                SDL_GetRGBA(((uint32_t*) mImage->pixels)[i], mImage->format, &r, 
                            &g, &b, &a);

                a = (uint8_t) (mParent->mStoredAlpha[i] * mAlpha);

                ((uint32_t *)(mImage->pixels))[i] = SDL_MapRGBA(mImage->format,
                                                                r, g, b, a);
            }
        }
        if (SDL_MUSTLOCK(mImage))
            SDL_UnlockSurface(mImage);
    }
}

