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

#ifndef MAPPEDLISTMODEL_H
#define MAPPEDLISTMODEL_H

#include <string>

/**
 * An interface for a model that represents a mapped list. This isn't really all
 * that similar to a GUIChan list model, since it stores its elements based on
 * an action event ID, instead of an integer, is responsible for managing its
 * own drawing events to allow for more flexibility to the implementer over how
 * their model is displayed, and that it's a more literal list as well, with the
 * current index being unknowable to the listbox itself, although in order to
 * allow for mouse support, an override is available which can do direct access.
 *
 * Use MappedLists when you need more precision with your lists or need to make
 * a nontraditional list (e.g. a list composed of different widgets, although it
 * probably would be severe overkill and cut performance a lot), and the GUIChan
 * lists when you don't.
 */
class MappedListModel
{
    public:
        /**
         * Destructor.
         */
        virtual ~MappedListModel() { }

        /**
         * Selects the next element in the mapped list.
         */
        virtual void selectNext() = 0;

        /**
         * Selects the previous element in the mapped list.
         */
        virtual void selectPrevious() = 0;

        /**
         * Selects the first element in the mapped list.
         */
        virtual void selectFirst() = 0;

        /**
         * Selects the last element in the mapped list.
         */
        virtual void selectEnd() = 0;

        /**
         * Set the selected element based on a given Y value. Used to allow for
         * mouse interaction with this model type.
         *
         * @param y The y coordinate within the list which should get selected.
         */
        virtual void setSelectedByY(int y) = 0;

        /**
         * Clears the list model of all list elements.
         */
        virtual void clear() = 0;

        /**
         * Returns the action ID of the current selected element.
         *
         * @return The current selection. NULL if nothing selected.
         */
        const std::string &getSelected() { return mSelected; }

        /**
         * Gets the width of the displayed content. 
         */
        unsigned getWidth() { return mWidth; }

        /**
         * Gets the total used height for the model.
         */
        unsigned int getHeight() { return mHeight; }

        /**
         * Contains how to draw this particular list type.
         */
        virtual void draw(gcn::Graphics *graphics) = 0;

    protected:
        unsigned int mWidth;
        unsigned int mHeight;

        std::string mSelected;
};

#endif
