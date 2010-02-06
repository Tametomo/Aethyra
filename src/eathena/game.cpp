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

#include "../bindings/guichan/gui.h"
#include "../bindings/guichan/inputmanager.h"

#include "../bindings/guichan/dialogs/setupdialog.h"

#include "../bindings/guichan/widgets/shortcutwindow.h"

#include "../core/configuration.h"
#include "../core/log.h"

#include "../core/image/particle/particle.h"

#include "../core/map/map.h"

#include "../core/map/sprite/localplayer.h"

#include "../core/utils/dtor.h"

std::string map_path;

MapLoader *mapLoader = NULL;

EmoteShortcut *emoteShortcut = NULL;
ItemShortcut *itemShortcut = NULL;

BuyDialog *buyDialog = NULL;
BuySellDialog *buySellDialog = NULL;
ChatWindow *chatWindow = NULL;
EmoteWindow *emoteWindow = NULL;
EquipmentWindow *equipmentWindow = NULL;
InventoryWindow *inventoryWindow = NULL;
MenuBar *menuBar = NULL;
Minimap *minimap = NULL;
MiniStatusWindow *miniStatusWindow = NULL;
NpcIntegerDialog *npcIntegerDialog = NULL;
NpcListDialog *npcListDialog = NULL;
NpcStringDialog *npcStringDialog = NULL;
NpcTextDialog *npcTextDialog = NULL;
SellDialog *sellDialog = NULL;
ShortcutWindow *emoteShortcutWindow = NULL;
ShortcutWindow *itemShortcutWindow = NULL;
SkillDialog *skillDialog = NULL;
StatusWindow *statusWindow = NULL;
StorageWindow *storageWindow = NULL;
TradeWindow *tradeWindow = NULL;

Setup_Game *setupGame = NULL;
Setup_Players *setupPlayers = NULL;

BeingManager *beingManager = NULL;
FloorItemManager *floorItemManager = NULL;
Particle* particleEngine = NULL;
Viewport *viewport = NULL;                    /**< Viewport on the map. */

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

    // Add game specific tabs to the setup window
    setupGame = new Setup_Game();
    setupWindow->addTab(setupGame);

    setupPlayers = new Setup_Players();
    setupWindow->addTab(setupPlayers);
}

/**
 * Destroy all the globally accessible gui windows
 */
static void destroyGuiWindows()
{
    logger->setChatWindow(NULL);

    destroy(buyDialog);
    destroy(buySellDialog);
    destroy(chatWindow);
    destroy(emoteShortcutWindow);
    destroy(emoteWindow);
    destroy(equipmentWindow);
    destroy(inventoryWindow);
    destroy(itemShortcutWindow);
    destroy(menuBar);
    destroy(minimap);
    destroy(miniStatusWindow);
    destroy(npcIntegerDialog);
    destroy(npcListDialog);
    destroy(npcStringDialog);
    destroy(npcTextDialog);
    destroy(sellDialog);
    destroy(skillDialog);
    destroy(statusWindow);
    destroy(storageWindow);
    destroy(tradeWindow);

    // Remove some game specific tabs from the setup window
    setupWindow->removeTab(setupGame);
    setupWindow->removeTab(setupPlayers);

    destroy(setupGame);
    destroy(setupPlayers);

    // Unload XML databases
    ColorDB::unload();
    EffectDB::unload();
    EmoteDB::unload();
    ItemDB::unload();
    MonsterDB::unload();
    NPCDB::unload();
    SkillDB::unload();
}

Game::Game():
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

    beingManager = new BeingManager();
    floorItemManager = new FloorItemManager();

    // Initialize beings
    beingManager->setPlayer(player_node);

    mapLoader = new MapLoader();
    particleEngine = new Particle(NULL);
    particleEngine->setupEngine();

    // Create the viewport
    viewport = new Viewport();

    gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());
    top->add(viewport);
    viewport->requestMoveToBottom();
    viewport->setVisible(false);

    createGuiWindows();

    map_path = map_path.substr(0, map_path.rfind("."));
    mapLoader->changeMap(map_path);
    MessageOut outMsg(CMSG_MAP_LOADED);

    mGameTime = tick_time;
}

Game::~Game()
{
    destroyGuiWindows();

    destroy(beingManager);
    destroy(floorItemManager);
    destroy(player_node);
    destroy(particleEngine);
    destroy(mapLoader);
    destroy(viewport);

    // Clear the network handlers
    network->unregisterHandler(mBeingHandler.get());
    network->unregisterHandler(mBuySellHandler.get());
    network->unregisterHandler(mChatHandler.get());
    network->unregisterHandler(mEquipmentHandler.get());
    network->unregisterHandler(mInventoryHandler.get());
    network->unregisterHandler(mItemHandler.get());
    network->unregisterHandler(mNpcHandler.get());
    network->unregisterHandler(mPlayerHandler.get());
    network->unregisterHandler(mSkillHandler.get());
    network->unregisterHandler(mTradeHandler.get());
}

void Game::logic()
{
    if (mapLoader->getCurrentMap())
        mapLoader->getCurrentMap()->update(get_elapsed_time(mGameTime));

    beingManager->logic();

    // Update the particle engine
    // TODO: Modify the particle engine to be able to update asynchronously
    //       based on the ticks elapsed since the last update.
    while (get_elapsed_time(mGameTime) > 0)
    {
        particleEngine->update();
        mGameTime++;
    }

    if (!network->isConnected())
        network->interrupt();

    mGameTime = tick_time;
}

