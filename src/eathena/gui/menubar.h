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

#ifndef MENU_H
#define MENU_H

#include <vector>

#include <guichan/widgets/button.hpp>

#include "../../bindings/guichan/widgets/popup.h"

/**
 * The Button Menu.
 *
 * \ingroup Interface
 */
class MenuBar : public Popup
{
    public:
        /**
         * Constructor.
         */
        MenuBar();

        /**
         * Reflows the menu bar on resolution changing and font size changing.
         */
        void adjustSizeAndPosition();

        /**
         * Draw this window
         */
        void draw(gcn::Graphics *graphics);

        std::vector<gcn::Button*> buttons;
};

extern MenuBar *menuBar;

#endif

