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

#include <guichan/font.hpp>

#include "confirmdialog.h"

#include "../handlers/wordtextwraphandler.h"

#include "../widgets/button.h"
#include "../widgets/container.h"
#include "../widgets/textbox.h"

#include "../../../core/utils/gettext.h"

ConfirmDialog::ConfirmDialog(const std::string &title, const std::string &msg,
                             Window *parent, bool modal):
    Window(title, modal, parent)
{
    mTextBox = new TextBox(new WordTextWrapHandler());
    mTextBox->setEditable(false);
    mTextBox->setOpaque(false);
    mTextBox->setTextWrapped(msg, 260);

    mYesButton = new Button(_("Yes"), "yes", this);
    mNoButton = new Button(_("No"), "no", this);

    fontChanged();

    add(mTextBox);
    add(mYesButton);
    add(mNoButton);

    if (getParent())
    {
        setLocationRelativeTo(getParent());
        getParent()->moveToTop(this);
    }

    setVisible(true);
    mYesButton->requestFocus();
}

void ConfirmDialog::fontChanged()
{
    Window::fontChanged();

    const int numRows = mTextBox->getNumberOfRows();
    const int inWidth = mYesButton->getWidth() + mNoButton->getWidth() + 
                        (2 * getPadding());
    const int fontHeight = getFont()->getHeight();
    const int height = numRows * fontHeight;
    int width = getFont()->getWidth(getCaption());

    if (width < mTextBox->getMinWidth())
        width = mTextBox->getMinWidth();
    if (width < inWidth)
        width = inWidth;

    setContentSize(mTextBox->getMinWidth() + fontHeight, height + fontHeight +
                   mNoButton->getHeight());
    mTextBox->setPosition(getPadding(), getPadding());

    mYesButton->setPosition((width - inWidth) / 2, height + 
                            (2 * mYesButton->getSpacing()));
    mNoButton->setPosition(mYesButton->getX() + inWidth - mNoButton->getWidth(),
                           height + (2 * mNoButton->getSpacing()));
}

void ConfirmDialog::close()
{
    Window::close();
    windowContainer->scheduleDelete(this);
}

unsigned int ConfirmDialog::getNumRows()
{
    return mTextBox->getNumberOfRows();
}

void ConfirmDialog::action(const gcn::ActionEvent &event)
{
    // Proxy button events to our listeners
    ActionListenerIterator i;
    for (i = mActionListeners.begin(); i != mActionListeners.end(); ++i)
        (*i)->action(event);

    // Can we receive anything else anyway?
    if (event.getId() == "yes" || event.getId() == "no")
    {
        close();
        return;
    }

    Window::action(event);
}
