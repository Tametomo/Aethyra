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
#include <physfs.h>
#include <SDL_image.h>
#include <unistd.h>
#include <vector>

#include <libxml/parser.h>

#include <SDL/SDL_ttf.h>

#include "configuration.h"
#include "game.h"
#include "log.h"
#include "main.h"
#include "playerrelations.h"

#include "bindings/guichan/graphics.h"
#include "bindings/guichan/gui.h"
#include "bindings/guichan/inputmanager.h"
#include "bindings/guichan/keyboardconfig.h"
#include "bindings/guichan/palette.h"

#ifdef USE_OPENGL
#include "bindings/guichan/opengl/openglgraphics.h"
#endif

#include "bindings/guichan/sdl/sdlgraphics.h"
#include "bindings/guichan/sdl/sdlinput.h"

#include "bindings/sdl/sound.h"

#include "gui/charserver.h"
#include "gui/charselect.h"
#include "gui/debugwindow.h"
#include "gui/desktop.h"
#include "gui/help.h"
#include "gui/login.h"
#include "gui/okdialog.h"
#include "gui/register.h"
#include "gui/setup.h"
#include "gui/updatewindow.h"

#include "net/charserverhandler.h"
#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/maploginhandler.h"
#include "net/messageout.h"
#include "net/network.h"
#include "net/serverinfo.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "resources/db/colordb.h"
#include "resources/db/effectdb.h"
#include "resources/db/emotedb.h"
#include "resources/db/itemdb.h"
#include "resources/db/monsterdb.h"
#include "resources/db/npcdb.h"
#include "resources/db/skilldb.h"

#include "resources/sprite/localplayer.h"

#include "utils/gettext.h"
#include "utils/lockedarray.h"
#include "utils/stringutils.h"

#ifdef __APPLE__
#include <CoreFoundation/CFBundle.h>
#endif

#ifdef WIN32
#include <SDL_syswm.h>
#else
#include <cerrno>
#include <sys/stat.h>
#endif

// Account infos
char n_server, n_character;

Graphics *graphics;

// TODO Anyone knows a good location for this? Or a way to make it non-global?
class SERVER_INFO;
SERVER_INFO **server_info;

bool mInGame = false;
unsigned char state;
std::string errorMessage;
unsigned char screen_mode;

Sound sound;
Music *bgm;
Game *game;

Configuration config;         /**< XML file configuration reader */
Logger *logger;               /**< Log object */
KeyboardConfig keyboard;
InputManager *inputManager;

CharServerHandler charServerHandler;
LoginData loginData;
LockedArray<LocalPlayer*> charInfo(MAX_SLOT + 1);

Palette *guiPalette;

DebugWindow *debugWindow;
HelpWindow *helpWindow;
Setup* setupWindow;

// This anonymous namespace hides whatever is inside from other modules.
namespace {

std::string homeDir;
std::string updateHost;
std::string updatesDir;

LoginHandler loginHandler;
MapLoginHandler mapLoginHandler;

SDL_Surface *icon;

/**
 * A structure holding the values of various options that can be passed from
 * the command line.
 */
struct Options
{
    /**
     * Constructor.
     */
    Options():
        printHelp(false),
        printVersion(false),
        skipUpdate(false),
        chooseDefault(false)
    {};

