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

#include "helpdialog.h"

#include "../layout.h"

#include "../widgets/button.h"
#include "../widgets/browserbox.h"
#include "../widgets/scrollarea.h"

#include "../../../core/resourcemanager.h"

#include "../../../core/utils/gettext.h"

HelpDialog::HelpDialog():
    Window(_("Help"))
{
    setMinWidth(300);
    setMinHeight(250);
    setContentSize(455, 350);
    setWindowName(_("Help"));
    setResizable(true);
    saveVisibility(false);
    setCloseButton(true);

    setDefaultSize(500, 400, ImageRect::CENTER);

    mBrowserBox = new BrowserBox();
    mBrowserBox->setOpaque(false);
    mScrollArea = new ScrollArea(mBrowserBox);
    mOkButton = new Button(_("Close"), "close", this);

    mScrollArea->setDimension(gcn::Rectangle(5, 5, 445,
                                             335 - mOkButton->getHeight()));
    mOkButton->setPosition(450 - mOkButton->getWidth(),
                           345 - mOkButton->getHeight());

    mBrowserBox->setLinkHandler(this);

    fontChanged();
    loadWindowState();
    loadHelp("index");
}

void HelpDialog::fontChanged()
{
    Window::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    place(0, 0, mScrollArea, 5, 3).setPadding(3);
    place(4, 3, mOkButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    restoreFocus();
}

void HelpDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "close")
        close();
}

void HelpDialog::handleLink(const std::string& link)
{
    std::string helpFile = link;
    loadHelp(helpFile);
}

void HelpDialog::loadHelp(const std::string &helpFile)
{
    mBrowserBox->clearRows();

    loadFile("header");
    loadFile(helpFile);

    mScrollArea->setVerticalScrollAmount(0);
}

void HelpDialog::loadFile(const std::string &file)
{
    ResourceManager *resman = ResourceManager::getInstance();
    std::vector<std::string> lines =
        resman->loadTextFile("help/" + file + ".txt");

    for (unsigned int i = 0; i < lines.size(); ++i)
    {
        mBrowserBox->addRow(lines[i]);
    }
}

void HelpDialog::requestFocus()
{
    mOkButton->requestFocus();
}

void HelpDialog::close()
{
    Window::close();

    loadHelp("index");
}
