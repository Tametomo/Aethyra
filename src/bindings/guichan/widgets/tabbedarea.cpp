/*
 *  Aethyra
 *  Copyright (C) 2008  The Mana World Development Team
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

#include <guichan/focushandler.hpp>

#include <guichan/widgets/container.hpp>

#include "tab.h"
#include "tabbedarea.h"

#include "../protectedfocuslistener.h"

#include "../sdl/sdlinput.h"

TabbedArea::TabbedArea() :
    gcn::TabbedArea()
{
    mWidgetContainer->setOpaque(false);

    mProtFocusListener = new ProtectedFocusListener();

    addFocusListener(mProtFocusListener);

    mProtFocusListener->blockKey(SDLK_LEFT);
    mProtFocusListener->blockKey(SDLK_RIGHT);
    mProtFocusListener->blockKey(SDLK_UP);
    mProtFocusListener->blockKey(SDLK_DOWN);
}

TabbedArea::~TabbedArea()
{
    if (mFocusHandler && mFocusHandler->isFocused(this))
        mFocusHandler->focusNone();

    removeFocusListener(mProtFocusListener);
    delete mProtFocusListener;
}

int TabbedArea::getNumberOfTabs()
{
    return mTabs.size();
}

gcn::Tab* TabbedArea::getTab(const std::string &name)
{
    TabContainer::iterator itr = mTabs.begin(), itr_end = mTabs.end();
    while (itr != itr_end)
    {
        if ((*itr).first->getCaption() == name)
            return static_cast<Tab*>((*itr).first);

        ++itr;
    }
    return NULL;
}

void TabbedArea::draw(gcn::Graphics *graphics)
{
    if (mTabs.empty())
        return;

    drawChildren(graphics);
}

gcn::Widget* TabbedArea::getWidget(const std::string &name)
{
    TabContainer::iterator itr = mTabs.begin(), itr_end = mTabs.end();
    while (itr != itr_end)
    {
        if ((*itr).first->getCaption() == name)
            return (*itr).second;

        ++itr;
    }

    return NULL;
}

void TabbedArea::addTab(const std::string &caption, gcn::Widget *widget)
{
    Tab* tab = new Tab();
    tab->setCaption(caption);
    mTabsToDelete.push_back(tab);

    addTab(tab, widget);
}

void TabbedArea::addTab(Tab *tab, gcn::Widget *widget)
{
    tab->setTabbedArea(this);
    tab->addActionListener(this);

    mTabContainer->add(tab);
    mTabs.push_back(std::pair<Tab*, gcn::Widget*>(tab, widget));

    if (!mSelectedTab)
        setSelectedTab(tab);

    adjustTabPositions();
    adjustSize();
}

void TabbedArea::logic()
{
    if (!isVisible())
        return;

    logicChildren();
}

void TabbedArea::mousePressed(gcn::MouseEvent &mouseEvent)
{
    if (mouseEvent.isConsumed())
        return;

    if (mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
        gcn::Widget* widget = mTabContainer->getWidgetAt(mouseEvent.getX(),
                                                         mouseEvent.getY());
        gcn::Tab* tab = dynamic_cast<gcn::Tab*>(widget);

        if (tab != NULL)
        {
            setSelectedTab(tab);
            requestFocus();
        }
    }
}

void TabbedArea::keyPressed(gcn::KeyEvent& keyEvent)
{
    if (keyEvent.isConsumed() || !isFocused())
        return;

    if (keyEvent.getKey().getValue() == Key::LEFT)
    {
        int index = getSelectedTabIndex();
        index--;

        if (index < 0)
            setSelectedTab(mTabs[mTabs.size() - 1].first);
        else
            setSelectedTab(mTabs[index].first);

        keyEvent.consume();
    }
    else if (keyEvent.getKey().getValue() == Key::RIGHT)
    {
        int index = getSelectedTabIndex();
        index++;

        if (index >= (int) mTabs.size())
            setSelectedTab(mTabs[0].first);
        else
            setSelectedTab(mTabs[index].first);

        keyEvent.consume();
    }
}

void TabbedArea::fontChanged()
{
    gcn::TabbedArea::fontChanged();

    adjustTabPositions();
    adjustSize();
}
