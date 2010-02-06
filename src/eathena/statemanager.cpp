/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
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

#include "game.h"
#include "statemanager.h"

#include "db/colordb.h"
#include "db/effectdb.h"
#include "db/emotedb.h"
#include "db/itemdb.h"
#include "db/monsterdb.h"
#include "db/npcdb.h"
#include "db/skilldb.h"

#include "gui/charselect.h"
#include "gui/debugwindow.h"
#include "gui/login.h"
#include "gui/register.h"
#include "gui/serverlistdialog.h"
#include "gui/updatewindow.h"

#include "net/charserverhandler.h"
#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/maploginhandler.h"
#include "net/messageout.h"
#include "net/network.h"
#include "net/serverinfo.h"

#include "../options.h"

#include "../bindings/guichan/gui.h"
#include "../bindings/guichan/inputmanager.h"
#include "../bindings/guichan/palette.h"

#include "../bindings/guichan/dialogs/confirmdialog.h"
#include "../bindings/guichan/dialogs/helpdialog.h"
#include "../bindings/guichan/dialogs/okdialog.h"
#include "../bindings/guichan/dialogs/setupdialog.h"

#include "../bindings/guichan/widgets/desktop.h"

#include "../bindings/sdl/sound.h"

#include "../core/configuration.h"
#include "../core/log.h"
#include "../core/resourcemanager.h"

#include "../core/map/sprite/localplayer.h"

#include "../core/utils/dtor.h"
#include "../core/utils/gettext.h"
#include "../core/utils/stringutils.h"

LoginData loginData;
Desktop *desktop = NULL;
Game *game = NULL;

DebugWindow *debugWindow = NULL;
HelpDialog *helpDialog = NULL;
ConfirmDialog *exitConfirm = NULL;
OkDialog *errorDialog = NULL;
Setup* setupWindow = NULL;

Palette *guiPalette = NULL;

/**
 * Listener used for exiting handling.
 */
struct ExitListener : public gcn::ActionListener
{
    void action(const gcn::ActionEvent &event)
    {
        if (event.getId() == "yes" || event.getId() == "ok")
        {
            sound.fadeOutMusic(1000);

            stateManager->setState(QUIT_STATE);
        }

        exitConfirm = NULL;
    }
} exitListener;

struct ErrorListener : public gcn::ActionListener
{
    void action(const gcn::ActionEvent &event)
    {
        network->clearError();
        errorDialog = NULL;

        stateManager->nextState();
    }
} errorListener;

StateManager::StateManager() :
    mState(EXIT_STATE),
    mNextState(EXIT_STATE),
    mCharInfo(MAX_PLAYER_SLOTS),
    mError(""),
    mCharServerHandler(new CharServerHandler()),
    mLoginHandler(new LoginHandler()),
    mMapLoginHandler(new MapLoginHandler())
{
    // Pallete colors are needed for the Setup dialog
    guiPalette = new Palette();

    setupWindow = new Setup();
    debugWindow = new DebugWindow();
    helpDialog = new HelpDialog();

    game = NULL;

    loginData.username = options.username;

    if (loginData.username.empty() && config.getValue("remember", 0))
        loginData.username = config.getValue("username", "");

    loginData.skipLogin = !options.password.empty();

    if (loginData.skipLogin)
        loginData.password = options.password;

    loginData.hostname = config.getValue("host", "www.aethyra.org");
    loginData.port = (short) config.getValue("port", 21001);
    loginData.remember = config.getValue("remember", 0);
    loginData.registerLogin = false;

    SDLNet_Init();
    network = new Network();

    setState(START_STATE);
}

StateManager::~StateManager()
{
    destroy(debugWindow);
    destroy(helpDialog);
    destroy(setupWindow);
    destroy(guiPalette);
    destroy(network);
    SDLNet_Quit();
}

void StateManager::nextState()
{
    setState(mNextState);
}

