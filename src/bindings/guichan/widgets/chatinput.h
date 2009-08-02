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

#ifndef CHATINPUT_H
#define CHATINPUT_H

#include <list>

#include <guichan/focuslistener.hpp>

#include "textfield.h"

class ProxyWidget;

/**
 * The chat input hides when it loses focus. It is also invisible by default.
 */
class ChatInput : public TextField, public gcn::FocusListener
{
    public:
        /**
         * Constructor.
         */
        ChatInput();

        void setProxy(ProxyWidget* proxy) { mProxy = proxy; }

        /**
         * Called if the chat input gains focus. If a proxy class is available
         * to this input, disable it while focused.
         */
        void focusGained(const gcn::Event &event);

        /** Push a message into the history */
        void pushToHistory(const std::string &message);

        /**
         * Called if the chat input loses focus. It will set itself to
         * invisible as result.
         */
        void focusLost(const gcn::Event &event);

        /** Called when key is pressed */
        void keyPressed(gcn::KeyEvent &event);

        /** Scrolls through the chat history */
        void mouseWheelMovedUp(gcn::MouseEvent& mouseEvent);
        void mouseWheelMovedDown(gcn::MouseEvent& mouseEvent);

    private:
        ProxyWidget* mProxy;

        typedef std::list<std::string> History;
        typedef History::iterator HistoryIterator;
        History mHistory;           /**< Command history */
        HistoryIterator mCurHist;   /**< History iterator */
};

#endif
