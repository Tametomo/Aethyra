/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
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

#include <algorithm>

#include <guichan/graphics.hpp>
#include <guichan/font.hpp>

#include "richtextbox.h"

#include "../palette.h"

#include "../handlers/linkhandler.h"

RichTextBox::RichTextBox(unsigned int mode, bool opaque):
    gcn::Widget(),
    mLaidOutTextValid(false),
    mLinkHandler(NULL),
    mMode(mode),
    mHighMode(UNDERLINE | BACKGROUND),
    mOpaque(opaque),
    mUseLinksAndUserColors(true),
    mSelectedLink(-1),
    mMaxRows(0)
{
    setFocusable(false);
    addMouseListener(this);
    addWidgetListener(this);
}

RichTextBox::~RichTextBox()
{
    removeWidgetListener(this);
    removeMouseListener(this);
}

void RichTextBox::setLinkHandler(LinkHandler* linkHandler)
{
    mLinkHandler = linkHandler;
}

void RichTextBox::setOpaque(bool opaque)
{
    mOpaque = opaque;
}

void RichTextBox::setHighlightMode(unsigned int highMode)
{
    mHighMode = highMode;
}

void RichTextBox::disableLinksAndUserColors()
{
    mUseLinksAndUserColors = false;
}

void RichTextBox::addRow(const std::string &row)
{
    std::string newRow;
    gcn::Font *font = getFont();

    // Invalidate the cached prewrapped lines
    mLaidOutTextValid = false;

    // Use links and user defined colors
    if (mUseLinksAndUserColors)
    {
        std::string tmp = row;
        std::string::size_type idx1, idx2, idx3;
        HYPERLINK hLink;

        // Check for links in format "@@link|Caption@@"
        idx1 = tmp.find("@@");
        while (idx1 != std::string::npos)
        {
            idx2 = tmp.find("|", idx1);
            idx3 = tmp.find("@@", idx2);

            if (idx2 == std::string::npos || idx3 == std::string::npos)
                break;

            hLink.link = tmp.substr(idx1 + 2, idx2 - (idx1 + 2));
            hLink.caption = tmp.substr(idx2 + 1, idx3 - (idx2 + 1));
            hLink.y1 = mTextRows.size() * font->getHeight();
            hLink.y2 = hLink.y1 + font->getHeight();

            newRow += tmp.substr(0, idx1);

            std::string tmp2 = newRow;
            idx1 = tmp2.find("##");
            while (idx1 != std::string::npos)
            {
                if ((idx1 + 3) != tmp2.size() && tmp2[idx1 + 3] == '#')
                {
                    tmp2.erase(idx1, 1);
                    while (idx1 < tmp2.size() && tmp2[idx1] == '#');
                        idx1++;
                }
                else
                    tmp2.erase(idx1, 3);

                idx1 = tmp2.find("##", idx1);
            }
            hLink.x1 = font->getWidth(tmp2) - 1;
            hLink.x2 = hLink.x1 + font->getWidth(hLink.caption) + 1;

            mLinks.push_back(hLink);

            newRow += "##<" + hLink.caption;

            tmp.erase(0, idx3 + 2);
            if (!tmp.empty())
            {
                newRow += "##>";
            }
            idx1 = tmp.find("@@");
        }

        newRow += tmp;
    }
    // Don't use links and user defined colors
    else
    {
        newRow = row;
    }

    mTextRows.push_back(newRow);
    mQueuedRows.push_back(newRow);

    // Auto size mode
    if (mMode == AUTO_SIZE)
    {
        std::string plain = newRow;
        for (std::string::size_type idx1 = plain.find("##");
             idx1 != std::string::npos; idx1 = plain.find("##"))
        {
            if ((idx1 + 3) < plain.size() && plain[idx1 + 3] == '#')
            {
                plain.erase(idx1, 1);
                while (idx1 < plain.size() && plain[idx1] == '#');
                    idx1++;
                continue;
            }
            plain.erase(idx1, 3);
        }

        // Adjust the RichTextBox size
        int w = font->getWidth(plain);
        if (w > getWidth())
            setWidth(w);
    }
}

