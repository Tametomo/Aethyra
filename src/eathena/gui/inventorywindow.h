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

#ifndef INVENTORYWINDOW_H
#define INVENTORYWINDOW_H

#include <guichan/actionlistener.hpp>
#include <guichan/selectionlistener.hpp>

#include "../structs/inventory.h"

#include "../../bindings/guichan/widgets/window.h"

class Item;
class ItemContainer;
class ProgressBar;

/**
 * Inventory dialog.
 *
 * \ingroup Interface
 */
class InventoryWindow : public Window, gcn::ActionListener,
                                       gcn::SelectionListener
{
    public:
        /**
         * Constructor.
         */
        InventoryWindow(int invSize = (INVENTORY_SIZE - 2));

        /**
         * Logic (updates buttons and weight information).
         */
        void logic();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Returns the selected item.
         */
        Item* getSelectedItem() const;

        void selectNone() const;

        /**
         * Updates button states.
         */
        void updateButtons();

        void mouseClicked(gcn::MouseEvent &event);

        void valueChanged(const gcn::SelectionEvent &event);

        /**
         * Focuses on the item container on gaining focus.
         */
        void requestFocus();

        /**
         * Allows for progress bars to reset on visibility. This is done more
         * for a "bling" reason, not a necessary reason.
         */
        void widgetShown(const gcn::Event& event);

        /**
         * Deselects the currently selected item.
         */
        void widgetHidden(const gcn::Event& event);

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

    private:
        ItemContainer *mItems;

        std::string mWeight;
        std::string mSlots;
        int mUsedSlots;
        int mTotalWeight;
        int mMaxWeight;
        gcn::Button *mShortcutButton, *mTradeButton, *mStoreButton;
        gcn::Button *mUseButton, *mDropButton;
        gcn::ScrollArea *mInvenScroll;

        gcn::Label *mWeightLabel;
        gcn::Label *mSlotsLabel;

        ProgressBar *mWeightBar;
        ProgressBar *mSlotsBar;

        int mMaxSlots;

        bool mItemDesc;

        std::list<gcn::SelectionListener*> mListeners;

        /**
         * Sends out selection events to the list of selection listeners.
         */
        void distributeValueChangedEvent(void);
};

extern InventoryWindow *inventoryWindow;

#endif
