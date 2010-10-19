/*
 *  Aethyra
 *  Copyright (c) 2004 - 2008 Olof Naessén and Per Larsson
 *  Copyright (C) 2010  Aethyra Development Team
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

#include <guichan/focushandler.hpp>

#include "container.h"
#include "tab.h"
#include "tabbedarea.h"

#include "../protectedfocuslistener.h"

#include "../sdl/sdlinput.h"

#include "../../../core/utils/dtor.h"

TabbedArea::TabbedArea() :
    Container(),
    mSelectedTab(NULL)
{
    setFocusable(true);
    addKeyListener(this);
    addMouseListener(this);

    mTabContainer = new Container();
    mTabContainer->setOpaque(false);
    mWidgetContainer = new Container();
    mWidgetContainer->setOpaque(false);

    add(mTabContainer);
    add(mWidgetContainer);

    mProtFocusListener = new ProtectedFocusListener();

    addFocusListener(mProtFocusListener);

    mProtFocusListener->blockKey(SDLK_LEFT);
    mProtFocusListener->blockKey(SDLK_RIGHT);
    mProtFocusListener->blockKey(SDLK_UP);
    mProtFocusListener->blockKey(SDLK_DOWN);
}

TabbedArea::~TabbedArea()
{
    TabContainer::iterator itr = mTabs.begin(), itr_end = mTabs.end();
    while (itr != itr_end)
    {
        itr->first->setTabbedArea(NULL);
        ++itr;
    }

    delete_all(mTabsToDelete);
    mTabContainer->clear(); // Avoid deleting tabs which this class didn't create
    mWidgetContainer->clear();

    if (mFocusHandler && mFocusHandler->isFocused(this))
        mFocusHandler->focusNone();

    removeFocusListener(mProtFocusListener);
    destroy(mProtFocusListener);
}

void TabbedArea::logic()
{
    if (!isVisible())
        return;

    gcn::BasicContainer::logic();
}


void TabbedArea::addTab(const std::string &caption, gcn::Widget *widget)
{
    Tab* tab = new Tab(caption);
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

void TabbedArea::removeTab(Tab *tab)
{
    // If the tab isn't contained in this tabbed area, ignore the removal
    // request
    if (!contains(tab))
        return;

    // Ensure that we only permanently delete tabs which we created locally
    std::list<Tab*>::iterator itr = mTabsToDelete.begin(),
                              itr_end = mTabsToDelete.end();
    while (itr != itr_end)
    {
        if (*itr == tab)
        {
            mTabContainer->scheduleDelete(tab);
            mTabsToDelete.remove(*itr); // Remove the tab from the to
                                        // delete list, since the container
                                        // is taking care of this
            break;
        }

        ++itr;
    }

    // Change the selected tab if the tab to be deleted is currently selected
    TabContainer::iterator tabItr = getTabIterator(tab);
    if (tab == mSelectedTab)
    {
        if (tabItr != mTabs.end())
        {
            setSelectedTab((*(++tabItr)).first);
            --tabItr;
        }
        else if (mTabs.size() > 1)
            setSelectedTab((*(--mTabs.end())).first);
        else
            setSelectedTab(NULL);
    }

    tabItr = mTabs.erase(tabItr);

    // Fix tab positioning
    fontChanged();
}

void TabbedArea::setSelectedTab(Tab* tab)
{
    if (mSelectedTab)
        mWidgetContainer->remove(getWidget(mSelectedTab->getCaption()));

    // Check if the tab is in this tabbed area
    if (contains(tab))
        mSelectedTab = tab;

    if (mSelectedTab)
        mWidgetContainer->add(getWidget(mSelectedTab->getCaption()));
}

Tab* TabbedArea::getTab(const std::string &name)
{
    TabContainer::iterator itr = mTabs.begin(), itr_end = mTabs.end();
    while (itr != itr_end)
    {
        if (itr->first->getCaption() == name)
            return itr->first;

        ++itr;
    }
    return NULL;
}

gcn::Widget* TabbedArea::getWidget(const std::string &name)
{
    TabContainer::iterator itr = mTabs.begin(), itr_end = mTabs.end();
    while (itr != itr_end)
    {
        if (itr->first->getCaption() == name)
            return itr->second;

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

bool TabbedArea::contains(Tab *tab)
{
    TabContainer::iterator itr = mTabs.begin(), itr_end = mTabs.end();
    bool contained = false;

    while (itr != itr_end)
    {
        if (itr->first == tab)
        {
            contained = true;
            break;
        }

        ++itr;
    }

    return contained;
}

void TabbedArea::setWidth(int width)
{
    // This may seem odd, but we want the TabbedArea to adjust
    // it's size properly before we call Widget::setWidth as
    // Widget::setWidth might distribute a resize event.
    gcn::Rectangle dim = mDimension;
    mDimension.width = width;
    adjustSize();
    mDimension = dim;
    gcn::Widget::setWidth(width);
}

void TabbedArea::setHeight(int height)
{
    // This may seem odd, but we want the TabbedArea to adjust
    // it's size properly before we call Widget::setHeight as
    // Widget::setHeight might distribute a resize event.
    gcn::Rectangle dim = mDimension;
    mDimension.height = height;
    mDimension = dim;
    gcn::Widget::setHeight(height);
}

void TabbedArea::setSize(int width, int height)
{
    // This may seem odd, but we want the TabbedArea to adjust
    // it's size properly before we call Widget::setSize as
    // Widget::setSize might distribute a resize event.
    gcn::Rectangle dim = mDimension;
    mDimension.width = width;
    mDimension.height = height;
    adjustSize();
    mDimension = dim;
    gcn::Widget::setSize(width, height);
}

void TabbedArea::setDimension(const gcn::Rectangle& dimension)
{
    // This may seem odd, but we want the TabbedArea to adjust
    // it's size properly before we call Widget::setDimension as
    // Widget::setDimension might distribute a resize event.
    gcn::Rectangle dim = mDimension;
    mDimension = dimension;
    adjustSize();
    mDimension = dim;
    gcn::Widget::setDimension(dimension);      
}

void TabbedArea::fontChanged()
{
    gcn::BasicContainer::fontChanged();

    adjustTabPositions();
    adjustSize();
}

void TabbedArea::action(const gcn::ActionEvent& actionEvent)
{
    gcn::Widget* source = actionEvent.getSource();
    Tab* tab = dynamic_cast<Tab*>(source);

    if (tab != NULL)
        setSelectedTab(tab);
}

void TabbedArea::keyPressed(gcn::KeyEvent& keyEvent)
{
    if (keyEvent.isConsumed() || !isFocused())
        return;

    if (keyEvent.getKey().getValue() == Key::LEFT)
    {
        TabContainer::iterator index = getTabIterator(mSelectedTab);

        if (index != mTabs.begin())
            setSelectedTab((--index)->first);
        else
            setSelectedTab((--mTabs.end())->first);

        keyEvent.consume();
    }
    else if (keyEvent.getKey().getValue() == Key::RIGHT)
    {
        TabContainer::iterator index = getTabIterator(mSelectedTab);

        if (index != (--mTabs.end()))
            setSelectedTab((++index)->first);
        else
            setSelectedTab(mTabs.begin()->first);

        keyEvent.consume();
    }
}

void TabbedArea::mousePressed(gcn::MouseEvent &mouseEvent)
{
    if (mouseEvent.isConsumed())
        return;

    if (mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
        gcn::Widget* widget = mTabContainer->getWidgetAt(mouseEvent.getX(),
                                                         mouseEvent.getY());
        Tab* tab = dynamic_cast<Tab*>(widget);

        if (tab != NULL)
        {
            setSelectedTab(tab);
            requestFocus();
        }
    }
}

TabbedArea::TabContainer::iterator TabbedArea::getTabIterator(Tab *tab)
{
    TabContainer::iterator itr = mTabs.begin(), itr_end = mTabs.end(),
                           itr_ret = itr_end;

    while (itr != itr_end)
    {
        if (itr->first == tab)
        {
            itr_ret = itr;
            break;
        }

        ++itr;
    }

    return itr;
}

int TabbedArea::getMaxTabHeight()
{
    int maxTabHeight = 0;

    TabContainer::iterator itr = mTabs.begin(), itr_end = mTabs.end();
    while (itr != itr_end)
    {
        if (itr->first->getHeight() > maxTabHeight)
            maxTabHeight = itr->first->getHeight();

        ++itr;
    }

    return maxTabHeight;
}

void TabbedArea::adjustSize()
{
    int maxTabHeight = getMaxTabHeight();

    mTabContainer->setSize(getWidth() - 2, maxTabHeight);
    mWidgetContainer->setPosition(1, maxTabHeight + 1);
    mWidgetContainer->setSize(getWidth() - 2, getHeight() - maxTabHeight - 2);
}

void TabbedArea::adjustTabPositions()
{
    int maxTabHeight = getMaxTabHeight();
    int x = 0;

    TabContainer::iterator itr = mTabs.begin(), itr_end = mTabs.end();
    while (itr != itr_end)
    {
         Tab* tab = itr->first;
         tab->setPosition(x, maxTabHeight - tab->getHeight());
         x += tab->getWidth();

         itr++;
    }
}

