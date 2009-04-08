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

#ifndef IGNORE_CHOICES_LIST_MODEL_H
#define IGNORE_CHOICES_LIST_MODEL_H

#include <string>

#include <guichan/listmodel.hpp>

/**
 * Class for choosing one of the various `what to do when ignoring a player'
 * options
 */
class IgnoreChoicesListModel : public gcn::ListModel
{
    public:
        virtual ~IgnoreChoicesListModel(void) {}

        virtual int getNumberOfElements(void)
        { return player_relations.getPlayerIgnoreStrategies()->size(); }

        virtual std::string getElementAt(int i)
        {
            return (i >= getNumberOfElements() ? "???" :
                (*player_relations.getPlayerIgnoreStrategies())[i]->mDescription);
        }
};

#endif
