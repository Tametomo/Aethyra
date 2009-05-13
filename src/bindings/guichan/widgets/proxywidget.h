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

#ifndef PROXYWIDGET_H
#define PROXYWIDGET_H

#include <guichan/focuslistener.hpp>
#include <guichan/widget.hpp>

enum ProxyAction {
    FOCUS_WHEN_HIDDEN = 1 // Also when not enabled. Currently, isFocusable()
                          // checks for both, which is great for most situations,
                          // but not for every situation. For instance, it would
                          // be great to be able to override isFocusable() if
                          // you had a hiding input bar on a window which
                          // hides when not used.
};

/**
 * A proxy widget is a widget which performs an action on another widget's
 * behalf that GUIChan currently won't allow. If you add a function here that
 * can't be done any other way, then the correct action would be to first make
 * absolutely sure there is no other way of doing it, then if you still are sure
 * it can't be done, notify the GUIChan team to see if you can get a patch to
 * allow for it in their next release, then get rid of the ProxyWidget when you
 * can do that action with GUIChan. Also please leave a comment next to each
 * action as you add it to describe why it's an issue.
 */
class ProxyWidget : public gcn::Widget, public gcn::FocusListener
{
    public:
        /**
         * Constructor.
         */
        ProxyWidget(gcn::Widget *widget, ProxyAction action);

        /**
         * Used to perform a specific ProxyAction. Use only when no listener
         * for your type of action exists.
         */
        void logic() {}

        /**
         * Overridden to be able to handle FOCUS_WHEN_HIDDEN actions without the
         * need of a logic poll.
         */
        void focusGained(const gcn::Event &event);

        void draw(gcn::Graphics* graphics) {}

    private:
        gcn::Widget *mActingWidget;
        ProxyAction mAction;
};

#endif
