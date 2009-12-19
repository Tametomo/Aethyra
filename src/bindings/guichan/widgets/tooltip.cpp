/*
 *  Aethyra
 *  Copyright (C) 2008,  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2009,  Aethyra Development Team
 *
 *  This file is part of Aethyra derived from original code
 *  from The Legend of Mazzeroth.
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

#include "textbox.h"
#include "tooltip.h"

#include "../gui.h"
#include "../graphics.h"
#include "../palette.h"
#include "../skin.h"

#include "../handlers/wordtextwraphandler.h"

ToolTip::ToolTip(gcn::Container *container):
    Container(),
    mText("")
{
    mToolTipBox = new TextBox(new WordTextWrapHandler());
    mToolTipBox->setEditable(false);
    mToolTipBox->setOpaque(false);

    add(mToolTipBox);
    container->add(this);

    setVisible(false);
}

void ToolTip::draw(gcn::Graphics *graphics)
{
    const int alpha = (int) (Skin::getAlpha() * 255.0f);
    Graphics *g = static_cast<Graphics*>(graphics);

    g->setColor(gcn::Color(245, 245, 181, alpha));
    g->fillRectangle(gcn::Rectangle(1, 1, getWidth() - 6, getHeight() - 6));

    g->setColor(gcn::Color(186, 186, 69, alpha));
    g->drawRectangle(gcn::Rectangle(0, 0, getWidth() - 5, getHeight() - 5));

    drawChildren(graphics);
}

void ToolTip::fontChanged()
{
    setText(mText);
    adjustSize();
}

void ToolTip::setText(std::string text)
{
    mText = text;
    mToolTipBox->setTextColor(&guiPalette->getColor(Palette::TEXT));
    mToolTipBox->setTextWrapped(text, 130);
}

void ToolTip::adjustSize()
{
    const int width = mToolTipBox->getMinWidth() + 9;
    const int fontHeight = getFont()->getHeight();
    const int numRows = mToolTipBox->getNumberOfRows();
    const int height = numRows * fontHeight + 7;

    setSize(width, height);

    const int xPos = (getWidth() - width) / 2 + 1;
    const int yPos = (getHeight() - height) / 2 + 1;

    mToolTipBox->setPosition(xPos, yPos);
}

void ToolTip::view(int x, int y)
{
    if (graphics->getWidth() < (x + getWidth() + 5))
        x = graphics->getWidth() - getWidth();

    x = x - getWidth() / 2;
    y = y - getHeight() - 10;

    setPosition(x, y);

    setVisible(true);
    requestMoveToTop();
}

