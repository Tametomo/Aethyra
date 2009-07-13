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

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "sdl/sdlinput.h"

class InputManager
{
    public:
        InputManager();
        ~InputManager();

        void forwardInput(const SDL_Event &event);

        void handleInput();

    private:
        void handleJoystickInput(const SDL_Event &event);

        bool handleKeyboardInput(const SDL_Event &event);
};

extern InputManager *inputManager;

#endif
