/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *  Copyright (C) 2009  Aethyra Development Team
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

#include <cassert>

#include <guichan/focushandler.hpp>

#include "chat.h"
#include "emotewindow.h"
#include "inventorywindow.h"
#include "itemamount.h"
#include "popupmenu.h"
#include "slotselection.h"
#include "storagewindow.h"
#include "trade.h"

#include "../beingmanager.h"
#include "../playerrelations.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../structs/item.h"

#include "../../bindings/guichan/graphics.h"

#include "../../bindings/guichan/handlers/shortcuthandler.h"

#include "../../bindings/guichan/models/linkmappedlistmodel.h"

#include "../../bindings/guichan/widgets/mappedlistbox.h"

#include "../../core/configuration.h"

#include "../../core/image/sprite/being.h"
#include "../../core/image/sprite/flooritem.h"
#include "../../core/image/sprite/localplayer.h"
#include "../../core/image/sprite/npc.h"

#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

extern std::string tradePartnerName;

PopupMenu::PopupMenu(MenuType type, gcn::Container *parent):
    Popup("PopupMenu", "graphics/gui/gui.xml", parent),
    mPreviousFocus(NULL),
    mBeing(NULL),
    mFloorItem(NULL),
    mItem(NULL),
    mEmote(-1),
    mType(type)
{
    mModel = new LinkMappedListModel();

    mMappedListBox = new MappedListBox(mModel);
    mMappedListBox->setPosition(getPadding(), getPadding());
    mMappedListBox->setWrappingEnabled(true);
    mMappedListBox->setFollowingMouse(true);
    mMappedListBox->addActionListener(this);
    add(mMappedListBox);

    loadPopupConfiguration();
}

