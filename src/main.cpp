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

#include <getopt.h>
#include <iostream>
#include <unistd.h>

#include "engine.h"
#include "main.h"
#include "options.h"

#include "core/utils/dtor.h"
#include "core/utils/gettext.h"
#include "core/utils/stringutils.h"

#include "eathena/statemanager.h"

Engine *engine;
Options options;
StateManager *stateManager;

namespace
{

static void printHelp()
{
    std::cout << _("Options: ") << std::endl
              << "  -C --configfile\t: " << _("Configuration file to use")
              << std::endl
              << "  -d --data\t\t: " << _("Directory to load game data from")
              << std::endl
              << "  -D --default\t\t: " << _("Bypass the login process with "
                 "default settings") << std::endl
              << "  -h --help\t\t: " << _("Display this help") << std::endl
              << "  -H --updatehost\t: " << _("Use this update host")
              << std::endl
              << "  -p --playername\t: " << _("Login with this player")
              << std::endl
              << "  -P --password\t\t: " << _("Login with this password")
              << std::endl
              << "  -u --skipupdate\t: " << _("Skip the update downloads")
              << std::endl
              << "  -U --username\t\t: " << _("Login with this username")
              << std::endl
              << "  -O --no-opengl\t: "
#ifdef USE_OPENGL
              << _("Disable OpenGL for this session") << std::endl
#else
              << _("Default (OpenGL has been disabled at build time)")
              << std::endl
#endif
              << "  -v --version\t\t: " << _("Display the version") << std::endl
              << std::endl
              << strprintf(_("Report bugs at: %s"),
                             "http://www.aethyra.com/mantis") << std::endl
              << strprintf(_("Homepage: %s"),
                             "http://www.aethyra.com") << std::endl
              << strprintf(_("Forums: %s"),
                             "http://www.aethyra.com/forums") << std::endl
              << strprintf(_("IRC: %s\tChannel: %s"), "irc.freenode.net", 
                             "#aethyra") << std::endl;
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
              << strprintf(_("\tCopyright (C) %s  Aethyra Development Team"),
                             "2008-2010")
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
#if defined(DEBUG) && defined(WIN32)
    // load mingw crash handler. Won't fail if dll is not present.
    LoadLibrary("exchndl.dll");
#endif

    parseOptions(argc, argv);

    if (options.printVersion)
        printVersion();

    if (options.printHelp)
        printHelp();
    else
    {
        engine = new Engine(argv[0]);
        stateManager = new StateManager();

        while (stateManager && !stateManager->isExiting())
            stateManager->logic();

        destroy(stateManager);
        destroy(engine);
    }

    return 0;
}

