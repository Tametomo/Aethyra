/*
 *  Extended support for activating emotes
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

#include <guichan/font.hpp>
#include <guichan/mouseinput.hpp>
#include <guichan/selectionlistener.hpp>

#include "emotecontainer.h"

#include "../graphics.h"
#include "../gui.h"

#include "../sdl/sdlinput.h"

#include "../../../configuration.h"
#include "../../../emoteshortcut.h"
#include "../../../log.h"

#include "../../../gui/popupmenu.h"

#include "../../../resources/image.h"
#include "../../../resources/resourcemanager.h"

#include "../../../resources/db/emotedb.h"

#include "../../../resources/sprite/animatedsprite.h"
#include "../../../resources/sprite/localplayer.h"

#include "../../../utils/stringutils.h"

const int EmoteContainer::gridWidth = 34;  // emote icon width + 4
const int EmoteContainer::gridHeight = 36; // emote icon height + 4

static const int NO_EMOTE = -1;

EmoteContainer::EmoteContainer(const std::string &actionEventId,
                               gcn::ActionListener *listener):
    mSelectedEmoteIndex(NO_EMOTE)
{
    if (!actionEventId.empty())
        setActionEventId(actionEventId);

    if (listener && !actionEventId.empty())
        addActionListener(listener);

    setFocusable(true);

    mPopupMenu = new PopupMenu(EMOTE);

    ResourceManager *resman = ResourceManager::getInstance();

    // Setup emote sprites
    for (int i = 0; i <= EmoteDB::getLast(); i++)
    {
        mEmoteImg.push_back(EmoteDB::getAnimation(i));
    }

    if (mEmoteImg.size() == 0)
        setEnabled(false);

    mSelImg = resman->getImage("graphics/gui/selection.png");
    if (!mSelImg)
        logger->error("Unable to load selection.png");

    mSelImg->setAlpha(config.getValue("guialpha", 0.8));

    mMaxEmote = EmoteDB::getLast() + 1;

    addKeyListener(this);
    addMouseListener(this);
    addWidgetListener(this);
}

EmoteContainer::~EmoteContainer()
{
    if (mSelImg)
       mSelImg->decRef();

    delete mPopupMenu;
}

void EmoteContainer::draw(gcn::Graphics *graphics)
{
    int columns = getWidth() / gridWidth;
    int emoteCount = 0;

    // Have at least 1 column
    if (columns < 1)
        columns = 1;

    for (int i = 0; i < mMaxEmote ; i++)
    {
        const int emoteX = ((i) % columns) * gridWidth;
        const int emoteY = ((i) / columns) * gridHeight;

        // Draw selection image below selected emote
        if (mSelectedEmoteIndex == i)
        {
            static_cast<Graphics*>(graphics)->drawImage(
                    mSelImg, emoteX, emoteY + 8);
        }

        // Draw emote icon
        if (mEmoteImg[i])
        {
            emoteCount++;
            mEmoteImg[i]->draw(static_cast<Graphics*>(graphics), emoteX, emoteY);
        }
    }

    if (!emoteCount)
        selectNone();
}

void EmoteContainer::widgetResized(const gcn::Event &event)
{
    recalculateHeight();
}

void EmoteContainer::recalculateHeight()
{
    int cols = getWidth() / gridWidth;

    if (cols < 1)
        cols = 1;

    const int rows = (mMaxEmote / cols) + (mMaxEmote % cols > 0 ? 1 : 0);
    const int height = rows * gridHeight + 8;
    if (height != getHeight())
        setHeight(height);
}

int EmoteContainer::getSelectedEmote()
{
    return 1 + mSelectedEmoteIndex;
}

void EmoteContainer::selectNone()
{
    setSelectedEmoteIndex(NO_EMOTE);
}

void EmoteContainer::setSelectedEmoteIndex(int index)
{
    if (index < 0 || index >= mMaxEmote )
        mSelectedEmoteIndex = NO_EMOTE;
    else
        mSelectedEmoteIndex = index;

    gcn::Rectangle scroll;
    const int columns = getWidth() / gridWidth;
    const int emoteY = mSelectedEmoteIndex / columns;

    if (mSelectedEmoteIndex == NO_EMOTE)
        scroll.y = 0;
    else
        scroll.y = gridHeight * emoteY;

    scroll.height = gridHeight;
    showPart(scroll);

    distributeValueChangedEvent();
}

void EmoteContainer::distributeValueChangedEvent()
{
    gcn::SelectionEvent event(this);
    std::list<gcn::SelectionListener*>::iterator i_end = mListeners.end();
    std::list<gcn::SelectionListener*>::iterator i;

    for (i = mListeners.begin(); i != i_end; ++i)
    {
        (*i)->valueChanged(event);
    }
}

void EmoteContainer::showPopup(bool useMouseCoordinates)
{
    const int emote = mSelectedEmoteIndex;
    int x = gui->getMouseX();
    int y = gui->getMouseY();

    if (emote == NO_EMOTE)
        return;

    if (!useMouseCoordinates)
    {
        int columns = getWidth() / gridWidth;
        const int emoteX = emote % columns;
        const int emoteY = emote / columns;
        const int xPos = emoteX * gridWidth + (gridWidth / 2);
        const int yPos = emoteY * gridHeight + (gridHeight / 2) + gui->getFont()->getHeight();

        x = xPos;
        y = yPos;

        gcn::Widget *widget = this;

        while (widget)
        {
            x += widget->getX();
            y += widget->getY();

            widget = widget->getParent();
        }
    }

    mPopupMenu->setEmote(getSelectedEmote());
    mPopupMenu->showPopup(x, y);
}

void EmoteContainer::keyPressed(gcn::KeyEvent &event)
{
    int columns = getWidth() / gridWidth;
    const int rows = mMaxEmote / columns;
    const int emoteX = mSelectedEmoteIndex % columns;
    const int emoteY = mSelectedEmoteIndex / columns;

    if (columns > mMaxEmote)
        columns = mMaxEmote;

    switch (event.getKey().getValue())
    {
        case Key::LEFT:
            if (emoteX != 0)
                setSelectedEmoteIndex((emoteY * columns) + emoteX - 1);
            break;
        case Key::RIGHT:
            if (emoteX < (columns - 1) &&
               ((emoteY * columns) + emoteX + 1) < mMaxEmote)
                setSelectedEmoteIndex((emoteY * columns) + emoteX + 1);
            break;
        case Key::UP:
            if (emoteY != 0)
                setSelectedEmoteIndex(((emoteY - 1) * columns) + emoteX);
            break;
        case Key::DOWN:
            if (emoteY < rows &&
               (((emoteY + 1) * columns) + emoteX) < mMaxEmote)
                setSelectedEmoteIndex(((emoteY + 1) * columns) + emoteX);
            break;
        case Key::ENTER:
        case Key::SPACE:
            if (event.isShiftPressed())
            {
                const std::string actionEventId = getActionEventId();
                setActionEventId("default");
                distributeActionEvent();
                setActionEventId(actionEventId);
            }
            else
                distributeActionEvent();
            break;
    }
}

void EmoteContainer::mousePressed(gcn::MouseEvent &event)
{
    int button = event.getButton();

    if (button == gcn::MouseEvent::LEFT || button == gcn::MouseEvent::RIGHT)
    {
        int columns = getWidth() / gridWidth;
        int mx = event.getX();
        int my = event.getY();
        int index = mx / gridWidth + ((my / gridHeight) * columns);

        if (index < mMaxEmote)
        {
            setSelectedEmoteIndex(index);
            emoteShortcut->setEmoteSelected(index + 1);
        }
    }
}
