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

#include <string>

#include "configuration.h"
#include "emoteshortcut.h"
#include "engine.h"
#include "game.h"
#include "itemshortcut.h"
#include "log.h"
#include "map.h"

#include "bindings/guichan/gui.h"
#include "bindings/guichan/inputmanager.h"

#include "bindings/guichan/widgets/browserbox.h"
#include "bindings/guichan/widgets/emoteshortcutcontainer.h"
#include "bindings/guichan/widgets/itemshortcutcontainer.h"

#include "gui/buy.h"
#include "gui/buysell.h"
#include "gui/chat.h"
#include "gui/debugwindow.h"
#include "gui/emotewindow.h"
#include "gui/equipmentwindow.h"
#include "gui/inventorywindow.h"
#include "gui/menubar.h"
#include "gui/minimap.h"
#include "gui/ministatus.h"
#include "gui/npcintegerdialog.h"
#include "gui/npclistdialog.h"
#include "gui/npcstringdialog.h"
#include "gui/npctext.h"
#include "gui/okdialog.h"
#include "gui/sell.h"
#include "gui/shortcutwindow.h"
#include "gui/skill.h"
#include "gui/status.h"
#include "gui/storagewindow.h"
#include "gui/trade.h"
#include "gui/viewport.h"

#include "net/beinghandler.h"
#include "net/buysellhandler.h"
#include "net/chathandler.h"
#include "net/equipmenthandler.h"
#include "net/inventoryhandler.h"
#include "net/itemhandler.h"
#include "net/messageout.h"
#include "net/network.h"
#include "net/npchandler.h"
#include "net/playerhandler.h"
#include "net/protocol.h"
#include "net/skillhandler.h"
#include "net/tradehandler.h"

#include "resources/beingmanager.h"
#include "resources/flooritemmanager.h"

#include "resources/particle/particle.h"

#include "resources/sprite/localplayer.h"

#include "utils/gettext.h"

std::string map_path;

Engine *engine = NULL;

OkDialog *disconnectedDialog = NULL;

ChatWindow *chatWindow;
MenuBar *menuBar;
StatusWindow *statusWindow;
MiniStatusWindow *miniStatusWindow;
BuyDialog *buyDialog;
SellDialog *sellDialog;
BuySellDialog *buySellDialog;
InventoryWindow *inventoryWindow;
EmoteWindow *emoteWindow;
NpcIntegerDialog *npcIntegerDialog;
NpcListDialog *npcListDialog;
NpcTextDialog *npcTextDialog;
NpcStringDialog *npcStringDialog;
SkillDialog *skillDialog;
Minimap *minimap;
EquipmentWindow *equipmentWindow;
TradeWindow *tradeWindow;
DebugWindow *debugWindow;
ShortcutWindow *itemShortcutWindow;
ShortcutWindow *emoteShortcutWindow;
StorageWindow *storageWindow;

BeingManager *beingManager = NULL;
FloorItemManager *floorItemManager = NULL;
Particle* particleEngine = NULL;

/**
 * Listener used for exiting handling.
 */
namespace
{
    struct ExitListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            if (event.getId() == "ok")
                game->setInGame(false);

            disconnectedDialog = NULL;
        }
    } exitListener;
}

/**
 * Create all the various globally accessible gui windows
 */
void createGuiWindows()
{
    // Initialize the shortcuts for the shortcut windows.
    itemShortcut = new ItemShortcut();
    emoteShortcut = new EmoteShortcut();

    // Create dialogs
    chatWindow = new ChatWindow();
    menuBar = new MenuBar();
    statusWindow = new StatusWindow(player_node);
    miniStatusWindow = new MiniStatusWindow();
    buyDialog = new BuyDialog();
    sellDialog = new SellDialog();
    buySellDialog = new BuySellDialog();
    inventoryWindow = new InventoryWindow();
    emoteWindow = new EmoteWindow();
    npcTextDialog = new NpcTextDialog();
    npcIntegerDialog = new NpcIntegerDialog();
    npcListDialog = new NpcListDialog();
    npcStringDialog = new NpcStringDialog();
    skillDialog = new SkillDialog();
    minimap = new Minimap();
    equipmentWindow = new EquipmentWindow();
    tradeWindow = new TradeWindow();
    debugWindow = new DebugWindow();
    itemShortcutWindow = new ShortcutWindow("ItemShortcut",
                                            new ItemShortcutContainer);
    emoteShortcutWindow = new ShortcutWindow("emoteShortcut",
                                             new EmoteShortcutContainer);
    storageWindow = new StorageWindow();
}