    bool printHelp;
    bool printVersion;
    bool skipUpdate;
    bool chooseDefault;
    std::string username;
    std::string password;
    std::string playername;
    std::string configPath;
    std::string updateHost;
    std::string dataPath;
};

/**
 * Parse the update host and determine the updates directory
 * Then verify that the directory exists (creating if needed).
 */
static void setUpdatesDir()
{
    std::stringstream updates;

    // If updatesHost is currently empty, fill it from config file
    if (updateHost.empty())
    {
        updateHost =
            config.getValue("updatehost", "http://www.aethyra.org/updates");
    }

    // Remove any trailing slash at the end of the update host
    if (updateHost.at(updateHost.size() - 1) == '/')
        updateHost.resize(updateHost.size() - 1);

    // Parse out any "http://" or "ftp://", and set the updates directory
    size_t pos;
    pos = updateHost.find("://");
    if (pos != updateHost.npos)
    {
        if (pos + 3 < updateHost.length())
        {
            updates << "updates/" << updateHost.substr(pos + 3)
                    << "/" << loginData.port;
            updatesDir = updates.str();
        }
        else
        {
            logger->log("Error: Invalid update host: %s", updateHost.c_str());
            errorMessage = _("Invalid update host: ") + updateHost;
            state = ERROR_STATE;
        }
    }
    else
    {
        logger->log("Warning: no protocol was specified for the update host");
        updates << "updates/" << updateHost << "/" << loginData.port;
        updatesDir = updates.str();
    }

    ResourceManager *resman = ResourceManager::getInstance();

    // Verify that the updates directory exists. Create if necessary.
    if (!resman->isDirectory("/" + updatesDir))
    {
        if (!resman->mkdir("/" + updatesDir))
        {
#if defined WIN32
            std::string newDir = homeDir + "\\" + updatesDir;
            std::string::size_type loc = newDir.find("/", 0);

            while (loc != std::string::npos)
            {
                newDir.replace(loc, 1, "\\");
                loc = newDir.find("/", loc);
            }

            if (!CreateDirectory(newDir.c_str(), 0) &&
                GetLastError() != ERROR_ALREADY_EXISTS)
            {
                logger->log("Error: %s can't be made, but doesn't exist!",
                            newDir.c_str());
                errorMessage = _("Error creating updates directory!");
                state = ERROR_STATE;
            }
#else
            logger->log("Error: %s/%s can't be made, but doesn't exist!",
                        homeDir.c_str(), updatesDir.c_str());
            errorMessage = _("Error creating updates directory!");
            state = ERROR_STATE;
#endif
        }
    }
}

/**
 * Do all initialization stuff
 */
static void init_engine(const Options &options)
{
    homeDir = std::string(PHYSFS_getUserDir()) + "/.aethyra";
#if defined WIN32
    if (!CreateDirectory(homeDir.c_str(), 0) &&
            GetLastError() != ERROR_ALREADY_EXISTS)
#elif defined __APPLE__
    // Use Application Directory instead of .aethyra
    homeDir = std::string(PHYSFS_getUserDir()) +
        "/Library/Application Support/Aethyra";
    if ((mkdir(homeDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) &&
            (errno != EEXIST))
#else
    // Checking if /home/user/.aethyra folder exists.
    if ((mkdir(homeDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) &&
            (errno != EEXIST))
#endif
    {
        std::cout << homeDir
                  << _(" can't be created, but it doesn't exist! Exiting.")
                  << std::endl;
        exit(1);
    }

    // Set log file
    logger->setLogFile(homeDir + std::string("/aethyra.log"));

#ifdef PACKAGE_VERSION
    logger->log("Starting Aethyra Version %s", PACKAGE_VERSION);
#else
    logger->log("Starting Aethyra - Version not defined"));
#endif

    // Initialize SDL
    logger->log("Initializing SDL...");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
    {
        std::cerr << _("Could not initialize SDL: ") << SDL_GetError()
                  << std::endl;
        exit(1);
    }
    atexit(SDL_Quit);

    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    ResourceManager *resman = ResourceManager::getInstance();

    if (!resman->setWriteDir(homeDir))
    {
        std::cout << homeDir
                  << _(" couldn't be set as home directory! Exiting.")
                  << std::endl;
        exit(1);
    }

    // Add the user's homedir to PhysicsFS search path
    resman->addToSearchPath(homeDir, false);

    // Add the main data directories to our PhysicsFS search path
    if (!options.dataPath.empty())
        resman->addToSearchPath(options.dataPath, true);

    resman->addToSearchPath("data", true);
#if defined __APPLE__
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path,
                                          PATH_MAX))
    {
        fprintf(stderr, _("Can't find Resources directory\n"));
    }
    CFRelease(resourcesURL);
    strncat(path, "/data", PATH_MAX - 1);
    resman->addToSearchPath(path, true);
#else
    resman->addToSearchPath(PKG_DATADIR "data", true);
#endif

