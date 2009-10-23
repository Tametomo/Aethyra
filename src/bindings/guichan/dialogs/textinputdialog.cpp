/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
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

#include "textinputdialog.h"

#include "../layout.h"

#include "../widgets/button.h"
#include "../widgets/textfield.h"

#include "../../../core/utils/gettext.h"

TextInputDialog::TextInputDialog(const std::string &caption):
    Window(caption)
{
    setWindowName("TextInputDialog");
    saveVisibility(false);

    mValueField = new TextField("", "ok", this);

    mOkButton = new Button(_("OK"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    fontChanged();
    loadWindowState();
}

void TextInputDialog::fontChanged()
{
    Window::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    place(0, 0, mValueField, 4);
    place(2, 1, mCancelButton);
    place(3, 1, mOkButton);

    adjustSize();
    restoreFocus();
}

void TextInputDialog::adjustSize()
{
    const int titleWidth = 3 * getFont()->getWidth(getCaption()) / 2;
    const int fontHeight = getFont()->getHeight();

    setWidth(titleWidth + 4 * getPadding());
    setHeight(fontHeight + mOkButton->getHeight() +
              mValueField->getHeight() + 8 * getPadding());

    setDefaultSize(getWidth(), getHeight(), ImageRect::CENTER);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);
    layout.setRowHeight(1, Layout::AUTO_SET);
    layout.setColWidth(2, Layout::AUTO_SET);
    layout.setColWidth(3, Layout::AUTO_SET);
}

std::string TextInputDialog::getValue()
{
    return mValueField->getText();
}

void TextInputDialog::setValue(const std::string &value)
{
    mValueField->setText(value);
}

void TextInputDialog::reset()
{
    mValueField->setText("");
}

void TextInputDialog::action(const gcn::ActionEvent &event)
{
    // Proxy events to our listeners
    ActionListenerIterator i;

    for (i = mActionListeners.begin(); i != mActionListeners.end(); ++i)
        (*i)->action(event);
}

void TextInputDialog::requestFocus()
{
    Window::requestFocus();

    mValueField->requestFocus();
}

void TextInputDialog::widgetShown(const gcn::Event& event)
{
    reset();
    Window::widgetShown(event);
}

