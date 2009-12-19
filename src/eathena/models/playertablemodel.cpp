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

#include "playerrelationlistmodel.h"
#include "playertablemodel.h"

#include "../../bindings/guichan/widgets/dropdown.h"
#include "../../bindings/guichan/widgets/label.h"
#include "../../bindings/guichan/widgets/table.h"

#include "../../core/utils/dtor.h"

#define NAME_COLUMN 0
#define RELATION_CHOICE_COLUMN 1

// The following column widths really shouldn't be hardcoded but should scale
// with the size of the widget... except that, right now, the widget doesn't
// exactly scale either.
#define NAME_COLUMN_WIDTH 230
#define RELATION_CHOICE_COLUMN_WIDTH 80

#define WIDGET_AT(row, column) (((row) * COLUMNS_NR) + column)

PlayerTableModel::PlayerTableModel(void) :
    mPlayers(NULL)
{
    playerRelationsUpdated();
}

PlayerTableModel::~PlayerTableModel(void)
{
    freeWidgets();

    if (mPlayers)
        destroy(mPlayers);
}

int PlayerTableModel::getColumnWidth(int index)
{
    return (index == NAME_COLUMN ? NAME_COLUMN_WIDTH :
                                   RELATION_CHOICE_COLUMN_WIDTH);
}

void PlayerTableModel::playerRelationsUpdated(void)
{
    signalBeforeUpdate();
    freeWidgets();

    std::vector<std::string> *player_names = player_relations.getPlayers();

    if (mPlayers)
        destroy(mPlayers);

    mPlayers = player_names;

    // set up widgets
    for (unsigned int r = 0; r < player_names->size(); ++r)
    {
        std::string name = (*player_names)[r];
        gcn::Widget *widget = new Label(name);
        mWidgets.push_back(widget);
        gcn::ListModel *playerRelation = new PlayerRelationListModel();

        gcn::DropDown *choicebox = new DropDown(playerRelation);
        choicebox->setSelected(player_relations.getRelation(name));
        mWidgets.push_back(choicebox);
    }

    signalAfterUpdate();
}

void PlayerTableModel::updateModelInRow(int row)
{
    gcn::DropDown *choicebox = dynamic_cast<gcn::DropDown *>(
                               getElementAt(row, RELATION_CHOICE_COLUMN));
    player_relations.setRelation(getPlayerAt(row),
                                 static_cast<PlayerRelation::relation>(
                                 choicebox->getSelected()));
}

void PlayerTableModel::freeWidgets(void)
{
    if (mPlayers)
        destroy(mPlayers);

    for (std::vector<gcn::Widget *>::const_iterator it = mWidgets.begin();
         it != mWidgets.end(); it++)
    {
        delete *it;
    }

    mWidgets.clear();
}
