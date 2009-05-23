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

#include "textinputdialog.h"

#include "../layout.h"

#include "../widgets/button.h"
#include "../widgets/textfield.h"

#include "../../../utils/gettext.h"

TextInputDialog::TextInputDialog(std::string caption):
    Window(caption)
{
    setWindowName("TextInputDialog");

    mValueField = new TextField("", "ok", this);

    setDefaultSize(175, 75, ImageRect::CENTER);

    mOkButton = new Button(_("OK"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    place(0, 0, mValueField, 3);
    place(1, 1, mCancelButton);
    place(2, 1, mOkButton);

    loadWindowState();
    setVisible(false);
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
    // Proxy button events to our listeners
    ActionListenerIterator i;

    for (i = mActionListeners.begin(); i != mActionListeners.end(); ++i)
        (*i)->action(event);
}

void TextInputDialog::requestFocus()
{
    mValueField->requestFocus();
}

