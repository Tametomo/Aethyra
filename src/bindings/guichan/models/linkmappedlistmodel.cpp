/*
 *  Aethyra
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

#include <guichan/font.hpp>
#include <guichan/graphics.hpp>

#include "linkmappedlistmodel.h"

#include "../gui.h"
#include "../palette.h"

#include "../../../core/configuration.h"

LinkMappedListModel::LinkMappedListModel()
{
    clear();
}

void LinkMappedListModel::addLink(std::string key, std::string text)
{
    const unsigned int fontHeight = gui->getFont()->getHeight();
    const unsigned int textWidth = gui->getFont()->getWidth(text) + 3;

    mActions.push_back(key);
    mText.push_back(text);
    mHeight = fontHeight * mActions.size() + 3;

    if (textWidth > mWidth)
        mWidth = textWidth;

    if (mSelected == "")
        mSelected = mActions[0];
}

void LinkMappedListModel::clear()
{
    mActions.clear();
    mText.clear();
    mSelected = "";
    mWidth = 3;
    mHeight = 3;
}

void LinkMappedListModel::selectNext()
{
    for (size_t i = 0; i < (mActions.size() - 1); i++)
    {
        if (mSelected == mActions[i])
        {
            i++;
            while (i < mActions.size() && mActions[i] == "")
                i++;

            mSelected = mActions[i];
            return;
        }
    }
    mSelected = "";
}

void LinkMappedListModel::selectPrevious()
{
    if (mActions[0] == mSelected)
    {
        mSelected = "";
        return;
    }

    for (size_t i = 1; i < mActions.size(); i++)
    {
        if (mSelected == mActions[i])
        {
            i--;
            while (i > 0 && mActions[i] == "")
                i--;

            mSelected = mActions[i];
            return;
        }
    }
}

void LinkMappedListModel::setSelectedByY(int y)
{
    size_t index = y / gui->getFont()->getHeight();
    mSelected = (index >= mActions.size() ? "" : mActions[index]);
}

std::string LinkMappedListModel::getSelectedByY(int y)
{
    size_t index = y / gui->getFont()->getHeight();
    return (index >= mActions.size() ? "" : mActions[index]);
}

void LinkMappedListModel::draw(gcn::Graphics *graphics)
{
    graphics->setFont(gui->getFont());
    const int fontHeight = gui->getFont()->getHeight();
    const int alpha = (int) (config.getValue("guialpha", 0.8) * 255.0f);
    int y = 0;

    for (size_t i = 0; i < mActions.size(); i++)
    {
        int textLength = gui->getFont()->getWidth(mText[i]);

        if (mSelected == mActions[i] && mSelected != "")
        {
            graphics->setColor(guiPalette->getColor(Palette::HIGHLIGHT, alpha));
            graphics->fillRectangle(gcn::Rectangle(1, y, textLength + 1,
                                                   fontHeight));
            graphics->setColor(guiPalette->getColor(Palette::HYPERLINK));
            graphics->drawLine(1, y + fontHeight, textLength + 1, y + fontHeight);
        }
        graphics->setColor(guiPalette->getColor(mActions[i] == "" ? Palette::TEXT :
                                                                    Palette::HYPERLINK));
        graphics->drawText(mText[i], 1, y);

        y += fontHeight;
    }
}