    // Fill configuration with defaults
    logger->log("Initializing configuration...");
    config.setValue("host", "www.aethyra.org");
    config.setValue("port", 21001);
    config.setValue("hwaccel", 0);
#if defined USE_OPENGL
    config.setValue("opengl", 1);
#else
    config.setValue("opengl", 0);
#endif
    config.setValue("screen", 0);
    config.setValue("sound", 1);
    config.setValue("guialpha", 0.8f);
    config.setValue("remember", 1);
    config.setValue("sfxVolume", 100);
    config.setValue("musicVolume", 60);
    config.setValue("fpslimit", 0);
    config.setValue("updatehost", "http://www.aethyra.org/updates");
    config.setValue("customcursor", 1);
    config.setValue("ChatLogLength", 128);

    // Checking if the configuration file exists... otherwise creates it with
    // default options !
    FILE *configFile = 0;
    std::string configPath = options.configPath;

    if (configPath.empty())
        configPath = homeDir + "/config.xml";

    configFile = fopen(configPath.c_str(), "r");

    // If we can't read it, it doesn't exist !
    if (configFile == NULL)
    {
        // We reopen the file in write mode and we create it
        configFile = fopen(configPath.c_str(), "wt");
    }

    if (configFile == NULL)
    {
        std::cout << "Can't create " << configPath << ". "
            "Using Defaults." << std::endl;
    }
    else
    {
        fclose(configFile);
        config.init(configPath);
    }

    SDL_WM_SetCaption("Aethyra", NULL);
#ifdef WIN32
    static SDL_SysWMinfo pInfo;
    SDL_GetWMInfo(&pInfo);
    HICON icon = LoadIcon(GetModuleHandle(NULL), "A");
    if (icon)
    {
        SetClassLong(pInfo.window, GCL_HICON, (LONG) icon);
    }
#else
    icon = IMG_Load(PKG_DATADIR "data/icons/aethyra.png");
    if (icon)
    {
        SDL_SetAlpha(icon, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
        SDL_WM_SetIcon(icon, NULL);
    }
#endif

#ifdef USE_OPENGL
    bool useOpenGL = (bool) config.getValue("opengl", 0) == 1;

    // Setup image loading for the right image format
    Image::setLoadAsOpenGL(useOpenGL);

    // Create the graphics context
    if (useOpenGL)
        graphics = new OpenGLGraphics();
    else
        graphics = new SDLGraphics();
#else
    // Create the graphics context
    graphics = new SDLGraphics();
#endif

    const int width = (int) config.getValue("screenwidth", defaultScreenWidth);
    const int height = (int) config.getValue("screenheight", defaultScreenHeight);
    const int bpp = 0;
    const bool fullscreen = ((int) config.getValue("screen", 0) == 1);
    const bool hwaccel = ((int) config.getValue("hwaccel", 0) == 1);

    // Try to set the desired video mode
    if (!graphics->setVideoMode(width, height, bpp, fullscreen, hwaccel))
    {
        std::cerr << _("Couldn't set ") << width << "x" << height << "x" 
                  << bpp << _(" video mode: ") << SDL_GetError() << std::endl;
        exit(1);
    }

    // Initialize for drawing
    graphics->_beginDraw();

    gui = new Gui(graphics);
    state = LOGIN_STATE; /**< Initial game state */

    // Initialize sound engine
    try
    {
        if (config.getValue("sound", 0) == 1)
            sound.init();

        sound.setSfxVolume((int) config.getValue("sfxVolume", defaultSfxVolume));
        sound.setMusicVolume((int) config.getValue("musicVolume",
                                                   defaultMusicVolume));
    }
    catch (const char *err)
    {
        state = ERROR_STATE;
        errorMessage = err;
        logger->log("Warning: %s", err);
    }

    // Initialize keyboard
    keyboard.init();

    // Initialise player relations
    player_relations.init();
}

/** Clear the engine */
static void exit_engine()
{
    // Before config.write() so that global windows can get their settings
    // written to the configuration file.
    delete debugWindow;
    delete helpWindow;

    config.write();

    delete gui;
    delete graphics;

    // Shutdown libxml
    xmlCleanupParser();

    // Shutdown sound
    sound.close();

    ResourceManager::deleteInstance();
    delete logger;

    SDL_FreeSurface(icon);
}

static void printHelp()
{
    std::cout
        << _("aethyra") << std::endl << std::endl
        << _("Options: ") << std::endl
        << _("  -C --configfile : Configuration file to use") << std::endl
        << _("  -d --data       : Directory to load game data from") << std::endl
        << _("  -D --default    : Bypass the login process with default settings")
        << std::endl
        << _("  -h --help       : Display this help") << std::endl
        << _("  -H --updatehost : Use this update host") << std::endl
        << _("  -p --playername : Login with this player") << std::endl
        << _("  -P --password   : Login with this password") << std::endl
        << _("  -u --skipupdate : Skip the update downloads") << std::endl
        << _("  -U --username   : Login with this username") << std::endl
        << _("  -v --version    : Display the version") << std::endl;
}

static void printVersion()
{
#ifdef PACKAGE_VERSION
    std::cout << _("Aethyra version ") << PACKAGE_VERSION << std::endl;
#else
    std::cout << _("Aethyra version ")
              << _("(local build?, PACKAGE_VERSION is not defined)")
              << std::endl;
#endif
}

static void parseOptions(int argc, char *argv[], Options &options)
{
    const char *optstring = "hvud:U:P:Dp:C:H:";

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
        }
    }
}

