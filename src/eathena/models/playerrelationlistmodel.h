/*
 *  Aethyra
 *  Copyright (C) 2008  The Mana World Development Team
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

#include <guichan/listmodel.hpp>

#include "../playerrelations.h"

#include "../../core/utils/gettext.h"

#define COLUMNS_NR 2 // name plus listbox
#define NAME_COLUMN 0
#define RELATION_CHOICE_COLUMN 1

#define ROW_HEIGHT 12
// The following column widths really shouldn't be hardcoded but should scale
// with the size of the widget... except that, right now, the widget doesn't
// exactly scale either.
#define NAME_COLUMN_WIDTH 230
#define RELATION_CHOICE_COLUMN_WIDTH 80

#define WIDGET_AT(row, column) (((row) * COLUMNS_NR) + column)

static const char *RELATION_NAMES[PlayerRelation::RELATIONS_NR] =
{
    N_("Neutral"),
    N_("Friend"),
    N_("Disregarded"),
    N_("Ignored")
};

class PlayerRelationListModel : public gcn::ListModel
{
    public:
        virtual ~PlayerRelationListModel(void) { }

        virtual int getNumberOfElements(void)
        { return PlayerRelation::RELATIONS_NR; }

        virtual std::string getElementAt(int i)
        {
            if (i >= getNumberOfElements() || i < 0)
                return "";
            return gettext(RELATION_NAMES[i]);
        }
};
