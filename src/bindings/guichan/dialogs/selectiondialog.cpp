/*
 *  Aethyra
 *  Copyright (C) 2010  Aethyra Development Team
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

#include <guichan/font.hpp>

#include "selectiondialog.h"

#include "../handlers/wordtextwraphandler.h"

#include "../widgets/button.h"
#include "../widgets/container.h"
#include "../widgets/radiobutton.h"
#include "../widgets/textbox.h"

#include "../../../core/utils/gettext.h"
#include "../../../core/utils/stringutils.h"

int SelectionDialog::mInstances = 0;

SelectionDialog::SelectionDialog(const std::string &title, const std::string &msg,
                                 Window *parent, bool modal):
    Window(title, modal, parent)
{
    // Create a globally unique id for this dialog
    mDialogKey = strprintf("selection%d", mInstances);
    mInstances++;

    mTextBox = new TextBox(new WordTextWrapHandler());
    mTextBox->setEditable(false);
    mTextBox->setOpaque(false);
    mTextBox->setTextWrapped(msg, 260);

    mOkButton = new Button(_("OK"), "ok", this);

    fontChanged();

    add(mTextBox);
    add(mOkButton);

    if (getParent())
    {
        setLocationRelativeTo(getParent());
        getParent()->moveToTop(this);
    }

    // Don't set the dialog to visible until there's an option to select
}

void SelectionDialog::fontChanged()
{
    Window::fontChanged();
    reflow();
}

void SelectionDialog::reflow()
{
    const int numRows = mTextBox->getNumberOfRows();
    const int fontHeight = getFont()->getHeight();
    const int radioHeight = (numRows * fontHeight) + getPadding();
    const int height = getNumRows() * fontHeight;
    int width = getFont()->getWidth(getCaption());

    if (width < mTextBox->getMinWidth())
        width = mTextBox->getMinWidth();
    if (width < mOkButton->getWidth())
        width = mOkButton->getWidth();

    setContentSize(mTextBox->getMinWidth() + fontHeight, height + fontHeight +
                   mOkButton->getHeight());
    mTextBox->setPosition(getPadding(), getPadding());

    for (size_t i = 0; i < mRadioButtons.size(); i++)
    {
        mRadioButtons[i]->setPosition(getPadding(), radioHeight + (i *
                                      fontHeight));
    }

    mOkButton->setPosition((width - mOkButton->getWidth()) / 2, height +
                           (2 * mOkButton->getSpacing()));
}

void SelectionDialog::close()
{
    Window::close();
    windowContainer->scheduleDelete(this);
}

unsigned int SelectionDialog::getNumRows()
{
    return (mTextBox->getNumberOfRows() + mRadioButtons.size());
}

void SelectionDialog::action(const gcn::ActionEvent &event)
{
    // Proxy button events to our listeners
    ActionListenerIterator i;
    for (i = mActionListeners.begin(); i != mActionListeners.end(); ++i)
        (*i)->action(event);

    // Can we receive anything else anyway?
    if (event.getId() == "ok")
    {
        for (size_t i = 0; i < mRadioButtons.size(); i++)
        {
            if (mRadioButtons[i]->isSelected())
            {
                setActionEventId(mRadioButtons[i]->getActionEventId());
                distributeActionEvent();
                break;
            }
        }
        close();
    }
}

void SelectionDialog::addOption(std::string key, std::string label)
{
    RadioButton *radio = new RadioButton(label, mDialogKey, false);
    radio->setActionEventId(key);
    mRadioButtons.push_back(radio);
    add(radio);
    reflow();

    // Set to visible now that there's an option to select
    setVisible(true);

    mRadioButtons[0]->setSelected(true);
    mOkButton->requestFocus();
}