/**
 * Reads the file "{Updates Directory}/resources2.txt" and attempts to load
 * each update mentioned in it.
 */
static void loadUpdates()
{
    if (updatesDir.empty()) return;
    const std::string updatesFile = "/" + updatesDir + "/resources2.txt";
    ResourceManager *resman = ResourceManager::getInstance();
    std::vector<std::string> lines = resman->loadTextFile(updatesFile);

    for (unsigned int i = 0; i < lines.size(); ++i)
    {
        std::stringstream line(lines[i]);
        std::string filename;
        line >> filename;
        resman->addToSearchPath(homeDir + "/" + updatesDir + "/" + filename,
                                false);
    }
}

// TODO Find some nice place for these functions
static void accountLogin(Network *network, LoginData *loginData)
{
    logger->log("Trying to connect to account server...");
    logger->log("Username is %s", loginData->username.c_str());
    network->connect(loginData->hostname, loginData->port);
    network->registerHandler(&loginHandler);
    loginHandler.setLoginData(loginData);

    // Send login infos
    MessageOut outMsg(0x0064);
    outMsg.writeInt32(0); // client version
    outMsg.writeString(loginData->username, 24);
    outMsg.writeString(loginData->password, 24);

    /*
     * eAthena calls the last byte "client version 2", but it isn't used at
     * at all. We're retasking it, with bit 0 to indicate whether the client
     * can handle the 0x63 "update host" packet. Clients prior to 0.0.25 send
     * 0 here.
     */
    outMsg.writeInt8(0x01);

    // Clear the password, avoids auto login when returning to login
    loginData->password = "";

    // Remove _M or _F from username after a login for registration purpose
    if (loginData->registerLogin)
    {
        loginData->username =
            loginData->username.substr(0, loginData->username.length() - 2);
    }

    // TODO This is not the best place to save the config, but at least better
    // than the login gui window
    if (loginData->remember)
    {
        config.setValue("host", loginData->hostname);
        config.setValue("username", loginData->username);
    }
    config.setValue("remember", loginData->remember);
}

