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

#ifndef LINKMAPPEDLISTMODEL_H
#define LINKMAPPEDLISTMODEL_H

#include <string>
#include <vector>

#include "mappedlistmodel.h"

/**
 * An implementation of a mapped list model which will create a clickable link
 * list when given an action ID and some display text. Events with a NULL action
 * event will be considered to not be links, and won't be selectable.
 */
class LinkMappedListModel : public MappedListModel
{
    public:
        /**
         * Constructor.
         */
        LinkMappedListModel();

        /**
         * Destructor.
         */
        ~LinkMappedListModel() { }

        /**
         * Adds a link to the list model.
         */
        void addLink(std::string key, std::string text);

        /**
         * Clears the list model of all links.
         */
        void clear();

        /**
         * Selects the next element in the mapped list.
         */
        void selectNext();

        /**
         * Selects the previous element in the mapped list.
         */
        void selectPrevious();

        /**
         * Selects the first element in the mapped list. This implementation
         * assumes that the user did not place a dead link here for simplicity.
         */
        void selectFirst() { mSelected = mActions[0]; }

        /**
         * Selects the last element in the mapped list. This implementation
         * assumes that the user did not place a dead link here for simplicity.
         */
        void selectEnd() { mSelected = mActions[mActions.size() - 1]; }

        /**
         * Set the selected element based on a given Y value. Used to allow for
         * mouse interaction with this model type.
         *
         * @param y The y coordinate within the list which should get selected.
         */
        void setSelectedByY(int y);

        /**
         * Gets the selected element based on a given Y value. Used to allow for
         * mouse interaction with this model type.
         *
         * @param y The y coordinate within the list which should get selected.
         *
         * @return The selection for this Y coordinate.
         */
        std::string getSelectedByY(int y);

        /**
         * Contains how to draw this particular list type.
         */
        void draw(gcn::Graphics *graphics);

    private:
        // Use dual vectors to preserve link ordering, and to allow duplicates.
        std::vector<std::string> mActions;
        std::vector<std::string> mText;
};

#endif
