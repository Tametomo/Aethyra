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

#include <SDL.h>

#include "modelistmodel.h"

#include "../../../core/log.h"

#include "../../../core/utils/stringutils.h"

#define MIN_H 600
#define MIN_W 800

ModeListModel::ModeListModel()
{
    /* Get available fullscreen/hardware modes */
    SDL_Rect **modes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_HWSURFACE);

    /* Check which modes are available */
    if (modes == (SDL_Rect **)0)
        logger->log("No modes available");
    else if (modes == (SDL_Rect **)-1)
        logger->log("All resolutions available");
    else 
    {
        //logger->log("Available Modes");
        for (int i = 0; modes[i]; ++i)
        {
            if ((modes[i]->w >= MIN_W) && (modes[i]->h >=MIN_H))
            {
               const std::string modeString =
                  toString((int)modes[i]->w) + "x" + toString((int)modes[i]->h);
               //logger->log(modeString.c_str());
               mVideoModes.push_back(modeString);
            }
        }
    }
}