void PopupMenu::action(const gcn::ActionEvent &event)
{
    // Talk To action
    if (event.getId() == "talk" && mBeing && mBeing->getType() == Being::NPC &&
        current_npc == 0)
    {
        dynamic_cast<NPC*>(mBeing)->talk();
    }

    // Trade action
    else if (event.getId() == "trade" && mBeing &&
             mBeing->getType() == Being::PLAYER)
    {
        player_node->trade(mBeing);
        tradePartnerName = mBeing->getName();
    }

    else if (event.getId() == "tradeitem" && mItem && tradeWindow &&
             tradeWindow->isVisible())
    {
        mPreviousFocus = mFocusHandler->getFocused();
        ItemAmountWindow *temp = new ItemAmountWindow(AMOUNT_TRADE_ADD,
                                                      inventoryWindow, mItem);
        temp->requestFocus();
    }

    else if (event.getId() == "showitempopup")
    {
        config.setValue("showItemPopups", true);
    }

    else if (event.getId() == "hideitempopup")
    {
        config.setValue("showItemPopups", false);
    }

    else if (event.getId() == "slotitem" && mItem)
    {
        mPreviousFocus = mFocusHandler->getFocused();
        SlotSelectionWindow *temp = new SlotSelectionWindow(ITEM_SHORTCUT,
                                                            inventoryWindow,
                                                            mItem->getId());
        temp->requestFocus();
    }

    else if (event.getId() == "slotemote" && mEmote != -1)
    {
        mPreviousFocus = mFocusHandler->getFocused();
        SlotSelectionWindow *temp = new SlotSelectionWindow(EMOTE_SHORTCUT,
                                                            emoteWindow, mEmote);
        temp->requestFocus();
    }

    // Attack action
    else if (event.getId() == "attack" && mBeing)
    {
        player_node->attack(mBeing, true);
    }

    else if (event.getId() == "unignore" && mBeing &&
             mBeing->getType() == Being::PLAYER)
    {
        player_relations.setRelation(mBeing->getName(), PlayerRelation::NEUTRAL);
    }

    else if (event.getId() == "ignore" && mBeing &&
             mBeing->getType() == Being::PLAYER)
    {
        player_relations.setRelation(mBeing->getName(), PlayerRelation::IGNORED);
    }

    else if (event.getId() == "disregard" && mBeing &&
             mBeing->getType() == Being::PLAYER)
    {
        player_relations.setRelation(mBeing->getName(), PlayerRelation::DISREGARDED);
    }

    else if (event.getId() == "friend" && mBeing &&
             mBeing->getType() == Being::PLAYER)
    {
        player_relations.setRelation(mBeing->getName(), PlayerRelation::FRIEND);
    }

    /*
    // Follow Player action
    else if (event.getId() == "follow")
    {
    }*/

    // Pick Up Floor Item action
    else if (event.getId() == "pickup" && mFloorItem)
    {
        player_node->pickUp(mFloorItem);
    }

    // Look To action
    else if (event.getId() == "look")
    {
    }

    else if (event.getId() == "use")
    {
        assert(mItem);
        if (mItem->isEquipment())
        {
            if (mItem->isEquipped())
                player_node->unequipItem(mItem);
            else
                player_node->equipItem(mItem);
        }
        else
        {
            player_node->useItem(mItem);
        }
    }

    else if (event.getId() == "useemote")
    {
        if (mEmote != -1)
            player_node->emote(mEmote);
    }

    else if (event.getId() == "chat")
    {
        mPreviousFocus = mFocusHandler->getFocused();
        chatWindow->addItemText(mItem->getInfo().getName());
        chatWindow->requestChatFocus();
    }

    else if (event.getId() == "drop")
    {
        mPreviousFocus = mFocusHandler->getFocused();
        ItemAmountWindow *temp = new ItemAmountWindow(AMOUNT_ITEM_DROP,
                                                      inventoryWindow, mItem);
        temp->requestFocus();
    }

    else if (event.getId() == "store" && storageWindow &&
             storageWindow->isVisible())
    {
        mPreviousFocus = mFocusHandler->getFocused();
        ItemAmountWindow *temp = new ItemAmountWindow(AMOUNT_STORE_ADD,
                                                      inventoryWindow, mItem);
        temp->requestFocus();
    }

    else if (event.getId() == "retrieve")
    {
        mPreviousFocus = mFocusHandler->getFocused();
        ItemAmountWindow *temp = new ItemAmountWindow(AMOUNT_STORE_REMOVE,
                                                      storageWindow, mItem);
        temp->requestFocus();
    }

    else if (event.getId() == "party" && mBeing &&
             mBeing->getType() == Being::PLAYER)
    {
        MessageOut outMsg(CMSG_PARTY_INVITE);
        outMsg.writeInt32(mBeing->getId());
    }

    else if (event.getId() == "name" && mBeing)
    {
        mPreviousFocus = mFocusHandler->getFocused();
        const std::string &name = mBeing->getType() == Being::NPC ?
                                  mBeing->getName().substr(0,
                                  mBeing->getName().size() - 6) :
                                  mBeing->getName();
        chatWindow->addInputText(name);
        chatWindow->requestChatFocus();
    }

    // Unknown actions
    else if (event.getId() != "cancel")
    {
        std::cout << event.getId() << std::endl;
    }

    setVisible(false);

    if (mPreviousFocus)
        mPreviousFocus->requestFocus();

    mEmote = -1;
    mPreviousFocus = NULL;
    mBeing = NULL;
    mFloorItem = NULL;
    mItem = NULL;
}

