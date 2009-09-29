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

#include <string.h>

#include "setupdialog.h"

#include "tabs/setup_audio.h"
#include "tabs/setup_colors.h"
#include "tabs/setup_input.h"
#include "tabs/setup_video.h"

#include "../layout.h"

#include "../widgets/button.h"
#include "../widgets/desktop.h"
#include "../widgets/tabbedarea.h"
#include "../widgets/windowcontainer.h"

#include "../../../core/configuration.h"

#include "../../../core/utils/dtor.h"
#include "../../../core/utils/gettext.h"

Setup::Setup():
    Window(_("Setup"))
{
    setWindowName("Setup");
    saveVisibility(false);
    setCloseButton(true);
    int width = 340 + 2 * getPadding();
    int height = 340 + 2 * getPadding() + getTitleBarHeight();

    static const char *buttonNames[] = {
        N_("Reset Windows"), N_("Cancel"), N_("Apply"), 0
    };

    mPanel = new TabbedArea();
    mPanel->setDimension(gcn::Rectangle(5, 5, width - 10, height - 40));

    mTabs.push_back(new Setup_Video());
    mTabs.push_back(new Setup_Audio());
    mTabs.push_back(new Setup_Input());
    mTabs.push_back(new Setup_Colors());

    for (std::list<SetupTabContainer*>::iterator i = mTabs.begin(),
         i_end = mTabs.end(); i != i_end; ++i)
    {
        SetupTabContainer *tab = *i;
        mPanel->addTab(tab->getName(), tab);
    }

    place(0, 0, mPanel, 7, 6).setPadding(2);

    for (int i = 0; buttonNames[i] != NULL; ++i)
    {
        Button *btn = new Button(gettext(buttonNames[i]), buttonNames[i], this);
        place(i + 4, 6, btn);

        // Store this button, as it needs to be enabled/disabled
        if (!strcmp(buttonNames[i], N_("Reset Windows")))
            mResetWindows = btn;
    }

    setDefaultSize(width, height, ImageRect::CENTER);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    loadWindowState();
}

Setup::~Setup()
{
    delete_all(mTabs);
}

void Setup::addTab(SetupTabContainer *tab)
{
    mTabs.push_back(tab);
    mPanel->addTab(tab->getName(), tab);
}

void Setup::removeTab(SetupTabContainer *tab)
{
    Tab* tabToDelete = mPanel->getTab(tab->getName());
    mPanel->removeTab(tabToDelete);

    for (std::list<SetupTabContainer*>::iterator i = mTabs.begin(),
         i_end = mTabs.end(); i != i_end; ++i)
    {
        if (*i == tab)
        {
            i = mTabs.erase(i);
            break;
        }
    }
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
        config.removeAllValues("Hidden");
        config.removeAllValues("OffsetX");
        config.removeAllValues("OffsetY");
        config.removeAllValues("Position");
        config.removeAllValues("Height");
        config.removeAllValues("Width");

        Widgets widgets = windowContainer->getWidgetList();
        WidgetIterator iter;

        for (iter = widgets.begin(); iter != widgets.end(); ++iter)
        {
            Window* window = dynamic_cast<Window*>(*iter);

            if (window && (!desktop || desktop->getCurrentDialog() != window))
                window->resetToDefaultSize();
        }
    }
}

void Setup::requestFocus()
{
    mPanel->requestFocus();
}

