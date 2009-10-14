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

#ifndef SETUP_H
#define SETUP_H

#include <list>

#include <guichan/actionlistener.hpp>

#include "../widgets/window.h"

class SetupTabContainer;
class TabbedArea;

/**
 * The setup dialog. Displays several tabs for configuring different aspects
 * of the game.
 *
 * @see Setup_Audio
 * @see Setup_Colors
 * @see Setup_Input
 * @see Setup_Players
 * @see Setup_Video
 *
 * \ingroup GUI
 */
class Setup : public Window, public gcn::ActionListener
{
    public:
        Setup();
        ~Setup();

        void addTab(SetupTabContainer *tab);

        void removeTab(SetupTabContainer *tab);

        /**
         * Event handling method.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Focuses on the tabbed area on gaining focus.
         */
        void requestFocus();

        void fontChanged();
    private:
        TabbedArea *mPanel;
        std::list<gcn::Button*> mButtons;
        std::list<SetupTabContainer*> mTabs;
};

extern Setup* setupWindow;

#endif
