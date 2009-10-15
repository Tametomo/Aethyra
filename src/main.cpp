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

#include <getopt.h>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "engine.h"
#include "main.h"
#include "options.h"

#include "bindings/guichan/gui.h"
#include "bindings/guichan/inputmanager.h"

#include "bindings/guichan/dialogs/okdialog.h"

#include "bindings/guichan/widgets/desktop.h"

#include "bindings/sdl/sound.h"

#include "core/configuration.h"
#include "core/log.h"
#include "core/resourcemanager.h"

#include "core/map/sprite/localplayer.h"

#include "core/utils/gettext.h"
#include "core/utils/lockedarray.h"
#include "core/utils/stringutils.h"

#include "eathena/game.h"
 
#include "eathena/db/colordb.h"
#include "eathena/db/effectdb.h"
#include "eathena/db/emotedb.h"
#include "eathena/db/itemdb.h"
#include "eathena/db/monsterdb.h"
#include "eathena/db/npcdb.h"
#include "eathena/db/skilldb.h"

#include "eathena/gui/charselect.h"
#include "eathena/gui/login.h"
#include "eathena/gui/register.h"
#include "eathena/gui/serverlistdialog.h"
#include "eathena/gui/updatewindow.h"

#include "eathena/net/charserverhandler.h"
#include "eathena/net/logindata.h"
#include "eathena/net/loginhandler.h"
#include "eathena/net/maploginhandler.h"
#include "eathena/net/messageout.h"
#include "eathena/net/network.h"
#include "eathena/net/serverinfo.h"

unsigned char state;
std::string errorMessage;

Game *game;
Engine *engine;
Options options;

CharServerHandler charServerHandler;
LoginData loginData;
LockedArray<LocalPlayer*> charInfo(MAX_PLAYER_SLOTS);

Desktop *desktop;

// This anonymous namespace hides whatever is inside from other modules.
namespace
{
std::string updateHost;

LoginHandler loginHandler;
MapLoginHandler mapLoginHandler;

static void printHelp()
{
    std::cout << _("Options: ") << std::endl
              << _("  -C --configfile\t: Configuration file to use") << std::endl
              << _("  -d --data\t\t: Directory to load game data from")
              << std::endl
              << _("  -D --default\t\t: Bypass the login process with default "
                   "settings") << std::endl
              << _("  -h --help\t\t: Display this help") << std::endl
              << _("  -H --updatehost\t: Use this update host") << std::endl
              << _("  -p --playername\t: Login with this player") << std::endl
              << _("  -P --password\t\t: Login with this password") << std::endl
              << _("  -u --skipupdate\t: Skip the update downloads") << std::endl
              << _("  -U --username\t\t: Login with this username") << std::endl
#ifdef USE_OPENGL
              << _("  -O --no-opengl\t: Disable OpenGL for this session")
              << std::endl
#else
              << _("  -O --no-opengl\t: Default (OpenGL has been disabled at "
                   "build time)") << std::endl
#endif
              << _("  -v --version\t\t: Display the version") << std::endl
              << std::endl
              << _("Report bugs at: http://www.aethyra.org/mantis")
              << std::endl
              << _("Homepage: http://www.aethyra.org") << std::endl
              << _("Forums: http://www.aethyra.org/forums") << std::endl
              << _("IRC: irc.freenode.net\tChannel: #aethyra") << std::endl;
}

static void printVersion()
{
#ifdef PACKAGE_VERSION
    std::string version = PACKAGE_VERSION;
#else
    std::string version = "not defined.";
#endif
    std::cout << strprintf(_("Aethyra - Version %s"), version.c_str())
              << std::endl << std::endl
              << _("\tCopyright (C) 2008-2009  Aethyra Development Team")
              << std::endl << std::endl
              << _("License GPLv2+: GNU GPL version 2 or later "
                   "<http://gnu.org/licenses/gpl2.html>") << std::endl
              << _("This is free software: you are free to change and "
                   "redistribute it.") << std::endl
              << _("There is NO WARRANTY, to the extent permitted by law.")
              << std::endl << std::endl;
}

static void parseOptions(int argc, char *argv[])
{
    const char *optstring = "hvud:U:P:Dp:C:H:O";

    const struct option long_options[] = {
        { "configfile", required_argument, 0, 'C' },
        { "data",       required_argument, 0, 'd' },
        { "default",    no_argument,       0, 'D' },
        { "playername", required_argument, 0, 'p' },
        { "password",   required_argument, 0, 'P' },
        { "help",       no_argument,       0, 'h' },
        { "updatehost", required_argument, 0, 'H' },
        { "skipupdate", no_argument,       0, 'u' },
        { "username",   required_argument, 0, 'U' },
        { "no-opengl",  no_argument,       0, 'O' },
        { "version",    no_argument,       0, 'v' },
        { 0 }
    };

    while (optind < argc)
    {

        int result = getopt_long(argc, argv, optstring, long_options, NULL);

        if (result == -1)
            break;

        switch (result)
        {
            case 'C':
                options.configPath = optarg;
                break;
            case 'd':
                options.dataPath = optarg;
                break;
            case 'D':
                options.chooseDefault = true;
                break;
            default: // Unknown option
            case 'h':
                options.printHelp = true;
                break;
            case 'H':
                options.updateHost = optarg;
                break;
            case 'p':
                options.playername = optarg;
                break;
            case 'P':
                options.password = optarg;
                break;
            case 'u':
                options.skipUpdate = true;
                break;
            case 'U':
                options.username = optarg;
                break;
            case 'v':
                options.printVersion = true;
                break;
            case 'O':
                options.noOpenGL = true;
#ifndef USE_OPENGL
                std::cout << _("--no-opengl is set by default (OpenGL has been "
                               "disabled at build time)") << std::endl;
#endif
                break;
        }
    }
}

} // namespace

