/*
 *  The Mana World
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _TMW_ITEMPOPUP_H__
#define _TMW_ITEMPOPUP_H__

#include "scrollarea.h"
#include "textbox.h"
#include "window.h"

#include "../item.h"

class ItemPopup : public Window
{
    public:
        ItemPopup();

        void setItem(const ItemInfo &item);
        unsigned int getNumRows();

    private:
        gcn::Label *mItemName;
        TextBox *mItemDesc;
        TextBox *mItemEffect;
        ScrollArea *mItemDescScroll;
        ScrollArea *mItemEffectScroll;
};

#endif // _TMW_ITEMPOPUP_H__