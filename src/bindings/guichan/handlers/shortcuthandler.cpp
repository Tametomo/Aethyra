/*
 *  Aethyra
 *  Copyright (C) 2007  The Mana World Development Team
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra derived from original code
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

#include <algorithm>

#include "shortcuthandler.h"

#include "../../../core/configuration.h"

#include "../../../core/utils/stringutils.h"

ShortcutHandler::ShortcutHandler(const std::string &prefix):
    mPrefix(prefix),
    mSelection(-1)
{
    load();
}

ShortcutHandler::~ShortcutHandler()
{
    save();
}

int ShortcutHandler::getShortcutIndex(int id) const
{
    int index = -1;

    for (int i = 0; i < SHORTCUTS; i++)
    {
        if (mIndex[i] == id)
        {
            index = i;
            break;
        }
    }

    return index;
}

void ShortcutHandler::load()
{
    if (mPrefix == "")
    {
        std::fill_n(mIndex, SHORTCUTS, -1);
        return;
    }

    for (int i = 0; i < SHORTCUTS; i++)
        mIndex[i] = config.getValue(mPrefix + toString(i), -1);
}

void ShortcutHandler::save() const
{
    if (mPrefix == "")
        return;

    for (int i = 0; i < SHORTCUTS; i++)
        config.setValue(mPrefix + toString(i), mIndex[i]);
}
