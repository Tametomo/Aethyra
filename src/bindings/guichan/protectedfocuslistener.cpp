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

#include "protectedfocuslistener.h"

#include "../sdl/keyboardconfig.h"

void ProtectedFocusListener::focusGained(const gcn::Event &event)
{
    keyboard.setPrintableKeyLock(mPrintableKeyLock);

    for (std::list<int>::iterator it = mBlockList.begin();
         it != mBlockList.end(); it++)
    {
        keyboard.lockKey(*it);
    }
}

void ProtectedFocusListener::focusLost(const gcn::Event &event)
{
    keyboard.setPrintableKeyLock(false);

    for (std::list<int>::iterator it = mBlockList.begin();
         it != mBlockList.end(); it++)
    {
        keyboard.unlockKey(*it);
    }
}

void ProtectedFocusListener::blockKey(const int keyValue)
{
    mBlockList.push_back(keyValue);
}

void ProtectedFocusListener::unblockKey(const int keyValue)
{
    for (std::list<int>::iterator it = mBlockList.begin();
         it != mBlockList.end(); it++)
    {
        if (*it == keyValue)
        {
            keyboard.unlockKey(*it);
            it = mBlockList.erase(it);
            return;
        }
    }
}
