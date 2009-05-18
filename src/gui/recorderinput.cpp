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

#include "recorderinput.h"

#include "../bindings/guichan/layout.h"

#include "../bindings/guichan/widgets/button.h"
#include "../bindings/guichan/widgets/label.h"
#include "../bindings/guichan/widgets/textfield.h"

#include "../utils/gettext.h"

RecorderInput::RecorderInput(std::string caption):
    Window(_("Recorder Input"))
{
    setWindowName("RecorderInput");

    mValueField = new TextField("", "ok", this);

    setDefaultSize(175, 90, ImageRect::CENTER);

    mCaption = new Label(_("File name to record to:"));
    mOkButton = new Button(_("OK"), "ok", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    place(0, 0, mCaption, 3);
    place(0, 1, mValueField, 3);
    place(1, 2, mCancelButton);
    place(2, 2, mOkButton);

    loadWindowState();
    setVisible(false);
}

std::string RecorderInput::getValue()
{
    return mValueField->getText();
}

void RecorderInput::setValue(const std::string &value)
{
    mValueField->setText(value);
}

void RecorderInput::reset()
{
    mValueField->setText("");
    requestFocus();
}

void RecorderInput::action(const gcn::ActionEvent &event)
{
    // Proxy button events to our listeners
    ActionListenerIterator i;
    for (i = mActionListeners.begin(); i != mActionListeners.end(); ++i)
    {
        (*i)->action(event);
    }
}

void RecorderInput::requestFocus()
{
    setVisible(true);
    mValueField->requestFocus();
}

void RecorderInput::close()
{
    Window::close();
}