void RichTextBox::logic()
{
    gcn::Widget::logic();

    if (!isVisible())
        return;

    if (!mQueuedRows.empty())
    {
        gcn::Font *font = getFont();

        // discard older rows when a row limit has been set
        if (mMaxRows > 0 && !mTextRows.empty())
        {
            while (mTextRows.size() > mMaxRows)
            {
                mTextRows.pop_front();
                for (unsigned int i = 0; i < mLinks.size(); i++)
                {
                    mLinks[i].y1 -= font->getHeight();
                    mLinks[i].y2 -= font->getHeight();

                    if (mLinks[i].y1 < 0)
                        mLinks.erase(mLinks.begin() + i);
                }
            }
        }

        if (mMode == AUTO_WRAP)
        {
            unsigned int y = 0;
            unsigned int nextChar;
            const char *hyphen = "~";
            const int hyphenWidth = font->getWidth(hyphen);
            int x = 0;

            for (TextRowIterator i = mTextRows.begin(); i != mTextRows.end(); i++)
            {
                std::string row = *i;
                for (unsigned int j = 0; j < row.size(); j++)
                {
                    std::string character = row.substr(j, 1);
                    x += font->getWidth(character);
                    nextChar = j + 1;

                    // Wraping between words (at blank spaces)
                    if ((nextChar < row.size()) && (row.at(nextChar) == ' '))
                    {
                        int nextSpacePos = row.find(" ", (nextChar + 1));

                        if (nextSpacePos <= 0)
                            nextSpacePos = row.size() - 1;

                        int nextWordWidth = font->getWidth(row.substr(nextChar,
                                                          (nextSpacePos -
                                                           nextChar)));

                        if ((x + nextWordWidth + 10) > getWidth())
                        {
                            x = 15; // Ident in new line
                            y += 1;
                            j++;
                        }
                    }
                    // Wrapping looong lines (brutal force)
                    else if ((x + 2 * hyphenWidth) > getWidth())
                    {
                        x = 15; // Ident in new line
                        y += 1;
                    }
                }
            }

            setHeight(font->getHeight() * (mTextRows.size() + y));
        }
        else
        {
            setHeight(font->getHeight() * mTextRows.size());
        }

        mQueuedRows.clear();
    }

    if (!mLaidOutTextValid)
        calculateTextLayout();
}

void RichTextBox::clearRows()
{
    mTextRows.clear();
    mQueuedRows.clear();
    mLaidOutTextValid = false;
    mLaidOutText.clear();
    mLinks.clear();
    setWidth(0);
    setHeight(0);
    mSelectedLink = -1;
}

void RichTextBox::sanitizeText(std::string &text)
{
    std::string::size_type start = text.find("##", 0);

    while (start != std::string::npos)
    {
        text.insert(start, "#");

        while (text.at(start) == '#')
            start++;

        start = text.find("##", start);
    }
}

struct MouseOverLink
{
    MouseOverLink(int x, int y) : mX(x),mY(y) { }
    bool operator() (HYPERLINK &link)
    {
        return (mX >= link.x1 && mX < link.x2 &&
                mY >= link.y1 && mY < link.y2);
    }
    int mX, mY;
};

void RichTextBox::mousePressed(gcn::MouseEvent &event)
{
    if (!mLinkHandler) return;
    LinkIterator i = find_if(mLinks.begin(), mLinks.end(),
            MouseOverLink(event.getX(), event.getY()));

    if (i != mLinks.end())
        mLinkHandler->handleLink(i->link);
}

void RichTextBox::mouseMoved(gcn::MouseEvent &event)
{
    LinkIterator i = find_if(mLinks.begin(), mLinks.end(),
                             MouseOverLink(event.getX(), event.getY()));

    mSelectedLink = (i != mLinks.end()) ? (i - mLinks.begin()) : -1;
}

void RichTextBox::widgetResized(const gcn::Event &event)
{
    /* Need to lay the text out again, as line-wrapping may
     * have changed.
     */
    mLaidOutTextValid = false;
}

void RichTextBox::draw(gcn::Graphics *graphics)
{
    /* Forge's addition: The remaining code [in calculateTextLayout] get nuts
     *   on infinite loop when getWidth is 0 (how and why
     * it might be so is yet to be explained). For the
     * time being, just protect with a basic sanity check.
     *
     * The infinite loop (well, sort of) is the increasing of x
     * while trying to fit part into the current width
     * (while (x + part.. + 10 > getWidth()), which cannot be false)
     */
    if (!isVisible() || !getWidth() || !getHeight())
       return;

    // End of Forge's addition

    if (mOpaque)
    {
        graphics->setColor(guiPalette->getColor(Palette::BACKGROUND));
        graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
    }

    if (mSelectedLink >= 0)
    {
        if ((mHighMode & BACKGROUND))
        {
            graphics->setColor(guiPalette->getColor(Palette::HIGHLIGHT));
            graphics->fillRectangle(gcn::Rectangle(
                        mLinks[mSelectedLink].x1, mLinks[mSelectedLink].y1,
                        mLinks[mSelectedLink].x2 - mLinks[mSelectedLink].x1,
                        mLinks[mSelectedLink].y2 - mLinks[mSelectedLink].y1));
        }

        if ((mHighMode & UNDERLINE))
        {
            graphics->setColor(guiPalette->getColor(Palette::HYPERLINK));
            graphics->drawLine(mLinks[mSelectedLink].x1, mLinks[mSelectedLink].y2,
                               mLinks[mSelectedLink].x2, mLinks[mSelectedLink].y2);
        }
    }

    gcn::Font *font = getFont();

    for (LaidOutTextIterator i = mLaidOutText.begin(); i != mLaidOutText.end();
         i++)
    {
        switch (i->type)
        {
            case LaidOutPart::HORIZONTAL_RULE:
                graphics->setColor(i->color);
                graphics->drawLine(0, i->y, getWidth(), i->y);
                break;
            default:
                graphics->setColor(i->color);
                font->drawString(graphics, i->text, i->x, i->y);
        }
    }
}

void RichTextBox::fontChanged()
{
    mLaidOutTextValid = false;
}

