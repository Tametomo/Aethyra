/*
 *  Aethyra
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
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

#include <guichan/widgets/label.hpp>

#include "itempopup.h"

#include "../bindings/guichan/graphics.h"
#include "../bindings/guichan/gui.h"
#include "../bindings/guichan/palette.h"

#include "../bindings/guichan/widgets/textbox.h"

#include "../resources/db/iteminfo.h"

#include "../utils/gettext.h"
#include "../utils/stringutils.h"

ItemPopup::ItemPopup():
    Popup("ItemPopup")
{
    mItemType = "";

    // Item Name
    mItemName = new gcn::Label("");
    mItemName->setFont(boldFont);
    mItemName->setPosition(getPadding(), getPadding());

    const int fontHeight = getFont()->getHeight();

    // Item Description
    mItemDesc = new TextBox();
    mItemDesc->setEditable(false);
    mItemDesc->setPosition(getPadding(), fontHeight);

    // Item Effect
    mItemEffect = new TextBox();
    mItemEffect->setEditable(false);
    mItemEffect->setPosition(getPadding(), 2 * fontHeight + 2 * getPadding());

    // Item Weight
    mItemWeight = new TextBox();
    mItemWeight->setEditable(false);
    mItemWeight->setPosition(getPadding(), 3 * fontHeight + 4 * getPadding());

    add(mItemName);
    add(mItemDesc);
    add(mItemEffect);
    add(mItemWeight);

    loadPopupConfiguration();
}

ItemPopup::~ItemPopup()
{
}

void ItemPopup::setItem(const ItemInfo &item)
{
    if (item.getName() == mItemName->getCaption())
        return;

    mItemName->setCaption(item.getName());
    mItemName->setWidth(boldFont->getWidth(item.getName()));
    mItemDesc->setTextWrapped(item.getDescription(), 196);
    mItemEffect->setTextWrapped(item.getEffect(), 196);
    mItemWeight->setTextWrapped(_("Weight: ") + toString(item.getWeight()) +
                                _(" grams"), 196);
    mItemType = item.getType();

    int minWidth = mItemName->getWidth();

    if (mItemDesc->getMinWidth() > minWidth)
        minWidth = mItemDesc->getMinWidth();
    if (mItemEffect->getMinWidth() > minWidth)
        minWidth = mItemEffect->getMinWidth();
    if (mItemWeight->getMinWidth() > minWidth)
        minWidth = mItemWeight->getMinWidth();

    minWidth += 8;
    setWidth(minWidth);

    const int numRowsDesc = mItemDesc->getNumberOfRows();
    const int numRowsEffect = mItemEffect->getNumberOfRows();
    const int numRowsWeight = mItemWeight->getNumberOfRows();
    const int height = getFont()->getHeight();

    if (item.getEffect().empty())
    {
        setContentSize(minWidth, (numRowsDesc + numRowsWeight + getPadding()) *
                       height);

        mItemWeight->setPosition(getPadding(), (numRowsDesc + getPadding()) *
                                 height);
    }
    else
    {
        setContentSize(minWidth, (numRowsDesc + numRowsEffect + numRowsWeight +
                       getPadding()) * height);

        mItemWeight->setPosition(getPadding(), (numRowsDesc + numRowsEffect +
                                 getPadding()) * height);
    }

    mItemDesc->setPosition(getPadding(), 2 * height);
    mItemEffect->setPosition(getPadding(), (numRowsDesc + getPadding()) * height);
}

void ItemPopup::updateColors()
{
    mItemName->setForegroundColor(getColor(mItemType));
    graphics->setColor(guiPalette->getColor(Palette::TEXT));
}

gcn::Color ItemPopup::getColor(const std::string& type)
{
    gcn::Color color;

    if (type.compare("generic") == 0)
        color = guiPalette->getColor(Palette::GENERIC);
    else if (type.compare("equip-head") == 0)
        color = guiPalette->getColor(Palette::HEAD);
    else if (type.compare("usable") == 0)
        color = guiPalette->getColor(Palette::USABLE);
    else if (type.compare("equip-torso") == 0)
        color = guiPalette->getColor(Palette::TORSO);
    else if (type.compare("equip-1hand") == 0)
        color = guiPalette->getColor(Palette::ONEHAND);
    else if (type.compare("equip-legs") == 0)
        color = guiPalette->getColor(Palette::LEGS);
    else if (type.compare("equip-feet") == 0)
        color = guiPalette->getColor(Palette::FEET);
    else if (type.compare("equip-2hand") == 0)
        color = guiPalette->getColor(Palette::TWOHAND);
    else if (type.compare("equip-shield") == 0)
        color = guiPalette->getColor(Palette::SHIELD);
    else if (type.compare("equip-ring") == 0)
        color = guiPalette->getColor(Palette::RING);
    else if (type.compare("equip-arms") == 0)
        color = guiPalette->getColor(Palette::ARMS);
    else if (type.compare("equip-ammo") == 0)
        color = guiPalette->getColor(Palette::AMMO);
    else
        color = guiPalette->getColor(Palette::UNKNOWN_ITEM);

    return color;
}

std::string ItemPopup::getItemName() const
{
    return mItemName->getCaption();
}

unsigned int ItemPopup::getNumRows() const
{
    return mItemDesc->getNumberOfRows() + mItemEffect->getNumberOfRows() +
           mItemWeight->getNumberOfRows();
}

void ItemPopup::view(int x, int y)
{
    if (graphics->getWidth() < (x + getWidth() + 5))
       x = graphics->getWidth() - getWidth();

    y = y - getHeight() - 10;

    setPosition(x, y);
    setVisible(true);
    requestMoveToTop();
}
