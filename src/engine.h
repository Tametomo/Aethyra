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

#ifndef ENGINE_H
#define ENGINE_H

#include <string>

#include <SDL.h>

/**
 * The Engine class sets up the core program environment by initializing the
 * window, setting up the graphics instances, and setting up all of the core
 * program functionalities that are independent of any specific program type,
 * including the local directory paths, initializing input devices, and logging
 * functionality.
 */
class Engine
{
    public:
        Engine(const char *prog);
        ~Engine();

        const std::string &getHomeDir() { return homeDir; }

    private:
        void initInternationalization();
        void initSDL();
        void initResman();
        void initConfig();
        void initWindow();
        void initSound();

        void setHomeDir();

        std::string homeDir;

        FILE *nullFile;
        SDL_Surface *icon;
};

extern Engine *engine;

#endif
