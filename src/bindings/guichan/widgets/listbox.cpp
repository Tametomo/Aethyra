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
#include <guichan/graphics.hpp>
#include <guichan/key.hpp>
#include <guichan/listmodel.hpp>

#include "listbox.h"

#include "../palette.h"

#include "../sdl/sdlinput.h"

#include "../../../configuration.h"

float ListBox::mAlpha = 1.0;

ListBox::ListBox(gcn::ListModel *listModel, const std::string &actionEventId,
                 gcn::ActionListener *listener):
    gcn::ListBox(listModel)
{
    if (!actionEventId.empty())
        setActionEventId(actionEventId);

    if (listener && !actionEventId.empty())
        addActionListener(listener);
}

void ListBox::draw(gcn::Graphics *graphics)
{
    if (!mListModel)
        return;

    if (config.getValue("guialpha", 0.8) != mAlpha)
        mAlpha = config.getValue("guialpha", 0.8);

    graphics->setColor(guiPalette->getColor(Palette::HIGHLIGHT,
            (int)(mAlpha * 255.0f)));
    graphics->setFont(getFont());

    const int fontHeight = getFont()->getHeight();

    // Draw filled rectangle around the selected list element
    if (mSelected >= 0)
        graphics->fillRectangle(gcn::Rectangle(0, fontHeight * mSelected,
                                               getWidth(), fontHeight));

    // Draw the list elements
    graphics->setColor(guiPalette->getColor(Palette::TEXT));
    for (int i = 0, y = 0; i < mListModel->getNumberOfElements();
         ++i, y += fontHeight)
    {
        graphics->drawText(mListModel->getElementAt(i), 1, y);
    }
}

void ListBox::incrementSelected()
{
    const int lastSelection = getListModel()->getNumberOfElements() - 1;

    if (getSelected() < lastSelection)
        setSelected(mSelected + 1);
    else if (getSelected() == lastSelection && mWrappingEnabled)
        setSelected(0);
}

void ListBox::decrementSelected()
{
    const int lastSelection = getListModel()->getNumberOfElements() - 1;

    if (getSelected() > 0)
        setSelected(mSelected - 1);
    else if (getSelected() == 0 && mWrappingEnabled)
        setSelected(lastSelection);
}

// -- KeyListener notifications
void ListBox::keyPressed(gcn::KeyEvent& keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ENTER || key.getValue() == Key::SPACE)
        distributeActionEvent();
    else if (key.getValue() == Key::UP)
        decrementSelected();
    else if (key.getValue() == Key::DOWN)
        incrementSelected();
    else if (key.getValue() == Key::HOME)
        setSelected(0);
    else if (key.getValue() == Key::END)
        setSelected(getListModel()->getNumberOfElements() - 1);
    else
        return;

    keyEvent.consume();
}

void ListBox::mouseWheelMovedUp(gcn::MouseEvent& mouseEvent)
{
    if (isFocused())
    {
        decrementSelected();
        mouseEvent.consume();
    }
}

void ListBox::mouseWheelMovedDown(gcn::MouseEvent& mouseEvent)
{
    if (isFocused())
    {
        incrementSelected();
        mouseEvent.consume();
    }
}

void ListBox::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() != gcn::MouseEvent::LEFT)
        return;

    // Make list selection update on drag, but guard against negative y
    int y = std::max(0, event.getY());
    setSelected(y / getRowHeight());
}
