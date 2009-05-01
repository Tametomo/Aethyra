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

#include "setup.h"
#include "setup_audio.h"
#include "setup_colors.h"
#include "setup_input.h"
#include "setup_players.h"
#include "setup_video.h"

#include "../bindings/guichan/widgets/button.h"
#include "../bindings/guichan/widgets/tabbedarea.h"

#include "../utils/dtor.h"
#include "../utils/gettext.h"

extern Window *chatWindow;
extern Window *statusWindow;
extern Window *buyDialog;
extern Window *sellDialog;
extern Window *buySellDialog;
extern Window *inventoryWindow;
extern Window *emoteWindow;
extern Window *npcTextDialog;
extern Window *npcStringDialog;
extern Window *skillDialog;
extern Window *minimap;
extern Window *equipmentWindow;
extern Window *tradeWindow;
extern Window *helpWindow;
extern Window *debugWindow;
extern Window *itemShortcutWindow;
extern Window *emoteShortcutWindow;
extern Window *storageWindow;

Setup::Setup():
    Window(_("Setup"))
{
    setCloseButton(true);
    int width = 340;
    int height = 340;
    setContentSize(width, height);

    static const char *buttonNames[] = {
        N_("Apply"), N_("Cancel"), N_("Reset Windows"), 0
    };
    int x = width;
    for (const char **curBtn = buttonNames; *curBtn; ++curBtn)
    {
        Button *btn = new Button(gettext(*curBtn), *curBtn, this);
        x -= btn->getWidth() + 5;
        btn->setPosition(x, height - btn->getHeight() - 5);
        add(btn);

        // Store this button, as it needs to be enabled/disabled
        if (!strcmp(*curBtn, N_("Reset Windows")))
            mResetWindows = btn;
    }

    TabbedArea *panel = new TabbedArea;
    panel->setDimension(gcn::Rectangle(5, 5, width - 10, height - 40));

    mTabs.push_back(new Setup_Video());
    mTabs.push_back(new Setup_Audio());
    mTabs.push_back(new Setup_Input());
    mTabs.push_back(new Setup_Colors());
    mTabs.push_back(new Setup_Players());

    for (std::list<SetupTabContainer*>::iterator i = mTabs.begin(),
         i_end = mTabs.end(); i != i_end; ++i)
    {
        SetupTabContainer *tab = *i;
        panel->addTab(tab->getName(), tab);
    }

    add(panel);

    setLocationRelativeTo(getParent());

    setInGame(false);
}

Setup::~Setup()
{
    delete_all(mTabs);
}

void Setup::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "Apply")
    {
        setVisible(false);
        for_each(mTabs.begin(), mTabs.end(), std::mem_fun(&SetupTabContainer::apply));
    }
    else if (event.getId() == "Cancel")
    {
        setVisible(false);
        for_each(mTabs.begin(), mTabs.end(), std::mem_fun(&SetupTabContainer::cancel));
    }
    else if (event.getId() == "Reset Windows")
    {
        // Bail out if this action happens to be activated before the windows
        // are created (though it should be disabled then)
        if (!statusWindow)
            return;

        chatWindow->resetToDefaultSize();
        statusWindow->resetToDefaultSize();
        buyDialog->resetToDefaultSize();
        sellDialog->resetToDefaultSize();
        buySellDialog->resetToDefaultSize();
        inventoryWindow->resetToDefaultSize();
        emoteWindow->resetToDefaultSize();
        npcTextDialog->resetToDefaultSize();
        npcStringDialog->resetToDefaultSize();
        skillDialog->resetToDefaultSize();
        minimap->resetToDefaultSize();
        equipmentWindow->resetToDefaultSize();
        tradeWindow->resetToDefaultSize();
        helpWindow->resetToDefaultSize();
        debugWindow->resetToDefaultSize();
        itemShortcutWindow->resetToDefaultSize();
        emoteShortcutWindow->resetToDefaultSize();
        storageWindow->resetToDefaultSize();
    }
}

void Setup::setInGame(bool inGame)
{
    mResetWindows->setEnabled(inGame);
}

Setup* setupWindow;
