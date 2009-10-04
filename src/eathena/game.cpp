/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra derived from original code
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

#include "beingmanager.h"
#include "flooritemmanager.h"
#include "game.h"
#include "maploader.h"
#include "playerrelations.h"

#include "db/colordb.h"
#include "db/effectdb.h"
#include "db/emotedb.h"
#include "db/itemdb.h"
#include "db/monsterdb.h"
#include "db/npcdb.h"
#include "db/skilldb.h"

#include "gui/buy.h"
#include "gui/buysell.h"
#include "gui/chat.h"
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
#include "gui/sell.h"
#include "gui/skill.h"
#include "gui/status.h"
#include "gui/storagewindow.h"
#include "gui/trade.h"
#include "gui/viewport.h"

#include "gui/tabs/setup_game.h"
#include "gui/tabs/setup_players.h"

#include "handlers/emoteshortcut.h"
#include "handlers/itemshortcut.h"

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

#include "widgets/emoteshortcutcontainer.h"
#include "widgets/itemshortcutcontainer.h"

#include "../main.h"

#include "../bindings/guichan/gui.h"
#include "../bindings/guichan/inputmanager.h"

#include "../bindings/guichan/dialogs/okdialog.h"
#include "../bindings/guichan/dialogs/setupdialog.h"

#include "../bindings/guichan/widgets/shortcutwindow.h"

#include "../core/configuration.h"
#include "../core/log.h"

#include "../core/image/particle/particle.h"

#include "../core/map/map.h"

#include "../core/map/sprite/localplayer.h"

#include "../core/utils/gettext.h"

std::string map_path;

MapLoader *mapLoader = NULL;

EmoteShortcut *emoteShortcut;
ItemShortcut *itemShortcut;

Setup_Game *setupGame = NULL;
Setup_Players *setupPlayers = NULL;

OkDialog *disconnectedDialog = NULL;

BuyDialog *buyDialog;
BuySellDialog *buySellDialog;
ChatWindow *chatWindow;
EmoteWindow *emoteWindow;
EquipmentWindow *equipmentWindow;
InventoryWindow *inventoryWindow;
MenuBar *menuBar;
Minimap *minimap;
MiniStatusWindow *miniStatusWindow;
NpcIntegerDialog *npcIntegerDialog;
NpcListDialog *npcListDialog;
NpcStringDialog *npcStringDialog;
NpcTextDialog *npcTextDialog;
SellDialog *sellDialog;
ShortcutWindow *emoteShortcutWindow;
ShortcutWindow *itemShortcutWindow;
SkillDialog *skillDialog;
StatusWindow *statusWindow;
StorageWindow *storageWindow;
TradeWindow *tradeWindow;

BeingManager *beingManager = NULL;
FloorItemManager *floorItemManager = NULL;
Particle* particleEngine = NULL;
Viewport *viewport = NULL;                    /**< Viewport on the map. */

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
                state = EXIT_STATE;

            disconnectedDialog = NULL;
        }
    } exitListener;
}

/**
 * Create all the various globally accessible gui windows
 */
void createGuiWindows()
{
    emoteShortcut = new EmoteShortcut();
    itemShortcut = new ItemShortcut();

    // Initialise player relations
    player_relations.init();

    // Create dialogs
    buyDialog = new BuyDialog();
    buySellDialog = new BuySellDialog();
    chatWindow = new ChatWindow();
    emoteShortcutWindow = new ShortcutWindow(new EmoteShortcutContainer());
    emoteWindow = new EmoteWindow();
    equipmentWindow = new EquipmentWindow();
    inventoryWindow = new InventoryWindow();
    itemShortcutWindow = new ShortcutWindow(new ItemShortcutContainer());
    menuBar = new MenuBar();
    minimap = new Minimap();
    miniStatusWindow = new MiniStatusWindow();
    npcIntegerDialog = new NpcIntegerDialog();
    npcListDialog = new NpcListDialog();
    npcStringDialog = new NpcStringDialog();
    npcTextDialog = new NpcTextDialog();
    sellDialog = new SellDialog();
    skillDialog = new SkillDialog();
    statusWindow = new StatusWindow(player_node);
    storageWindow = new StorageWindow();
    tradeWindow = new TradeWindow();
}

/**
 * Destroy all the globally accessible gui windows
 */
static void destroyGuiWindows()
{
    logger->setChatWindow(NULL);

    delete buyDialog;
    delete buySellDialog;
    delete chatWindow;
    delete emoteShortcutWindow;
    delete emoteWindow;
    delete equipmentWindow;
    delete inventoryWindow;
    delete itemShortcutWindow;
    delete menuBar;
    delete minimap;
    delete miniStatusWindow;
    delete npcIntegerDialog;
    delete npcListDialog;
    delete npcStringDialog;
    delete npcTextDialog;
    delete sellDialog;
    delete skillDialog;
    delete statusWindow;
    delete storageWindow;
    delete tradeWindow;

    // Unload XML databases
    ColorDB::unload();
    EffectDB::unload();
    EmoteDB::unload();
    ItemDB::unload();
    MonsterDB::unload();
    NPCDB::unload();
    SkillDB::unload();
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
    // Create the viewport
    viewport = new Viewport();

    gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());
    top->add(viewport);
    viewport->requestMoveToBottom();

    createGuiWindows();

    // Add game specific tabs to the setup window
    setupGame = new Setup_Game();
    setupWindow->addTab(setupGame);

    setupPlayers = new Setup_Players();
    setupWindow->addTab(setupPlayers);

    mapLoader = new MapLoader();

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
    mapLoader->changeMap(map_path);
    MessageOut outMsg(CMSG_MAP_LOADED);
}

Game::~Game()
{
    destroyGuiWindows();

    setupWindow->removeTab(setupGame);
    setupWindow->removeTab(setupPlayers);

    delete setupGame;
    setupGame = NULL;
    delete setupPlayers;
    setupPlayers = NULL;
    delete beingManager;
    beingManager = NULL;
    delete floorItemManager;
    floorItemManager = NULL;
    delete player_node;
    player_node = NULL;
    delete particleEngine;
    particleEngine = NULL;
    delete mapLoader;
    mapLoader = NULL;
    delete viewport;
    viewport = NULL;

    // Clear the network handlers
    mNetwork->unregisterHandler(mBeingHandler.get());
    mNetwork->unregisterHandler(mBuySellHandler.get());
    mNetwork->unregisterHandler(mChatHandler.get());
    mNetwork->unregisterHandler(mEquipmentHandler.get());
    mNetwork->unregisterHandler(mInventoryHandler.get());
    mNetwork->unregisterHandler(mItemHandler.get());
    mNetwork->unregisterHandler(mNpcHandler.get());
    mNetwork->unregisterHandler(mPlayerHandler.get());
    mNetwork->unregisterHandler(mSkillHandler.get());
    mNetwork->unregisterHandler(mTradeHandler.get());
}

void Game::logic() const
{
    int gameTime = tick_time;

    while (state == GAME_STATE)
    {
        if (mapLoader->getCurrentMap())
            mapLoader->getCurrentMap()->update(get_elapsed_time(gameTime));

        beingManager->logic();

        // Handle all necessary game logic
        while (get_elapsed_time(gameTime) > 0)
        {
            particleEngine->update();
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
                if (!mNetwork->getError().empty()) 
                    errorMessage = mNetwork->getError();
                else
                    errorMessage = _("Got disconnected from server!");

                disconnectedDialog = new OkDialog(_("Network Error"),
                                                    errorMessage);
                disconnectedDialog->addActionListener(&exitListener);
                disconnectedDialog->requestMoveToTop();
            }
        }
    }
}

