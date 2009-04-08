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

#ifndef PLAYER_TABLE_MODEL_H
#define PLAYER_TABLE_MODEL_H

#include <string>
#include <vector>

#include "tablemodel.h"

#define COLUMNS_NR 2 // name plus listbox
#define ROW_HEIGHT 12

#define WIDGET_AT(row, column) (((row) * COLUMNS_NR) + column)

class PlayerTableModel : public TableModel
{
    public:
        PlayerTableModel(void);

        virtual ~PlayerTableModel(void);

        virtual int getRows(void) { return mPlayers->size(); }

        virtual int getColumns(void) { return COLUMNS_NR; }

        virtual int getRowHeight(void) { return ROW_HEIGHT; }

        virtual int getColumnWidth(int index);

        virtual void playerRelationsUpdated(void);

        virtual void updateModelInRow(int row);

        virtual gcn::Widget *getElementAt(int row, int column)
        { return mWidgets[WIDGET_AT(row, column)]; }

        virtual void freeWidgets(void);

        std::string getPlayerAt(int index) { return (*mPlayers)[index]; }

    protected:
        std::vector<std::string> *mPlayers;
        std::vector<gcn::Widget*> mWidgets;
};

#endif
