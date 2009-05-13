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

#include "proxywidget.h"

ProxyWidget::ProxyWidget(gcn::Widget *widget, ProxyAction action) :
    gcn::Widget(),
    mActingWidget(widget),
    mAction(action)
{
    setFocusable(true);
    addFocusListener(this);

    setSize(0, 0);
}

void ProxyWidget::focusGained(const gcn::Event &event)
{
    if (!mActingWidget || !isVisible())
        return;

    if (mAction == FOCUS_WHEN_HIDDEN && event.getSource() == this &&
        mActingWidget->getParent()->isVisible())
    {
        mActingWidget->setVisible(true);
        mActingWidget->requestFocus();
    }
}
