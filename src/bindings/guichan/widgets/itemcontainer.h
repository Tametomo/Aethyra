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

#ifndef ITEMCONTAINER_H
#define ITEMCONTAINER_H

#include <list>
#include <string>

#include <guichan/keylistener.hpp>
#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>
#include <guichan/widgetlistener.hpp>

#include "../guichanfwd.h"

#include "../../../gui/popupmenu.h"

class Image;
class Inventory;
class Item;
class ItemContainerConfigListener;
class ItemPopup;

/**
 * An item container. Used to show items in inventory and trade dialog.
 *
 * \ingroup GUI
 */
class ItemContainer : public gcn::Widget, gcn::KeyListener, gcn::MouseListener,
                             gcn::WidgetListener
{
    public:
        /**
         * Constructor. Initializes the graphic.
         */
        ItemContainer(Inventory *inventory, const std::string &actionEventId = "",
                      gcn::ActionListener *listener = NULL);

        /**
         * Destructor.
         */
        virtual ~ItemContainer();

        /**
         * Handles the logic of the ItemContainer
         */
        void logic();

        /**
         * Draws the items.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Called whenever the widget changes size.
         */
        void widgetResized(const gcn::Event &event);

        /**
         * Handles mouse click.
         */
        void mousePressed(gcn::MouseEvent &event);

        /**
         * Returns the selected item.
         */
        Item* getSelectedItem();

        /**
         * Sets selected item to NULL.
         */
        void selectNone();

        /**
         * Adds a listener to the list that's notified each time a change to
         * the selection occurs.
         */
        void addSelectionListener(gcn::SelectionListener *listener)
        {
            mListeners.push_back(listener);
        }

        /**
         * Removes a listener from the list that's notified each time a change
         * to the selection occurs.
         */
        void removeSelectionListener(gcn::SelectionListener *listener)
        {
            mListeners.remove(listener);
        }

        /**
         * Shows a PopupMenu over the selected item.
         */
        void showPopupMenu(MenuType type, bool useMouseCoordinates = true);

        /**
         * Sets whether the item popup should be shown or not.
         */
        void enableItemPopup(bool enable);

        /**
         * Changes the item popup's visibility.
         */
        void setItemPopupVisibility(bool visible);

        /**
         * Display only items of a given type.
         * Clear the filter by setting it to "".
         */
        void setTypeFilter(const std::string& type);

    private:
        // KeyListener
        void keyPressed(gcn::KeyEvent &event);

        void mouseExited(gcn::MouseEvent &event);
        void mouseMoved(gcn::MouseEvent &event);

        /**
         * Gets the location to tell a popup to draw, and then store it in
         * x and y. The useMouseCoordinates boolean determines whether the
         * selected item index should be used, or the mouse coordinates.
         */
        void getPopupLocation(bool useMouseCoordinates, int &x, int &y);

        /**

         * Sets the currently selected item.  Invalid (e.g., negative) indices set `no item'.
         */
        void setSelectedItemIndex(int index);

        /**
         * Find the current item index by the most recently used item ID
         */
        void refindSelectedItem(void);

        /**
         * Determine and set the height of the container.
         */
        void recalculateHeight(void);

        /**
         * Sends out selection events to the list of selection listeners.
         */
        void distributeValueChangedEvent(void);

        /**
         * Gets the item at the cursor position.
         * (The returned item is owned by mInventory).
         *
         * @param posX The X Coordinate position (pixels).
         * @param posY The Y Coordinate position (pixels).
         * @return The item on success, NULL on failure.
         */
        Item* getItem(const int posX, const int posY);

        /**
         * Gets the item in a visible slot.  This only finds items
         * that can be seen with the current filter.
         * (The returned item is owned by mInventory).
         *
         * Will handle gridSlot being out of range (returns NULL).
         *
         * @param gridSlot The position (in slots, Y*columns+X).
         * @return The item on success, NULL on failure.
         */
        Item* getItemInVisibleSlot(const int gridSlot);

        /**
         * Finds the position where this item would be drawn,
         * taking account of any active filter.  The caller must convert this
         * back to rows and columns.
         *
         * @param searchItem The item to find.
         * @return The position if this item is visible, otherwise -1.
         */
        int getVisibleSlotForItem(const Item* searchItem) const;

        /**
         * Whether the item should be shown.
         */
        bool passesFilter(const Item* item) const;

        ItemContainerConfigListener *mConfigListener;

        Inventory *mInventory;
        Image *mSelImg;

        bool mShowItemInfo;

        int mSelectedItemIndex;
        int mLastSelectedItemId;  // last selected item ID. If we lose the item, find again by ID.
        int mMaxItems;
        int mOffset;

        ItemPopup *mItemPopup;
        PopupMenu *mPopupMenu;

        std::list<gcn::SelectionListener*> mListeners;

        static const int gridWidth;
        static const int gridHeight;

        std::string mTypeFilter;    // if not "", display only items of this type
};

#endif
