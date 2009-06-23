/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *  Copyright (C) 2009  Aethyra Development Team
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

#include <guichan/color.hpp>
#include <guichan/exception.hpp>

#include "graphics.h"
#include "truetypefont.h"

#include "../../core/image/image.h"

#define CACHE_SIZE 256

class TextChunk
{
    public:
        TextChunk(const std::string &text, const gcn::Color &color) :
            img(NULL), text(text), color(color)
        {
        }

        ~TextChunk()
        {
            delete img;
        }

        bool operator==(const std::string &str) const
        {
            return (str == text);
        }

        bool operator==(const TextChunk &chunk) const
        {
            return (chunk.text == text && chunk.color == color);
        }

        void generate(TTF_Font *font)
        {
            SDL_Color sdlCol;
            sdlCol.b = color.b;
            sdlCol.r = color.r;
            sdlCol.g = color.g;

            SDL_Surface *surface = TTF_RenderUTF8_Blended(
                    font, text.c_str(), sdlCol);

            if (!surface)
            {
                throw "Rendering font to surface failed: " +
                    std::string(TTF_GetError());
            }

            img = Image::load(surface);

            SDL_FreeSurface(surface);
        }

        Image *img;
        std::string text;
        gcn::Color color;
};

typedef std::list<TextChunk>::iterator CacheIterator;

static int fontCounter;

TrueTypeFont::TrueTypeFont(const std::string &filename, int size, int style)
{
    if (fontCounter == 0 && TTF_Init() == -1)
    {
        throw GCN_EXCEPTION("Unable to initialize SDL_ttf: " +
            std::string(TTF_GetError()));
    }

    ++fontCounter;
    mFont = TTF_OpenFont(filename.c_str(), size);

    if (!mFont)
    {
        throw GCN_EXCEPTION("SDLTrueTypeFont::SDLTrueTypeFont: " +
            std::string(TTF_GetError()));
    }

    TTF_SetFontStyle (mFont, style);
}

TrueTypeFont::~TrueTypeFont()
{
    TTF_CloseFont(mFont);
    --fontCounter;

    if (fontCounter == 0)
        TTF_Quit();
}

TrueTypeFont *TrueTypeFont::load(const std::string &fileName, int fontSize,
                                 int style)
{
    return new TrueTypeFont(fileName, fontSize, style);
}

void TrueTypeFont::drawString(gcn::Graphics *graphics,
                              const std::string &text,
                              int x, int y)
{
    if (text.empty())
        return;

    Graphics *g = dynamic_cast<Graphics *>(graphics);

    if (!g)
        throw "Not a valid graphics object!";

    gcn::Color col = g->getColor();
    const float alpha = col.a / 255.0f;

    /* The alpha value is ignored at string generation so avoid caching the
     * same text with different alpha values.
     */
    col.a = 255;

    TextChunk chunk(text, col);

    bool found = false;

    for (CacheIterator i = cache.begin(); i != cache.end(); i++)
    {
        if (chunk == (*i))
        {
            // Raise priority: move it to front
            cache.splice(cache.begin(), cache, i);
            found = true;
            break;
        }
    }

    // Surface not found
    if (!found)
    {
        if (cache.size() >= CACHE_SIZE)
            cache.pop_back();

        cache.push_front(chunk);
        cache.front().generate(mFont);
    }

    cache.front().img->setAlpha(alpha);
    g->drawImage(cache.front().img, x, y);
}

int TrueTypeFont::getWidth(const std::string& text) const
{
    for (CacheIterator i = cache.begin(); i != cache.end(); i++)
    {
        if ((*i) == text)
        {
            // Raise priority: move it to front
            // Assumption is that TTF::draw will be called next
            cache.splice(cache.begin(), cache, i);
            return i->img->getWidth();
        }
    }

    int w, h;
    TTF_SizeUTF8(mFont, text.c_str(), &w, &h);
    return w;
}

int TrueTypeFont::getHeight() const
{
    return TTF_FontHeight(mFont);
}
