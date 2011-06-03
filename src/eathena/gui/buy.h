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

#ifndef BUY_H
#define BUY_H

#include <guichan/selectionlistener.hpp>

#include <stdint.h>

#include "../../bindings/guichan/widgets/window.h"

class ShopListBox;
class ShopListModel;
class ListBox;

/**
 * The buy dialog.
 *
 * \ingroup Interface
 */
class BuyDialog : public Window, public gcn::SelectionListener
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        BuyDialog();

        /**
         * Destructor
         */
        ~BuyDialog();

        /**
         * Resets the dialog, clearing shop inventory.
         */
        void reset();

        /**
         * Sets the amount of available money.
         */
        void setMoney(int amount);

        /**
         * Adds an item to the shop inventory.
         */
        void addItem(int id, int price);

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Returns the number of items in the shop inventory.
         */
        int getNumberOfElements();

        /**
         * Updates the labels according to the selected item.
         */
        void valueChanged(const gcn::SelectionEvent &event);

        /**
         * Returns the name of item number i in the shop inventory.
         */
        std::string getElementAt(int i);

        /**
         * Updates the state of buttons and labels.
         */
        void updateButtonsAndLabels();

        /**
         * Focus on the shop item list on dialog focus.
         */
        void requestFocus();

        /**
         * Closes the Buy Window, as well as resetting the current npc.
         */
        void close();

        /**
         * Resets the listbox selection on shown events.
         */
        void widgetShown(const gcn::Event& event);

        void fontChanged();
    private:
        gcn::Button *mBuyButton;
        gcn::Button *mQuitButton;
        gcn::Button *mAddMaxButton;
        ShopListBox *mShopItemList;
        gcn::ScrollArea *mScrollArea;
        gcn::Label *mItemDescLabel;
        gcn::Label *mItemEffectLabel;
        gcn::Label *mMoneyLabel;
        gcn::Label *mQuantityLabel;
        gcn::Slider *mSlider;

        ShopListModel *mShopListModel;

        uint32_t mMoney;
        uint32_t mAmountItems;
        uint32_t mMaxItems;
};

extern BuyDialog *buyDialog;

#endif
