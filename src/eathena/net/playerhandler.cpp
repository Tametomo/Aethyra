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

#include "messagein.h"
#include "playerhandler.h"
#include "protocol.h"

#include "../gui/buy.h"
#include "../gui/buysell.h"
#include "../gui/chat.h"
#include "../gui/npctext.h"
#include "../gui/npcintegerdialog.h"
#include "../gui/npclistdialog.h"
#include "../gui/npcstringdialog.h"
#include "../gui/sell.h"
#include "../gui/skill.h"
#include "../gui/storagewindow.h"
#include "../gui/viewport.h"

#include "../../bindings/guichan/dialogs/okdialog.h"

#include "../../core/log.h"

#include "../../core/map/sprite/localplayer.h"
#include "../../core/map/sprite/npc.h"

#include "../../core/utils/stringutils.h"
#include "../../core/utils/gettext.h"

// TODO Move somewhere else
OkDialog *weightNotice = NULL;
OkDialog *deathNotice = NULL;

// Max. distance we are willing to scroll after a teleport;
// everything beyond will reset the port hard.
static const int MAP_TELEPORT_SCROLL_DISTANCE = 8;

// TODO Move somewhere else
namespace {

    /**
     * Listener used for handling the overweigth message.
     */
    struct WeightListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            weightNotice = NULL;
        }
    } weightListener;

    /**
     * Listener used for handling death message.
     */
    struct DeathListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            player_node->revive();
            deathNotice = NULL;
            npcIntegerDialog->reset();
            npcIntegerDialog->setVisible(false);
            npcListDialog->reset();
            npcListDialog->setVisible(false);
            npcStringDialog->setValue("");
            npcStringDialog->setVisible(false);
            npcTextDialog->clearText();
            npcTextDialog->setVisible(false);
            buyDialog->setVisible(false);
            sellDialog->setVisible(false);
            buySellDialog->setVisible(false);
            if (storageWindow->isVisible())
                storageWindow->close();
            viewport->closePopupMenu();
            current_npc = 0;
        }
    } deathListener;

} // anonymous namespace

static const char *randomDeathMessage()
{
    static char const *const deadMsg[] =
    {
        N_("You are dead."),
        N_("We regret to inform you that your character was killed in "
           "battle."),
        N_("You are not that alive anymore."),
        N_("The cold hands of the grim reaper are grabbing for your soul."),
        N_("Game Over!"),
        N_("Insert coin to continue"),
        N_("No, kids. Your character did not really die. It... "
           "err... went to a better place."),
        N_("Your plan of breaking your enemies weapon by "
           "bashing it with your throat failed."),
        N_("I guess this did not run too well."),
        // NetHack reference:
        N_("Do you want your possessions identified?"),
        // Secret of Mana reference:
        N_("Sadly, no trace of you was ever found..."),
        // Final Fantasy VI reference:
        N_("Annihilated."),
        // Earthbound reference:
        N_("Looks like you got your head handed to you."),
        // Leisure Suit Larry 1 reference:
        N_("You screwed up again, dump your body down the tubes "
           "and get you another one."),
        // Monty Python references (Dead Parrot sketch mostly):
        N_("You're not dead yet. You're just resting."),
        N_("You are no more."),
        N_("You have ceased to be."),
        N_("You've expired and gone to meet your maker."),
        N_("You're a stiff."),
        N_("Bereft of life, you rest in peace."),
        N_("If you weren't so animated, you'd be pushing up the daisies."),
        N_("Your metabolic processes are now history."),
        N_("You're off the twig."),
        N_("You've kicked the bucket."),
        N_("You've shuffled off your mortal coil, run down the "
           "curtain and joined the bleedin' choir invisibile."),
        N_("You are an ex-player."),
        N_("You're pining for the fjords.")
    };

    const int random = rand() % (sizeof(deadMsg) / sizeof(deadMsg[0]));
    return gettext(deadMsg[random]);
}

