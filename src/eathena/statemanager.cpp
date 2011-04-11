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
#include "../bindings/guichan/dialogs/selectiondialog.h"
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

#include "../main.h"

LoginData loginData;
Desktop *desktop = NULL;
Game *game = NULL;

DebugWindow *debugWindow = NULL;
HelpDialog *helpDialog = NULL;
ConfirmDialog *exitConfirm = NULL;
ConfirmDialog *warningDialog = NULL;
OkDialog *errorDialog = NULL;
SelectionDialog *selectionDialog = NULL;
Setup* setupWindow = NULL;

Palette *guiPalette = NULL;

std::string map_path = "";

// Yes, these are global. No, this won't get fixed. It's within a local
// namespace within this class so as to not be globally exposed, but is global
// to avoid having to either friend class WarningListeners and ErrorListeners
// (which has the problem in that these would need to be static within
// StateManager, which in this case, really isn't all that different from a
// global), or from exposing some helper functions which would allow for
// transitioning to these states at an inappropriate time.
namespace
{
    State mNextState = QUIT_STATE;
    State mFailState = LOGIN_STATE;
}

/**
 * Listener used for exiting handling.
 */
class ExitListener : public gcn::ActionListener
{
    void action(const gcn::ActionEvent &event)
    {
        exitConfirm = NULL;

        if (event.getId() == "yes" || event.getId() == "ok")
            stateManager->setState(QUIT_STATE);
    }
} exitListener;

class WarningListener : public gcn::ActionListener
{
    void action(const gcn::ActionEvent &event)
    {
        warningDialog = NULL;

        if (event.getId() == "yes" || event.getId() == "ok")
            stateManager->setState(mNextState);
        else if (event.getId() == "no")
            stateManager->setState(mFailState);
    }
} warningListener;

class ErrorListener : public gcn::ActionListener
{
    void action(const gcn::ActionEvent &event)
    {
        network->clearError();
        errorDialog = NULL;

        stateManager->setState(mNextState);
    }
} errorListener;

class SelectionListener : public gcn::ActionListener
{
    void action(const gcn::ActionEvent &event)
    {
        selectionDialog = NULL;
        config.setValue("opengl", event.getId() == "opengl");

        if (event.getId() == "sdl")
            stateManager->setState(LOGIN_STATE);
        else if (event.getId() == "opengl")
            stateManager->handleWarning(_("Change won't take effect until you "
                                          "restart the client. Would you like "
                                          "to quit now?"), QUIT_STATE,
                                          LOGIN_STATE);
    }
} selectionListener;

StateManager::StateManager() :
    mState(EXIT_STATE),
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

    loginData.hostname = config.getValue("host", "www.aethyraproject.org");
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
            if (desktop)
                desktop->resetProgressBar();

            errorDialog = new OkDialog(_("Error"), mError, NULL, true);
            errorDialog->addActionListener(&errorListener);
            errorDialog->requestMoveToTop();
            break;

        case WARNING_STATE:
            logger->log("State: WARNING");
            if (desktop)
                desktop->resetProgressBar();

            warningDialog = new ConfirmDialog(_("Warning"), mError, NULL, true);
            warningDialog->addActionListener(&warningListener);
            warningDialog->requestMoveToTop();
            break;

        case START_STATE:
            logger->log("State: START");

            destroy(desktop);

            desktop = new Desktop();
            top->add(desktop);

            sound.playMusic("Magick - Real.ogg");

#ifdef USE_OPENGL
            if (options.promptForGraphicsMode)
                setState(MODE_SELECTION_STATE);
            else
#endif
                setState(LOGIN_STATE);
            break;

        case MODE_SELECTION_STATE:
            logger->log("State: MODE_SELECTION");

            selectionDialog = new SelectionDialog(_("Welcome to Aethyra"),
                                                  _("You haven't selected a "
                                                    "graphical mode to use yet. "
                                                    "Which mode would you like "
                                                    "to use?"), NULL, false);
            selectionDialog->addOption("sdl", "SDL");
            selectionDialog->addOption("opengl", "OpenGL");
            selectionDialog->addActionListener(&selectionListener);
            selectionDialog->requestMoveToTop();
            break;

        case LOGIN_STATE:
            logger->log("State: LOGIN");
            desktop->resetProgressBar();

            if (loginData.skipLogin)
            {
                loginData.registerLogin = false;
                setState(LOGINSERV_CONNECT_STATE);
            }
            else
                desktop->changeCurrentDialog(new LoginDialog());
            break;

        case REGISTER_STATE:
            logger->log("State: REGISTER");
            desktop->changeCurrentDialog(new RegisterDialog());
            break;

        case LOGINSERV_CONNECT_STATE:
            logger->log("State: ACCT_CONNECT");
            desktop->useProgressBar(_("Connecting to account server..."));

            mLoginHandler.get()->login();
            break;

        case SERVER_SELECT_STATE:
            logger->log("State: SERVER_SELECT");

            // Only save the login data after a valid login
            if (loginData.remember)
            {
                config.setValue("host", loginData.hostname);
                config.setValue("username", loginData.username);
            }
            config.setValue("remember", loginData.remember);

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

                const std::string &updateHost = (!options.updateHost.empty() ?
                                                  options.updateHost :
                                                  loginData.updateHost);

                if (!options.skipUpdate)
                {
                    desktop->resetProgressBar();
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

            setState(CHARSERV_CONNECT_STATE);
            break;

        case CHARSERV_CONNECT_STATE:
            logger->log("State: CHARSERV_CONNECT");
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

        case MAPSERV_CONNECT_STATE:
            logger->log("State: MAPSERV_CONNECT");
            desktop->useProgressBar(_("Connecting to map server..."));
            network->unregisterHandler(mCharServerHandler.get());
            mMapLoginHandler.get()->login();
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

            sound.fadeOutMusic(1000);

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

    sound.logic();

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

void StateManager::handleWarning(const std::string &mes, const State nextState,
                                 const State failState)
{
    if (warningDialog)
        return;

    mNextState = nextState;
    mFailState = failState;
    mError = mes;

    setState(WARNING_STATE);
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