void StateManager::setState(const State state)
{
    if (mState == state)
        return;

    Window* currentDialog;
    gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());

    mState = state;

    switch (mState)
    {

        case ERROR_STATE:
            logger->log("State: ERROR");
            errorDialog = new OkDialog(_("Error"), mError, NULL, true);
            errorDialog->addActionListener(&errorListener);
            errorDialog->requestMoveToTop();
            break;

        case START_STATE:
            logger->log("State: START");

            destroy(desktop);

            desktop = new Desktop();
            top->add(desktop);

            setState(LOGIN_STATE);
            break;

        case LOGIN_STATE:
            logger->log("State: LOGIN");
            desktop->resetProgressBar();

            if (loginData.skipLogin)
            {
                loginData.registerLogin = false;
                setState(ACCOUNT_STATE);
            }
            else
                desktop->changeCurrentDialog(new LoginDialog());
            break;

        case REGISTER_STATE:
            logger->log("State: REGISTER");
            desktop->changeCurrentDialog(new RegisterDialog());
            break;

        case ACCOUNT_STATE:
            logger->log("State: ACCOUNT");
            desktop->useProgressBar(_("Connecting to account server..."));
            mLoginHandler.get()->login();
            break;

        case CHAR_SERVER_STATE:
            logger->log("State: CHAR_SERVER");

            if (loginData.servers == 1)
            {
                loginData.hostname = ipToString(server_info[0]->address);
                loginData.port = server_info[0]->port;
                loginData.updateHost = server_info[0]->updateHost;
                setState(UPDATE_STATE);
            }
            else
            {
                desktop->changeCurrentDialog(new ServerListDialog());

                if (options.chooseDefault || !options.playername.empty())
                {
                    currentDialog = desktop->getCurrentDialog();
                    ((ServerListDialog*) currentDialog)->action(
                        gcn::ActionEvent(NULL, "ok"));
                }
            }
            break;

        case UPDATE_STATE:
            {
                logger->log("State: UPDATE");
                desktop->resetProgressBar();

                const std::string &updateHost = (!options.updateHost.empty() ?
                                                  options.updateHost :
                                                  loginData.updateHost);

                if (!options.skipUpdate)
                {
                    desktop->changeCurrentDialog(new UpdaterWindow(updateHost));
                }
                else
                {
                    DownloadUpdates *download = new DownloadUpdates(updateHost, NULL);
                    download->addUpdatesToResman();
                    destroy(download);
                    setState(LOADDATA_STATE);
                }
            }
            break;

        case LOADDATA_STATE:
            logger->log("State: LOADDATA");

            // Add customdata directory
            ResourceManager::getInstance()->searchAndAddArchives("customdata/",
                                                                 "zip", false);

            // Load XML databases
            ColorDB::load();
            EffectDB::load();
            EmoteDB::load();
            ItemDB::load();
            MonsterDB::load();
            NPCDB::load();
            SkillDB::load();
            Being::load(); // Hairstyles

            // Reload in case there was a different wallpaper in the updates.
            desktop->reload();

            setState(CHAR_CONNECT_STATE);
            break;

        case CHAR_CONNECT_STATE:
            logger->log("State: CHAR_CONNECT");
            desktop->useProgressBar(_("Connecting to character server..."));
            network->unregisterHandler(mLoginHandler.get());
            mCharServerHandler.get()->login(&mCharInfo);
            break;

        case CHAR_SELECT_STATE:
            logger->log("State: CHAR_SELECT");
            desktop->resetProgressBar();
            desktop->changeCurrentDialog(new CharSelectDialog(&mCharInfo,
                                                (loginData.sex == 0) ?
                                                 GENDER_FEMALE : GENDER_MALE));

            currentDialog = desktop->getCurrentDialog();

            if (((CharSelectDialog*) currentDialog)->selectByName(options.playername))
                options.chooseDefault = true;
            else
                ((CharSelectDialog*) currentDialog)->selectByName(
                    config.getValue("lastCharacter", ""));

            if (options.chooseDefault)
                ((CharSelectDialog*) currentDialog)->action(
                    gcn::ActionEvent(NULL, "ok"));
            break;

        case CONNECTING_STATE:
            logger->log("State: CONNECTING");
            desktop->useProgressBar(_("Connecting to map server..."));
            network->unregisterHandler(mCharServerHandler.get());
            mMapLoginHandler.get()->login();

            setState(GAME_STATE);
            break;

        case GAME_STATE:
            logger->log("State: GAME");
            sound.fadeOutMusic(1000);

            game = new Game();

            destroy(desktop);
            break;

        case QUIT_STATE:
        case LOGOUT_STATE:
            if (mState == QUIT_STATE)
                logger->log("State: QUIT");
            else
                logger->log("State: LOGOUT");

            destroy(game);

            ColorDB::unload();
            EffectDB::unload();
            EmoteDB::unload();
            ItemDB::unload();
            MonsterDB::unload();
            NPCDB::unload();
            SkillDB::unload();

            network->disconnect();
            network->clearHandlers();

            setState(mState == QUIT_STATE ? EXIT_STATE : START_STATE);
            break;

        case EXIT_STATE:
            logger->log("State: EXIT");

            destroy(desktop);
            break;

        default:
            logger->log("State: Unknown state");
            handleException(_("Unknown program state. Exiting."), EXIT_STATE);
            break;
    }
}

void StateManager::logic()
{
    if (game)
        game->logic();

    // Handle SDL events
    inputManager->handleInput();
    gui->logic();

    if (network)
    {
        Network::NetState netState = network->getState();

        // Collect all messages that need to be dispatched, then send them all
        // when there isn't a network problem.
        if (netState == Network::NET_ERROR)
        {
            if (!network->getError().empty()) 
                mError = network->getError();
            else
                mError = _("Got disconnected from server!");

            handleException(mError, LOGOUT_STATE);
        }
        else if (mState != ERROR_STATE)
        {
            network->flush();
            network->dispatchMessages();
        }
    }
}

void StateManager::handleException(const std::string &mes, const State returnState)
{
    if (errorDialog)
        return;

    mNextState = returnState;
    mError = mes;

    setState(ERROR_STATE);
}

void StateManager::promptForQuit()
{
    if (!exitConfirm)
    {
        exitConfirm = new ConfirmDialog(_("Quit"),
                                        _("Are you sure you want to quit?"));
        exitConfirm->addActionListener(&exitListener);
        exitConfirm->requestMoveToTop();
    }
    else
        exitConfirm->action(gcn::ActionEvent(NULL, "no"));
}