PlayerHandler::PlayerHandler()
{
    static const Uint16 _messages[] = {
        SMSG_WALK_RESPONSE,
        SMSG_PLAYER_WARP,
        SMSG_PLAYER_STAT_UPDATE_1,
        SMSG_PLAYER_STAT_UPDATE_2,
        SMSG_PLAYER_STAT_UPDATE_3,
        SMSG_PLAYER_STAT_UPDATE_4,
        SMSG_PLAYER_STAT_UPDATE_5,
        SMSG_PLAYER_STAT_UPDATE_6,
        SMSG_PLAYER_ARROW_MESSAGE,
        0
    };
    handledMessages = _messages;
}

void PlayerHandler::handleMessage(MessageIn *msg)
{
    const int numOfAttr = LocalPlayer::LUK - LocalPlayer::STR;

    switch (msg->getId())
    {
        case SMSG_WALK_RESPONSE:
            /*
             * This client assumes that all walk messages succeed,
             * and that the server will send a correction notice
             * otherwise.
             */
            break;

        case SMSG_PLAYER_WARP:
            {
                std::string mapPath = msg->readString(16);
                bool nearby;
                Uint16 x = msg->readInt16();
                Uint16 y = msg->readInt16();

                logger->log("Warping to %s (%d, %d)", mapPath.c_str(), x, y);

                /*
                 * We must clear the local player's target *before* the call
                 * to changeMap, as it deletes all beings.
                 */
                player_node->stopAttack();

                nearby = (viewport->getMapName() == mapPath);

                // Switch the actual map, deleting the previous one if necessary
                viewport->changeMap(mapPath);

                current_npc = 0;

                int scrollOffsetX = 0;
                int scrollOffsetY = 0;

                /* Scroll if neccessary */
                if (!nearby
                    || (abs(x - player_node->mX) > MAP_TELEPORT_SCROLL_DISTANCE)
                    || (abs(y - player_node->mY) > MAP_TELEPORT_SCROLL_DISTANCE))
                {
                    scrollOffsetX = x - player_node->mX;
                    scrollOffsetY = y - player_node->mY;
                }

                player_node->setAction(Being::STAND);
                player_node->mFrame = 0;
                player_node->mX = x;
                player_node->mY = y;

                logger->log("Adjust scrolling by (%d, %d) tiles", scrollOffsetX,
                            scrollOffsetY);

                viewport->scrollBy(scrollOffsetX, scrollOffsetY);
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_1:
            {
                int type = msg->readInt16();
                int value = msg->readInt32();

                switch (type)
                {
                    case 0x0000: player_node->setWalkSpeed(value); break;
                    case 0x0005: player_node->mHp = value; break;
                    case 0x0006: player_node->mMaxHp = value; break;
                    case 0x0007: player_node->mMp = value; break;
                    case 0x0008: player_node->mMaxMp = value; break;
                    case 0x0009:
                                 player_node->mStatsPointsToAttribute = value;
                                 break;
                    case 0x000b: player_node->mLevel = value; break;
                    case 0x000c:
                                 player_node->mSkillPoint = value;
                                 skillDialog->update();
                                 break;
                    case 0x0018:
                                 if (value >= player_node->mMaxWeight / 2 &&
                                     player_node->mTotalWeight <
                                     player_node->mMaxWeight / 2)
                                 {
                                     weightNotice = new OkDialog(_("Message"),
                                             _("You are carrying more than "
                                               "half your weight. You are "
                                               "unable to regain health."));
                                     weightNotice->addActionListener(
                                             &weightListener);
                                 }
                                 player_node->mTotalWeight = value;
                                 break;
                    case 0x0019: player_node->mMaxWeight = value; break;
                    case 0x0029: player_node->ATK = value; break;
                    case 0x002b: player_node->MATK = value; break;
                    case 0x002d: player_node->DEF = value; break;
                    case 0x002e: player_node->DEF_BONUS = value; break;
                    case 0x002f: player_node->MDEF = value; break;
                    case 0x0031: player_node->HIT = value; break;
                    case 0x0032: player_node->FLEE = value; break;
                    case 0x0035: player_node->mAttackSpeed = value; break;
                    case 0x0037: player_node->mJobLevel = value; break;
                }

                if (player_node->mHp == 0 && !deathNotice)
                {
                    deathNotice = new OkDialog(_("Message"),
                                               randomDeathMessage());
                    deathNotice->addActionListener(&deathListener);
                    player_node->setAction(Being::DEAD);
                }
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_2:
            switch (msg->readInt16())
            {
                case 0x0001:
                    player_node->setXp(msg->readInt32());
                    break;
                case 0x0002:
                    player_node->mJobXp = msg->readInt32();
                    break;
                case 0x0014:
                    {
                        Uint32 curGp = player_node->mGp;
                        player_node->mGp = msg->readInt32();
                        if (player_node->mGp > curGp)
                            chatWindow->chatLog(strprintf(
                                _("You picked up %d GP"),
                                player_node->mGp - curGp));
                    }
                    break;
                case 0x0016:
                    player_node->mXpForNextLevel = msg->readInt32();
                    break;
                case 0x0017:
                    player_node->mJobXpForNextLevel = msg->readInt32();
                    break;
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_3:
            {
                const int type = msg->readInt32();
                const int base = msg->readInt32();
                const int bonus = msg->readInt32();
                const int total = base + bonus;
                const int index = type - LocalPlayer::STR;

                if (index >=0 && index <= numOfAttr)
                    player_node->mAttr[index] = total;
            }
            break;

        case SMSG_PLAYER_STAT_UPDATE_4:
            {
                const int type = msg->readInt16();
                const int fail = msg->readInt8();
                const int value = msg->readInt8();
                const int index = type - LocalPlayer::STR;

                if (fail != 1)
                    break;

                if (index >=0 && index <= numOfAttr)
                    player_node->mAttr[index] = value;
            }
            break;

        // Updates stats and status points
        case SMSG_PLAYER_STAT_UPDATE_5:
            player_node->mStatsPointsToAttribute = msg->readInt16();

            for (int i = LocalPlayer::STR; i <= LocalPlayer::LUK; i++)
            {
                const int index = i - LocalPlayer::STR;

                player_node->mAttr[index] = msg->readInt8();
                player_node->mAttrUp[index] = msg->readInt8();
            }

            player_node->ATK       = msg->readInt16();  // ATK
            player_node->ATK_BONUS  = msg->readInt16();  // ATK bonus
            player_node->MATK      = msg->readInt16();  // MATK max
            player_node->MATK_BONUS = msg->readInt16();  // MATK min
            player_node->DEF       = msg->readInt16();  // DEF
            player_node->DEF_BONUS  = msg->readInt16();  // DEF bonus
            player_node->MDEF      = msg->readInt16();  // MDEF
            player_node->MDEF_BONUS = msg->readInt16();  // MDEF bonus
            player_node->HIT       = msg->readInt16();  // HIT
            player_node->FLEE      = msg->readInt16();  // FLEE
            player_node->FLEE_BONUS = msg->readInt16();  // FLEE bonus
            msg->readInt16();  // critical
            msg->readInt16();  // unknown
            break;

        case SMSG_PLAYER_STAT_UPDATE_6:
            {
                const int index = msg->readInt16() - 0x0020;
                const int attr = msg->readInt8();

                if (index >=0 && index <= numOfAttr)
                    player_node->mAttrUp[index] = attr;
            }

            break;

        case SMSG_PLAYER_ARROW_MESSAGE:
            {
                const int type = msg->readInt16();

                switch (type)
                {
                    case 0:
                        chatWindow->chatLog(_("Equip arrows first."));
                        break;
                    default:
                        logger->log("0x013b: Unhandled message %i", type);
                        break;
                }
            }
            break;
    }
}

