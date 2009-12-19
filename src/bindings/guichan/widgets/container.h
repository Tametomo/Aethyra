/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
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

#ifndef GUI_CONTAINER_H
#define GUI_CONTAINER_H

#include <guichan/widgets/container.hpp>

/**
 * A widget container. The only difference between the standard Guichan
 * container and this one is that childs added to this container are
 * automatically deleted when the container is deleted.
 */
class Container : public gcn::Container
{
    public:
        ~Container();

        /**
         * Overridden to store the previously held focus when containers are
         * reflowing their layouts, so that it can be restored later.
         */
        virtual void clear();

        /**
         * Overridden to allow for widgets to be removed from the Container
         * on the next logic tick.
         *
         * TODO: Remove the need for this, and make direct window deletion safe.
         */
        virtual void logic();

        /**
         * Restores focus to the last known focus holder.
         */
        void restoreFocus();

        /**
         * Allows for a window to both be removed from this Container, as well
         * as being deleted.
         */
        void scheduleDelete(gcn::Widget *widget);

        /**
         * Get the number of widget instances
         */
        int getNumberOfInstances() { return mWidgets.size(); }

        /**
         * Get the list of all widgets stored in this container
         */
        WidgetList getWidgetList() { return mWidgets; }
    private:
        WidgetList mDeathList;
};

extern Container *windowContainer;

#endif
