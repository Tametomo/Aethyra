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

#include "chatinput.h"
#include "proxywidget.h"

#include "../sdl/sdlinput.h"

ChatInput::ChatInput() :
    TextField(),
    mProxy(NULL)
{
    setVisible(false);
    addFocusListener(this);
    mCurHist = mHistory.end();
}

void ChatInput::focusGained(const gcn::Event &event)
{
    if (mProxy)
        mProxy->setFocusable(false);
}

void ChatInput::focusLost(const gcn::Event &event)
{
    if (mProxy)
        mProxy->setFocusable(true);

    setVisible(false);
}

void ChatInput::pushToHistory(const std::string &message)
{
    // If message different from previous, put it in the history
    if (mHistory.empty() || message != mHistory.back())
        mHistory.push_back(message);

    // Reset history iterator
        mCurHist = mHistory.end();
}

void ChatInput::keyPressed(gcn::KeyEvent &event)
{
    TextField::keyPressed(event);

    if (event.getKey().getValue() == Key::DOWN && mCurHist != mHistory.end())
    {
        // Move forward through the history
        HistoryIterator prevHist = mCurHist++;

        if (mCurHist != mHistory.end())
        {
            setText(*mCurHist);
            setCaretPosition(getText().length());
        }
        else
            mCurHist = prevHist;
    }
    else if (event.getKey().getValue() == Key::UP && mCurHist !=
             mHistory.begin() && mHistory.size() > 0)
    {
        // Move backward through the history
        mCurHist--;
        setText(*mCurHist);
        setCaretPosition(getText().length());
    }
}

void ChatInput::mouseWheelMovedUp(gcn::MouseEvent& mouseEvent)
{
    TextField::mouseWheelMovedUp(mouseEvent);

    if (isFocused() && mCurHist != mHistory.begin() && mHistory.size() > 0)
    {
        // Move backward through the history
        mCurHist--;
        setText(*mCurHist);
        setCaretPosition(getText().length());
        mouseEvent.consume();
    }
}

void ChatInput::mouseWheelMovedDown(gcn::MouseEvent& mouseEvent)
{
    TextField::mouseWheelMovedDown(mouseEvent);

    if (isFocused() && mCurHist != mHistory.end())
    {
        // Move forward through the history
        HistoryIterator prevHist = mCurHist++;

        if (mCurHist != mHistory.end())
        {
            setText(*mCurHist);
            setCaretPosition(getText().length());
        }
        else
            mCurHist = prevHist;

        mouseEvent.consume();
    }
}
