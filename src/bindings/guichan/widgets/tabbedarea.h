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

#ifndef TABBEDAREA_H
#define TABBEDAREA_H

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/mouselistener.hpp>

#include <list>
#include <string>

#include "container.h"

#include "../guichanfwd.h"

class ProtectedFocusListener;
class Tab;

/**
 * A tabbed area. This is similar in API to the GUIChan tabbed area, but isn't
 * the same internally, because GUIChan's TabbedArea chose a poor matching STL
 * container, which causes problems for when you want to delete/insert tabs
 * anywhere but on the end of a tab container.
 */
class TabbedArea : public Container, public gcn::ActionListener,
                   public gcn::KeyListener, public gcn::MouseListener
{
    public:
        friend class Tab;

        /**
         * Constructor.
         */
        TabbedArea();

        virtual ~TabbedArea();

        /**
         * Do delayed deletion
         */
        void logic();

        /**
         * Add a tab
         * @param caption The Caption to display
         * @param widget The widget to show when tab is selected
         */
        void addTab(const std::string &caption, gcn::Widget *widget);

        /**
         * Add a tab
         * @param tab The tab
         * @param widget The widget to display
         */
        void addTab(Tab *tab, gcn::Widget *widget);

        /**
         * Overridden from GUIChan's removeTab method in order to ensure safe
         * deletion of tabs. Tabs removed through this function are added to a
         * death list, then cleaned up later, so as to not cause complications
         * when the particular tab's draw methods are called.
         */
        virtual void removeTab(Tab* tab);

        /**
         * Return how many tabs have been created
         */
        int getNumberOfTabs() const { return mTabs.size(); }

        /**
         * Checks if a tab is selected or not.
         */
        virtual bool isTabSelected(Tab* tab) const
            { return mSelectedTab == tab; }

        /**
         * Changes the selected tab.
         */
        virtual void setSelectedTab(Tab* tab);

        /**
         * Gets the selected tab.
         */
        Tab* getSelectedTab() const { return mSelectedTab; }

        /**
         * Return tab with specified name as caption
         */
        Tab* getTab(const std::string &name);

        /**
         * Returns the widget with the tab that has specified caption
         */
        gcn::Widget* getWidget(const std::string &name);

        /**
         * Whether the tab is in this tab container or not
         */
        bool contains(Tab *tab);

        /**
         * Draw the tabbed area.
         */
        void draw(gcn::Graphics *graphics);

        void setWidth(int width);

        void setHeight(int height);

        void setSize(int width, int height);

        void setDimension(const gcn::Rectangle& dimension);

        void fontChanged();


        // Inherited from ActionListener

        void action(const gcn::ActionEvent& actionEvent);

        // Inherited from KeyListener

        void keyPressed(gcn::KeyEvent &event);

        // Inherited from MouseListener

        void mousePressed(gcn::MouseEvent &mouseEvent);

    protected:
        typedef std::list<std::pair<Tab*, gcn::Widget*> > TabContainer;
        std::list<Tab*> mTabsToDelete;

        /**
         * Finds the tab iterator for a specific tab in this container.
         *
         * WARNING: This will only work properly if the tab is in the container.
         *          Use the contains() function to find out for sure before
         *          using this if there's any uncertainty.
         */
        TabContainer::iterator getTabIterator(Tab* tab);

        /**
         * Gets the maximum tab height used for all tabs in this tabbed area.
         */
        int getMaxTabHeight();

        /**
         * Adjusts the size of the tab container and the widget container.
         */
        void adjustSize();

        /**
         * Adjusts the positions of the tabs.
         */
        void adjustTabPositions();

        ProtectedFocusListener *mProtFocusListener;
        Container* mTabContainer;
        Container* mWidgetContainer;

        Tab *mSelectedTab;
        TabContainer mTabs;
};

#endif

