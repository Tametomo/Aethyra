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

#include <string>

#include <guichan/font.hpp>

#include "textbox.h"

#include "../palette.h"

#include "../../../core/utils/dtor.h"

TextBox::TextBox(TextWrapHandler *wrapHandler) :
    gcn::TextBox(),
    mMaxDimension(100),
    mRawText(""),
    mWrapHandler(wrapHandler),
    mTextColor(&guiPalette->getColor(Palette::TEXT))
{
    setOpaque(false);
    setFocusable(false);
    setEditable(false);
    setFrameSize(0);
    mMinWidth = getWidth();
}

TextBox::~TextBox()
{
    destroy(mWrapHandler);
}

void TextBox::setTextWrapped(const std::string &text, int maxDimension)
{
    mMaxDimension = maxDimension;

    // Make sure parent scroll area sets width of this widget
    if (getParent())
        getParent()->logic();

    mMinWidth = getFont()->getWidth(text);

    mRawText = text;

    std::string wrappedText;

    if (mWrapHandler)
    {
        mMinWidth = maxDimension;
        wrappedText = mWrapHandler->wrapText(getFont(), mRawText, mMinWidth);
    }

    gcn::TextBox::setText(wrappedText);
}

void TextBox::fontChanged()
{
    gcn::TextBox::fontChanged();
    setTextWrapped(mRawText, mMaxDimension);
    adjustSize();
}