/**
 * Destroy all the globally accessible gui windows
 */
static void destroyGuiWindows()
{
    logger->setChatWindow(NULL);
    delete chatWindow;
    delete statusWindow;
    delete miniStatusWindow;
    delete menuBar;
    delete buyDialog;
    delete sellDialog;
    delete buySellDialog;
    delete inventoryWindow;
    delete emoteWindow;
    delete npcIntegerDialog;
    delete npcListDialog;
    delete npcTextDialog;
    delete npcStringDialog;
    delete skillDialog;
    delete minimap;
    delete equipmentWindow;
    delete tradeWindow;
    delete debugWindow;
    delete itemShortcutWindow;
    delete emoteShortcutWindow;
    delete storageWindow;

    delete itemShortcut;
    delete emoteShortcut;
}

Game::Game(Network *network):
    mNetwork(network),
    mBeingHandler(new BeingHandler(config.getValue("EnableSync", 0) == 1)),
    mBuySellHandler(new BuySellHandler()),
    mChatHandler(new ChatHandler()),
    mEquipmentHandler(new EquipmentHandler()),
    mInventoryHandler(new InventoryHandler()),
    mItemHandler(new ItemHandler()),
    mNpcHandler(new NPCHandler()),
    mPlayerHandler(new PlayerHandler()),
    mSkillHandler(new SkillHandler()),
    mTradeHandler(new TradeHandler())
{
    createGuiWindows();
    engine = new Engine();
    mInGame = true;

    beingManager = new BeingManager();
    floorItemManager = new FloorItemManager();

    particleEngine = new Particle(NULL);
    particleEngine->setupEngine();

    // Initialize beings
    beingManager->setPlayer(player_node);

    network->registerHandler(mBeingHandler.get());
    network->registerHandler(mBuySellHandler.get());
    network->registerHandler(mChatHandler.get());
    network->registerHandler(mEquipmentHandler.get());
    network->registerHandler(mInventoryHandler.get());
    network->registerHandler(mItemHandler.get());
    network->registerHandler(mNpcHandler.get());
    network->registerHandler(mPlayerHandler.get());
    network->registerHandler(mSkillHandler.get());
    network->registerHandler(mTradeHandler.get());

    /*
     * To prevent the server from sending data before the client
     * has initialized, Sanga modified the July 2008 eAthena client
     * to wait for a "ping" from the client to complete its
     * initialization. Eventually, this will not be needed.
     *
     * Note: This only affects the latest eAthena version.  This
     * packet is handled by the older version, but its response
     * is ignored by the client
     */
    MessageOut msg(CMSG_CLIENT_PING);
    msg.writeInt32(tick_time);

    map_path = map_path.substr(0, map_path.rfind("."));
    engine->changeMap(map_path);
    MessageOut outMsg(CMSG_MAP_LOADED);
}

Game::~Game()
{
    delete player_node;
    destroyGuiWindows();

    delete beingManager;
    delete floorItemManager;
    delete particleEngine;
    delete engine;

    beingManager = NULL;
    floorItemManager = NULL;
}

void Game::logic()
{
    int gameTime = tick_time;

    while (mInGame)
    {
        engine->getCurrentMap()->update(get_elapsed_time(gameTime));

        // Handle all necessary game logic
        while (get_elapsed_time(gameTime) > 0)
        {
            engine->logic();
            gameTime++;
        }

        // This is done because at some point tick_time will wrap.
        gameTime = tick_time;
        inputManager->handleInput();
        gui->logic();

        // Handle network stuff
        mNetwork->flush();
        mNetwork->dispatchMessages();

        if (!mNetwork->isConnected())
        {
            if (!disconnectedDialog)
            {
                disconnectedDialog = new OkDialog(_("Network Error"),
                                                  _("The connection to the "
                                                    "server was lost, the "
                                                    "program will now quit"));
                disconnectedDialog->addActionListener(&exitListener);
                disconnectedDialog->requestMoveToTop();
            }
        }
    }
}

