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

#include "music.h"

#include "../log.h"

Music::Music(Mix_Music *music):
    mMusic(music)
{
}

Music::~Music()
{
    Mix_FreeMusic(mMusic);
}


Resource *Music::load(SDL_RWops *rw)
{
    if (Mix_Music *music = Mix_LoadMUS_RW(rw))
        return new Music(music);
    else
    {
        logger->log("Error, failed to load music: %s", Mix_GetError());
        return NULL;
    }
}

bool Music::play(const int loops, const int fadeIn)
{
    if (fadeIn > 0)
        return Mix_FadeInMusic(mMusic, loops, fadeIn);
    else
        return Mix_PlayMusic(mMusic, loops);
}