static void charLogin(Network *network, LoginData *loginData)
{
    logger->log("Trying to connect to char server...");
    network->connect(loginData->hostname, loginData->port);
    network->registerHandler(&charServerHandler);
    charServerHandler.setCharInfo(&charInfo);
    charServerHandler.setLoginData(loginData);

    // Send login infos
    MessageOut outMsg(0x0065);
    outMsg.writeInt32(loginData->account_ID);
    outMsg.writeInt32(loginData->session_ID1);
    outMsg.writeInt32(loginData->session_ID2);
    // [Fate] The next word is unused by the old char server, so we squeeze in
    //        tmw client version information
    outMsg.writeInt16(CLIENT_PROTOCOL_VERSION);
    outMsg.writeInt8(loginData->sex);

    // We get 4 useless bytes before the real answer comes in
    network->skip(4);
}

static void mapLogin(Network *network, LoginData *loginData)
{
    logger->log("Memorizing selected character %s",
            player_node->getName().c_str());
    config.setValue("lastCharacter", player_node->getName());

    logger->log("Trying to connect to map server...");
    logger->log("Map: %s", map_path.c_str());

    network->connect(loginData->hostname, loginData->port);
    network->registerHandler(&mapLoginHandler);

    // Send login infos
    MessageOut outMsg(0x0072);
    outMsg.writeInt32(loginData->account_ID);
    outMsg.writeInt32(player_node->mCharId);
    outMsg.writeInt32(loginData->session_ID1);
    outMsg.writeInt32(loginData->session_ID2);
    outMsg.writeInt8(loginData->sex);

    // We get 4 useless bytes before the real answer comes in
    network->skip(4);
}

} // namespace

extern "C" char const *_nl_locale_name_default(void);

static void initInternationalization()
{
#if ENABLE_NLS
#ifdef WIN32
    putenv(("LANG=" + std::string(_nl_locale_name_default())).c_str());
    // mingw doesn't like LOCALEDIR to be defined for some reason
    bindtextdomain("aethyra", "translations/");
#else
    bindtextdomain("aethyra", LOCALEDIR);
#endif
    setlocale(LC_MESSAGES, "");
    bind_textdomain_codeset("aethyra", "UTF-8");
    textdomain("aethyra");
#endif
}

