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

#ifndef ENGINE_H
#define ENGINE_H

#ifndef PKG_DATADIR
#define PKG_DATADIR ""
#endif

#include <string>

#include "options.h"

#ifdef USE_OPENGL
#include "bindings/guichan/opengl/openglgraphics.h"
#endif

#include "bindings/guichan/sdl/sdlgraphics.h"

// Default program values
// -------------------
// Screen
// Sound
const short defaultSfxVolume = 100;
const short defaultMusicVolume = 60;

class LoginData;
class Network;
class Options;

class Engine
{
    public:
        Engine(const char *prog, const Options &options);
        ~Engine();

        /**
         * Parse the update host and determine the updates directory
         * Then verify that the directory exists (creating if needed).
         */
        void setUpdatesDir(std::string &updateHost, const LoginData &loginData);

        const std::string &getHomeDir() { return homeDir; }

        const std::string &getUpdatesDir() { return updatesDir; }

        Network *getNetwork() { return network; }

    private:
        void initConfig();
        void initInternationalization();
        void initWindow();

        std::string homeDir;
        std::string updatesDir;

        Options options;
        Network *network;

        FILE *nullFile;
        SDL_Surface *icon;
};

extern Engine *engine;

#endif
