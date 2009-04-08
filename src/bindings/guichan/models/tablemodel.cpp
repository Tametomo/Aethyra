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

#include "tablemodel.h"

void TableModel::installListener(TableModelListener *listener)
{
    listeners.insert(listener);
}

void TableModel::removeListener(TableModelListener *listener)
{
    listeners.erase(listener);
}

void TableModel::signalBeforeUpdate(void)
{
    for (std::set<TableModelListener *>::const_iterator it = listeners.begin(); it != listeners.end(); it++)
        (*it)->modelUpdated(false);
}

void TableModel::signalAfterUpdate(void)
{
    for (std::set<TableModelListener *>::const_iterator it = listeners.begin(); it != listeners.end(); it++)
        (*it)->modelUpdated(true);
}