/** Main */
int main(int argc, char *argv[])
{
    logger = new Logger();

    Options options;

    parseOptions(argc, argv, options);

    if (options.printHelp)
    {
        printHelp();
        return 0;
    }
    else if (options.printVersion)
    {
        printVersion();
        return 0;
    }

    initInternationalization();

    // Initialize libxml2 and check for potential ABI mismatches between
    // compiled version and the shared library actually used.
    xmlInitParser();
    LIBXML_TEST_VERSION;

    // Redirect libxml errors to /dev/null
    FILE *nullFile = fopen("/dev/null", "w");
    xmlSetGenericErrorFunc(nullFile, NULL);

    // Initialize PhysicsFS
    PHYSFS_init(argv[0]);

    init_engine(options);

    unsigned int oldstate = !state; // We start with a status change.

    // Needs to be created in main, as the updater uses it
    guiPalette = new Palette;

    game = NULL;

    setupWindow = new Setup();
    debugWindow = new DebugWindow();
    helpWindow = new HelpWindow();

    sound.playMusic("Magick - Real.ogg");

    loginData.username = options.username;

    if (loginData.username.empty())
    {
        if (config.getValue("remember", 0))
            loginData.username = config.getValue("username", "");
    }

    if (!options.password.empty())
        loginData.password = options.password;

    loginData.hostname = config.getValue("host", "www.aethyra.org");
    loginData.port = (short) config.getValue("port", 21001);
    loginData.remember = config.getValue("remember", 0);
    loginData.registerLogin = false;

    SDLNet_Init();
    Network *network = new Network();
    InputManager *inputManager = new InputManager();

    gcn::Container *top = static_cast<gcn::Container*>(gui->getTop());

    Desktop *desktop = NULL;

    while (state != EXIT_STATE)
    {
        Window* currentDialog;
        unsigned char errorState = loginData.registerLogin ? REGISTER_STATE :
                                                             LOGIN_STATE;

        if (desktop)
            desktop->resize();

        // Handle SDL events
        inputManager->handleInput();
        network->flush();
        network->dispatchMessages();

        if (network->getState() == Network::NET_ERROR)
        {
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
                case UPDATE_STATE:
                    loadUpdates();

                    desktop->reload();
                    break;

                    // Those states don't cause a network disconnect
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

            if (desktop &&state != ACCOUNT_STATE && state != CHAR_CONNECT_STATE &&
                desktop->getCurrentDialog())
            {
                desktop->removeCurrentDialog();
            }

            switch (state)
            {
                case LOADDATA_STATE:
                    logger->log("State: LOADDATA");

                    // Add customdata directory
                    ResourceManager::getInstance()->searchAndAddArchives(
                        "customdata/", "zip", false);

                    // Load XML databases
                    EffectDB::load();
                    SkillDB::load();
                    ColorDB::load();
                    ItemDB::load();
                    MonsterDB::load();
                    NPCDB::load();
                    EmoteDB::load();
                    Being::load(); // Hairstyles

                    state = CHAR_CONNECT_STATE;
                    break;

                case LOGIN_STATE:
                    logger->log("State: LOGIN");

                    desktop = new Desktop();
                    top->add(desktop);

                    if (!loginData.password.empty())
                    {
                        loginData.registerLogin = false;
                        state = ACCOUNT_STATE;
                    }
                    else
                    {
                        desktop->changeCurrentDialog(new LoginDialog(&loginData));
                    }
                    break;

                case REGISTER_STATE:
                    logger->log("State: REGISTER");
                    desktop->changeCurrentDialog(new RegisterDialog(&loginData));
                    break;

                case CHAR_SERVER_STATE:
                    logger->log("State: CHAR_SERVER");

                    if (n_server == 1)
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
                        desktop->changeCurrentDialog(new ServerSelectDialog(
                                                         &loginData, nextState));

                        if (options.chooseDefault || !options.playername.empty())
                        {
                            currentDialog = desktop->getCurrentDialog();
                            ((ServerSelectDialog*) currentDialog)->action(
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

                    logger->log("State: GAME");
                    game = new Game(network);
                    game->logic();
                    delete game;
                    break;

                case UPDATE_STATE:
                    if (options.skipUpdate)
                    {
                        state = LOADDATA_STATE;
                    }
                    else
                    {
                        // Determine which source to use for the update host
                        if (!options.updateHost.empty())
                            updateHost = options.updateHost;
                        else
                            updateHost = loginData.updateHost;

                        setUpdatesDir();
                        logger->log("State: UPDATE");

                        desktop->changeCurrentDialog(new UpdaterWindow(updateHost,
                                                                       homeDir + "/"
                                                                       + updatesDir));
                    }
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
                    mapLogin(network, &loginData);
                    break;

                case CHAR_CONNECT_STATE:
                    desktop->useProgressBar(_("Connecting to character server..."));
                    charLogin(network, &loginData);
                    break;

                case ACCOUNT_STATE:
                    desktop->useProgressBar(_("Connecting to account server..."));
                    accountLogin(network, &loginData);
                    break;

                default:
                    state = EXIT_STATE;
                    break;
            }
        }
    }

    delete guiPalette;
    delete inputManager;
    delete network;
    SDLNet_Quit();

    if (nullFile)
        fclose(nullFile);

    logger->log("State: EXIT");
    exit_engine();
    PHYSFS_deinit();
    return 0;
}