void PopupMenu::showPopup(int x, int y)
{
    mModel->clear();

    if (mType < BEING)
    {
        if (mFloorItem && mType == FLOOR_ITEM)
            mItem = mFloorItem->getItem();

        if (!mItem)
            return;

        const std::string &name = mItem->getName();

        if (mType <= INVENTORY)
        {
            if (mItem->isEquipment())
            {
                if (mItem->isEquipped())
                    mModel->addLink("use", _("Unequip"));
                else
                    mModel->addLink("use", _("Equip"));
            }
            else
                mModel->addLink("use", _("Use"));

            mModel->addLink("drop", _("Drop"));

            if (tradeWindow &&  tradeWindow->canTrade() &&
                !tradeWindow->tradingItem(mItem))
                mModel->addLink("tradeitem", _("Trade"));

            if (storageWindow && storageWindow->isVisible())
                mModel->addLink("store", _("Store"));

            if (mType == INVENTORY)
                mModel->addLink("slotitem", _("Add to Item Shortcuts"));
        }
        else if (mType == STORAGE)
            mModel->addLink("retrieve", _("Retrieve"));
        else if (mType == FLOOR_ITEM)
            mModel->addLink("pickup", strprintf(_("Pick Up %s"), name.c_str()));

        if (mType < FLOOR_ITEM)
        {
            if (config.getValue("showItemPopups", true))
                mModel->addLink("hideitempopup", _("Hide Item Info"));
            else
                mModel->addLink("showitempopup", _("Show Item Info"));
        }

        mModel->addLink("chat", _("Add to Chat"));
    }
    else if (mType == BEING)
    {
        if (!mBeing)
            return;

        // Any being's name can be added to chat
        mModel->addLink("name", _("Add name to chat"));

        const std::string &name = mBeing->getType() == Being::NPC ?
                                  mBeing->getName().substr(0,
                                  mBeing->getName().size() - 6) :
                                  mBeing->getName();

        switch (mBeing->getType())
        {
            case Being::PLAYER:
            {
                // Players can be traded with. Later also follow and
                // add as buddy will be options in this menu.
                mModel->addLink("trade", strprintf(_("Trade With %s"), name.c_str()));
                mModel->addLink("attack", strprintf(_("Attack %s"), name.c_str()));

                mModel->addLink("", "---");

                switch (player_relations.getRelation(name))
                {
                    case PlayerRelation::NEUTRAL:
                        mModel->addLink("friend", strprintf(_("Befriend %s"), name.c_str()));

                    case PlayerRelation::FRIEND:
                        mModel->addLink("disregard", strprintf(_("Disregard %s"), name.c_str()));
                        mModel->addLink("ignore", strprintf(_("Ignore %s"), name.c_str()));
                        break;

                    case PlayerRelation::IGNORED:
                        mModel->addLink("unignore", strprintf(_("Un-Ignore %s"), name.c_str()));

                    case PlayerRelation::DISREGARDED:
                        mModel->addLink("ignore", strprintf(_("Completely ignore %s"), name.c_str()));
                        break;
                }

                //mModel->addLink("follow", _(strprintf("Follow %s"), name.c_str()));

                
                /*mModel->addLink("", "---");
                mModel->addLink("party", strprintf(_("Invite %s to party"), name.c_str()));*/
            }
            break;

            case Being::NPC:
                // NPCs can be talked to (single option, candidate for removal
                // unless more options would be added)
                mModel->addLink("talk", strprintf(_("Talk To %s"), name.c_str()));
                break;

            case Being::MONSTER:
                // Monsters can be attacked
                mModel->addLink("attack", strprintf(_("Attack %s"), name.c_str()));
                break;

            default:
                /* Other beings aren't interesting... */
                return;
        }
    }
    else if (mType == EMOTE)
    {
        if (mEmote != -1)
        {
            mModel->addLink("useemote", _("Use"));
            mModel->addLink("slotemote", _("Add to Emote Shortcuts"));
        }
    }

    mModel->addLink("", "---");
    mModel->addLink("cancel", _("Cancel"));

    mMappedListBox->adjustSize();

    setContentSize(mMappedListBox->getWidth() + (2 * getPadding()),
                   mMappedListBox->getHeight() + getPadding());

    if (graphics->getWidth() < (x + getWidth()))
        x = graphics->getWidth() - getWidth();
    if (graphics->getHeight() < (y + getHeight()))
        y = graphics->getHeight() - getHeight();

    setPosition(x, y);
    setVisible(true);
    requestFocus();
    requestMoveToTop();
}

void PopupMenu::requestFocus()
{
    mPreviousFocus = mFocusHandler->getFocused();

    mMappedListBox->requestFocus();
}
