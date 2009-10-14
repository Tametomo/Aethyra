/*
 *  Aethyra
 *  Copyright 2008 Aethyra Development Team
 *
 *  This file is part of Aethyra.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef EQUIPMENT_WINDOW_H
#define EQUIPMENT_WINDOW_H

#include <guichan/actionlistener.hpp>
#include <guichan/selectionlistener.hpp>

#include "../../bindings/guichan/widgets/window.h"

class Button;
class BeingBox;
class Equipment;
class EquipmentConfigListener;
class Icon;
class Inventory;
class Item;
class ItemContainer;
class ItemPopup;
class PopupMenu;

/**
 * Equipment dialog.
 *
 * \ingroup Interface
 */
class EquipmentWindow : public Window,
                        public gcn::ActionListener,
                        public gcn::SelectionListener
{
    public:
        friend class EquipmentConfigListener;

        /**
         * Constructor.
         */
        EquipmentWindow();

        /**
         * Destructor.
         */
        ~EquipmentWindow();

        /**
         * Draws the equipment window.
         */
        void draw(gcn::Graphics *graphics);

        void action(const gcn::ActionEvent &event);

        void mousePressed(gcn::MouseEvent& mouseEvent);

        /**
         * Focuses on the item container on gaining focus.
         */
        void requestFocus();

        /**
         * Updates button states.
         */
        void updateButtons();

        /**
         * Resets the item filter for the equipment window on shown events.
         */
        void widgetHidden(const gcn::Event& event);

        enum {
            // Equipment rules:
            EQUIP_LEGS_SLOT = 0,
            EQUIP_FIGHT1_SLOT,
            EQUIP_GLOVES_SLOT,
            EQUIP_RING2_SLOT,
            EQUIP_RING1_SLOT,
            EQUIP_FIGHT2_SLOT,
            EQUIP_FEET_SLOT,
            EQUIP_CAPE_SLOT,
            EQUIP_HEAD_SLOT,
            EQUIP_TORSO_SLOT,
            EQUIP_AMMO_SLOT,
            EQUIP_VECTOREND
        };

        void fontChanged();
    private:
        void widgetResized(const gcn::Event &event);

        void mouseExited(gcn::MouseEvent &event);
        void mouseMoved(gcn::MouseEvent &event);

        Item* getItem(const int x, const int y);

        void setSelected(int index);

        void valueChanged(const gcn::SelectionEvent &event);

        Equipment *mEquipment;
        Inventory *mInventory;
        Button *mEquipButton;                   /**< Button for both equipping and unequipping. */
        Icon *mEquipIcon[EQUIP_VECTOREND];      /**< Equipment Icons. */

        static EquipmentConfigListener *mConfigListener;

        static int mInstances;
        static bool mShowItemInfo;

        static ItemPopup *mItemPopup;
        static PopupMenu *mPopupMenu;

        BeingBox *mBeingBox;

        int mSelected;                          /**< Index of selected slot (the slot may be empty). */

        ItemContainer *mItems;                  /**< Equippable items */
        gcn::ScrollArea *mInvenScroll;
};

extern EquipmentWindow *equipmentWindow;

#endif