void RichTextBox::calculateTextLayout()
{
    int x = 0, y = 0;
    int wrappedLines = 0;
    int link = 0;
    gcn::Font *font = getFont();

    mLaidOutText.clear();

    for (TextRowIterator i = mTextRows.begin(); i != mTextRows.end(); i++)
    {
        const gcn::Color textColor = guiPalette->getColor(Palette::TEXT);
        gcn::Color selColor = textColor;
        gcn::Color prevColor = selColor;
        const std::string row = *(i);
        bool wrapped = false;
        x = 0;

        // Check for separator lines
        if (row.find("---", 0) == 0)
        {
            LaidOutPart temp(LaidOutPart::HORIZONTAL_RULE, row, 0,
                             y + (font->getHeight() / 2), textColor);
            mLaidOutText.push_back(temp);
            y += font->getHeight();
            continue;
        }

        // TODO: Check if we must take texture size limits into account here
        // TODO: Check if some of the O(n) calls can be removed
        for (std::string::size_type start = 0, end = std::string::npos;
             start != end; start = end, end = std::string::npos)
        {
            // Wrapped line continuation shall be indented.
            if (wrapped)
            {
                y += font->getHeight();
                x = 15;

                // Clear flag, in case this line contains more than one part
                wrapped = false;
            }

            // "Tokenize" the string at control sequences
            if (mUseLinksAndUserColors)
                end = row.find("##", start + 1);

            if (mUseLinksAndUserColors || (!mUseLinksAndUserColors &&
                (start == 0)))
            {
                // Check for color change in format "##x", x = [L,P,0..9]
                if (row.find("##", start) == start && row.size() > start + 2)
                {
                    const char c = row.at(start + 2);
                    bool valid;
                    const gcn::Color col = guiPalette->getColor(c, valid);

                    if (c == '>')
                        selColor = prevColor;
                    else if (c == '<')
                    {
                        const int size = font->getWidth(mLinks[link].caption) + 1;
                        mLinks[link].x1 = x;
                        mLinks[link].y1 = y;
                        mLinks[link].x2 = mLinks[link].x1 + size;
                        mLinks[link].y2 = y + font->getHeight() - 1;
                        link++;
                        prevColor = selColor;
                        selColor = col;
                    }
                    else if (valid)
                        selColor = col;
                    else
                    {
                        switch (c)
                        {
                            case '1':
                                selColor = RED;
                                break;
                            case '2':
                                selColor = GREEN;
                                break;
                            case '3':
                                selColor = BLUE;
                                break;
                            case '4':
                                selColor = ORANGE;
                                break;
                            case '5':
                                selColor = YELLOW;
                                break;
                            case '6':
                                selColor = PINK;
                                break;
                            case '7':
                                selColor = PURPLE;
                                break;
                            case '8':
                                selColor = GRAY;
                                break;
                            case '9':
                                selColor = BROWN;
                                break;
                            case '0':
                                selColor = BLACK;
                                break;
                            case '#':
                                {
                                    while (end < row.size() &&
                                           row[end] == '#')
                                    {
                                        ++end;
                                    }
                                    start -= 2;
                                } 
                                break;
                            default :
                                selColor = textColor;
                                break;
                        }
                    }
                    start += 3;

                    if (start == row.size())
                        break;
                }
            }

            std::string::size_type len = end == std::string::npos ? end :
                                                                    end - start;
            std::string part = (start == end) ? "" : row.substr(start, len);

            // Auto wrap mode
            if (mMode == AUTO_WRAP && (x + font->getWidth(part) + 10) >
                getWidth() && !part.empty())
            {
                bool forced = false;
                char const *hyphen = "~";
                int hyphenWidth = font->getWidth(hyphen);

                /* FIXME: This code layout makes it easy to crash remote
                   clients by talking garbage. Forged long utf-8 characters
                   will cause either a buffer underflow in substr or an
                   infinite loop in the main loop. */
                do
                {
                    if (!forced)
                        end = row.rfind(' ', end);

                    // Check if we have to (stupidly) force-wrap
                    if (end == std::string::npos || end <= start)
                    {
                        forced = true;
                        end = row.size();
                        x += hyphenWidth; // Account for the wrap-notifier
                        continue;
                    }

                    // Skip to the start of the current character
                    while ((row[end] & 192) == 128)
                        end--;

                    end--; // And then to the last byte of the previous one

                    part = (start == end) ? "" : row.substr(start,
                                                            end - start + 1);
                } while (end > start && (x + font->getWidth(part) + 10) > getWidth());

                if (forced)
                {
                    x -= hyphenWidth; // Remove the wrap-notifier accounting
                    LaidOutPart temp(hyphen, getWidth() - hyphenWidth, y,
                                     selColor);
                    mLaidOutText.push_back(temp);
                    end++; // Skip to the next character
                }
                else
                    end += 2; // Skip to after the space

                wrapped = true;
                wrappedLines++;
            }
            LaidOutPart temp(part, x, y, selColor);
            mLaidOutText.push_back(temp);
            x += font->getWidth(part);
        }
        y += font->getHeight();
    }
    setHeight(y);
    mLaidOutTextValid = true;
}
