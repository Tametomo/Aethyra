/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra derived from original code
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

#include <sstream>

#include "listdialog.h"

#include "../layout.h"

#include "../widgets/button.h"
#include "../widgets/listbox.h"
#include "../widgets/scrollarea.h"

#include "../../../core/utils/gettext.h"

ListDialog::ListDialog(const std::string &caption):
    Window(caption)
{
    setResizable(true);

    setMinWidth(200);
    setMinHeight(150);

    setDefaultSize(260, 200, ImageRect::CENTER);

    mOptionList = new ListBox(this, "ok", this);
    mOptionList->setWrappingEnabled(true);

    mScrollArea = new ScrollArea(mOptionList);

    mOkButton = new Button(_("OK"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    setContentSize(260, 175);
    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    fontChanged();
}

void ListDialog::fontChanged()
{
    Window::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    place(0, 0, mScrollArea, 5).setPadding(3);
    place(3, 1, mCancelButton);
    place(4, 1, mOkButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);
}

int ListDialog::getNumberOfElements()
{
    return mOptions.size();
}

std::string ListDialog::getElementAt(int i)
{
    return mOptions[i];
}

const int ListDialog::getSelected()
{
    return mOptionList->getSelected();
}

void ListDialog::addOption(const std::string &option)
{
    mOptions.push_back(option);
}

void ListDialog::reset()
{
    mOptionList->setSelected(-1);
    mOptions.clear();
}

void ListDialog::action(const gcn::ActionEvent &event)
{
    // Proxy events to our listeners
    ActionListenerIterator i;

    for (i = mActionListeners.begin(); i != mActionListeners.end(); ++i)
        (*i)->action(event);
}

void ListDialog::requestFocus()
{
    if (getNumberOfElements() > 0)
        mOptionList->requestFocus();

    mOkButton->setEnabled(getNumberOfElements() > 0);
}

void ListDialog::widgetShown(const gcn::Event& event)
{
    Window::widgetShown(event);

    if (getNumberOfElements() > 0)
        mOptionList->setSelected(0);
}

void ListDialog::widgetHidden(const gcn::Event& event)
{
    Window::widgetHidden(event);
    reset();
}

