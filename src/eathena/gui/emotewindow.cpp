/*
 *  Extended support for activating emotes
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

#include "emotewindow.h"

#include "../db/emotedb.h"

#include "../handlers/emoteshortcut.h"

#include "../widgets/emotecontainer.h"

#include "../../bindings/guichan/gui.h"
#include "../../bindings/guichan/layout.h"

#include "../../bindings/guichan/widgets/button.h"
#include "../../bindings/guichan/widgets/scrollarea.h"

#include "../../core/map/sprite/localplayer.h"

#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

extern Window *emoteShortcutWindow;

EmoteWindow::EmoteWindow():
    Window(_("Emote"))
{
    setWindowName("Emote");
    setResizable(true);
    setCloseButton(true);
    setMinWidth(80);
    setMinHeight(130);
    setDefaultSize(322, 200, ImageRect::CENTER);

    mUseButton = new Button(_("Use"), "use", this);
    mUseButton->setEnabled(false);

    mEmotes = new EmoteContainer("showpopup", this);
    mEmotes->addSelectionListener(this);

    mShortcutButton = new Button(_("Shortcuts"), "shortcuts", this);

    mEmoteScroll = new ScrollArea(mEmotes);
    mEmoteScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    fontChanged();
    loadWindowState();
}

void EmoteWindow::fontChanged()
{
    Window::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    place(0, 0, mEmoteScroll, 5, 4);
    place(0, 4, mShortcutButton);
    place(4, 4, mUseButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    restoreFocus();
}

void EmoteWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "use" || event.getId() == "default")
    {
        int emote = mEmotes->getSelectedEmote();

        if (emote == -1)
            return;

        player_node->emote(emote);
    }
    else if (event.getId() == "showpopup")
        mEmotes->showPopup(false);
    else if (event.getId() == "shortcuts")
    {
        emoteShortcutWindow->setVisible(!emoteShortcutWindow->isVisible());

        if (emoteShortcutWindow->isVisible())
            emoteShortcutWindow->requestMoveToTop();
    }
}

int EmoteWindow::getSelectedEmote() const
{
    return mEmotes->getSelectedEmote();
}

void EmoteWindow::valueChanged(const gcn::SelectionEvent &event)
{
    if (event.getSource() == mEmotes)
    {
        mUseButton->setEnabled(mEmotes->getSelectedEmote() != 0);

        const int index = mEmotes->getSelectedEmote();

        if (index > 0 && index <= (EmoteDB::getLast() + 1))
            emoteShortcut->setSelected(index);
        else
            emoteShortcut->setSelected(-1);
    }
}

void EmoteWindow::mouseClicked(gcn::MouseEvent &event)
{
    Window::mouseClicked(event);

    if (event.getButton() == gcn::MouseEvent::RIGHT &&
        event.getSource() == mEmotes)
        mEmotes->showPopup();
}

void EmoteWindow::requestFocus()
{
    Window::requestFocus();

    mEmotes->requestFocus();
}

void EmoteWindow::widgetHidden(const gcn::Event& event)
{
    Window::widgetHidden(event);
    mEmotes->selectNone();
}
