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

#ifndef SOUND_EFFECT_H
#define SOUND_EFFECT_H

#ifdef __APPLE__
#include <SDL_mixer/SDL_mixer.h>
#else
#include <SDL_mixer.h>
#endif

#include "../resource.h"

/**
 * Defines a class for loading and storing sound effects.
 */
class SoundEffect : public Resource
{
    public:
        /**
         * Destructor.
         */
        virtual ~SoundEffect();

        /**
         * Loads a sample from a buffer in memory.
         *
         * @param rw         The SDL_RWops to load the sample data from.
         *
         * @return <code>NULL</code> if the an error occurred, a valid pointer
         *         otherwise.
         */
        static Resource *load(SDL_RWops *rw);

        /**
         * Plays the sample.
         *
         * @param loops     Number of times to repeat the playback.
         * @param volume    Sample playback volume.
         *
         * @return <code>true</code> if the playback started properly
         *         <code>false</code> otherwise.
         */
        virtual bool play(const int loops, const int volume);

    protected:
        /**
         * Constructor.
         */
        SoundEffect(Mix_Chunk *soundEffect): mChunk(soundEffect) {}

        Mix_Chunk *mChunk;
};

#endif