/** Main */
int main(int argc, char *argv[])
{
    parseOptions(argc, argv);

    if (options.printVersion)
        printVersion();

    if (options.printHelp)
    {
        printHelp();
        return 0;
    }

    engine = new Engine(argv[0]);

    state = START_STATE; /**< Initial game state */

    unsigned int oldstate = !state; // We start with a status change.

    game = NULL;

    loginData.username = options.username;

    if (loginData.username.empty() && config.getValue("remember", 0))
        loginData.username = config.getValue("username", "");

    if (!options.password.empty())
        loginData.password = options.password;

    loginData.hostname = config.getValue("host", "www.aethyra.org");
    loginData.port = (short) config.getValue("port", 21001);
    loginData.remember = config.getValue("remember", 0);
    loginData.registerLogin = false;

    gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());

    while (state != EXIT_STATE)
    {
        Window* currentDialog;
        unsigned char errorState = loginData.registerLogin ? REGISTER_STATE :
                                                             LOGIN_STATE;

        // Handle SDL events
        inputManager->handleInput();
        network->flush();
        network->dispatchMessages();

        const int netState = network->getState();

        if (netState == Network::NET_ERROR || netState == Network::FATAL)
        {
            if (netState == Network::FATAL)
                errorState = EXIT_STATE;

            state = ERROR_STATE;

            if (!network->getError().empty()) 
                errorMessage = network->getError();
            else
                errorMessage = _("Got disconnected from server!");
        }

        gui->logic();

        if (state != oldstate)
        {
            switch (oldstate)
            {
                // These states other than default don't cause a network
                // disconnect
                case UPDATE_STATE:
                    desktop->reload();
                    break;

                case LOADDATA_STATE:
                    break;

                case ACCOUNT_STATE:
                case CHAR_CONNECT_STATE:
                case CONNECTING_STATE:
                    desktop->resetProgressBar();
                    break;

                default:
                    network->disconnect();
                    network->clearHandlers();
                    break;
            }

            oldstate = state;

            switch (state)
            {
                case LOADDATA_STATE:
                    logger->log("State: LOADDATA");

                    // Add customdata directory
                    ResourceManager::getInstance()->searchAndAddArchives(
                        "customdata/", "zip", false);

                    // Load XML databases
                    ColorDB::load();
                    EffectDB::load();
                    EmoteDB::load();
                    ItemDB::load();
                    MonsterDB::load();
                    NPCDB::load();
                    SkillDB::load();
                    Being::load(); // Hairstyles

                    state = CHAR_CONNECT_STATE;
                    break;

                case START_STATE:
                    logger->log("State: START");

                    desktop = new Desktop();
                    top->add(desktop);

                    state = LOGIN_STATE;
                    break;

                case LOGIN_STATE:
                    logger->log("State: LOGIN");

                    if (!loginData.password.empty())
                    {
                        loginData.registerLogin = false;
                        state = ACCOUNT_STATE;
                    }
                    else
                        desktop->changeCurrentDialog(new LoginDialog());
                    break;

                case REGISTER_STATE:
                    logger->log("State: REGISTER");
                    desktop->changeCurrentDialog(new RegisterDialog());
                    break;

                case CHAR_SERVER_STATE:
                    logger->log("State: CHAR_SERVER");

                    if (loginData.servers == 1)
                    {
                        SERVER_INFO *si = *server_info;
                        loginData.hostname = ipToString(si->address);
                        loginData.port = si->port;
                        loginData.updateHost = si->updateHost;
                        state = UPDATE_STATE;
                    }
                    else
                    {
                        int nextState = UPDATE_STATE;
                        desktop->changeCurrentDialog(new ServerListDialog(nextState));

                        if (options.chooseDefault || !options.playername.empty())
                        {
                            currentDialog = desktop->getCurrentDialog();
                            ((ServerListDialog*) currentDialog)->action(
                                gcn::ActionEvent(NULL, "ok"));
                        }
                    }
                    break;
                case CHAR_SELECT_STATE:
                    logger->log("State: CHAR_SELECT");
                    desktop->changeCurrentDialog(new CharSelectDialog(&charInfo,
                                                        (loginData.sex == 0) ?
                                                         GENDER_FEMALE :
                                                         GENDER_MALE));

                    currentDialog = desktop->getCurrentDialog();

                    if (((CharSelectDialog*) currentDialog)->selectByName(
                            options.playername))
                        options.chooseDefault = true;
                    else
                        ((CharSelectDialog*) currentDialog)->selectByName(
                            config.getValue("lastCharacter", ""));

                    if (options.chooseDefault)
                        ((CharSelectDialog*) currentDialog)->action(
                            gcn::ActionEvent(NULL, "ok"));

                    break;

                case GAME_STATE:
                    sound.fadeOutMusic(1000);

                    delete desktop;
                    desktop = NULL;

                    logger->log("State: GAME");
                    game = new Game();
                    game->logic();
                    delete game;
                    game = NULL;

                    network->disconnect();
                    network->clearHandlers();
                    break;

                case UPDATE_STATE:
                    logger->log("State: UPDATE");

                    updateHost = (!options.updateHost.empty() ?
                                  options.updateHost : loginData.updateHost);

                    if (!options.skipUpdate)
                    {
                        UpdaterWindow *updateDialog = new UpdaterWindow(updateHost);
                        desktop->changeCurrentDialog(updateDialog);
                        desktop->reload();
                    }
                    else
                        state = LOADDATA_STATE;

                    break;

                case ERROR_STATE:
                    logger->log("State: ERROR");
                    desktop->showError(new OkDialog(_("Error"), errorMessage),
                                       errorState);
                    network->disconnect();
                    network->clearHandlers();
                    break;

                case CONNECTING_STATE:
                    logger->log("State: CONNECTING");
                    desktop->useProgressBar(_("Connecting to map server..."));
                    mapLoginHandler.login();
                    break;

                case CHAR_CONNECT_STATE:
                    desktop->useProgressBar(_("Connecting to character server..."));
                    charServerHandler.login(&charInfo);
                    break;

                case ACCOUNT_STATE:
                    desktop->useProgressBar(_("Connecting to account server..."));
                    loginHandler.login();
                    break;

                case EXIT_STATE:
                    logger->log("State: EXIT");
                    delete desktop;
                    desktop = NULL;
                    break;

                default:
                    state = EXIT_STATE;
                    break;
            }
        }
    }

    delete engine;
    return 0;
}

