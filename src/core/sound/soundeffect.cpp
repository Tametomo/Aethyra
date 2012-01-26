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

#include "soundeffect.h"

#include "../log.h"

SoundEffect::~SoundEffect()
{
    Mix_FreeChunk(mChunk);
}

Resource *SoundEffect::load(SDL_RWops *rw)
{
    // Load the music data and free the RWops structure
    Mix_Chunk *tmpSoundEffect = Mix_LoadWAV_RW(rw, 1);

    if (tmpSoundEffect)
        return new SoundEffect(tmpSoundEffect);
    else
    {
        logger->log("Error, failed to load sound effect: %s", Mix_GetError());
        return NULL;
    }
}

bool SoundEffect::play(const int loops, const int volume)
{
    Mix_VolumeChunk(mChunk, volume);

    return Mix_PlayChannel(-1, mChunk, loops) != -1;
}
