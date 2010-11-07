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

#include <string>

#include <guichan/actionlistener.hpp>

#include "emotewindow.h"
#include "equipmentwindow.h"
#include "inventorywindow.h"
#include "menubar.h"
#include "skill.h"
#include "status.h"

#include "../../bindings/guichan/graphics.h"

#include "../../bindings/guichan/dialogs/setupdialog.h"

#include "../../bindings/guichan/widgets/button.h"
#include "../../bindings/guichan/widgets/window.h"

#include "../../core/utils/gettext.h"

namespace {
    struct MenuBarListener : public gcn::ActionListener
    {
        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event);
    } listener;
}

MenuBar::MenuBar():
    Popup("Menu")
{
    setVisible(true);

    // Buttons
    static const char *buttonNames[] =
    {
        N_("Status"),
        N_("Equipment"),
        N_("Inventory"),
        N_("Skills"),
        N_("Emote"),
        N_("Setup"),
        0
    };

    for (const char **curBtn = buttonNames; *curBtn; curBtn++)
    {
        gcn::Button *btn = new Button(gettext(*curBtn), *curBtn, &listener);
        btn->setFocusable(false);
        buttons.push_back(btn);
        add(btn);
    }

    adaptToNewSize();
}

void MenuBar::adaptToNewSize()
{
    Popup::adaptToNewSize();

    int x = 0, h = 0;

    for (size_t i = 0; i < buttons.size(); i++)
    {
        buttons[i]->setPosition(x, 0);
        buttons[i]->adjustSize();
        x += buttons[i]->getWidth() + 3;
        h = buttons[i]->getHeight();
    }

    setPosition(graphics->getWidth() - x - 3, 3);
    setContentSize(x - 3, h);
}

void MenuBar::draw(gcn::Graphics *graphics)
{
    drawChildren(graphics);
}

void MenuBarListener::action(const gcn::ActionEvent &event)
{
    Window *window = NULL;

    if (event.getId() == "Status")
        window = statusWindow;
    else if (event.getId() == "Equipment")
        window = equipmentWindow;
    else if (event.getId() == "Inventory")
        window = inventoryWindow;
    else if (event.getId() == "Skills")
        window = skillDialog;
    else if (event.getId() == "Emote")
        window = emoteWindow;
    else if (event.getId() == "Setup")
        window = setupWindow;

    if (window)
        window->setVisible(!window->isVisible());
}
