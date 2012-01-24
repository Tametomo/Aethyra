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

#ifndef SOUND_H
#define SOUND_H

#ifdef __APPLE__
#include <SDL_mixer/SDL_mixer.h>
#else
#include <SDL_mixer.h>
#endif
#include <string>

class Music;

/** Sound engine
 *
 * \ingroup CORE
 */
class Sound
{
    public:
        /**
         * Constructor.
         */
        Sound();

        /**
         * Destructor.
         */
        ~Sound();

        /**
         * Installs the sound engine.
         */
        void init();

        /**
         * Removes all sound functionalities.
         */
        void close();

        /**
         * Starts background music.
         *
         * @param path The full path to the music file.
         */
        void playMusic(const std::string &path);

        /**
         * Stops currently running background music track.
         */
        void stopMusic();

        /**
         * Fades in background music.
         *
         * @param path The full path to the music file.
         * @param ms   Duration of fade-in effect (ms)
         */
        void fadeInMusic(const std::string &path, const int ms = 1000);

        /**
         * Fades out currently running background music track.
         *
         * @param ms   Duration of fade-out effect (ms)
         */
        void fadeOutMusic(const int ms = 1000);

        /**
         * Fades out a background music and play a new one.
         *
         * @param path The full path to the fade in music file.
         * @param ms Duration of fade-out effect (ms)
         */
        void fadeOutAndPlayMusic(const std::string &path, const int ms = 1000);

        int getMaxVolume() const { return MIX_MAX_VOLUME; }

        void setMusicVolume(int volume);
        void setSfxVolume(int volume);

        /**
         * Plays an item.
         *
         * @param path The resource path to the sound file.
         */
        void playSfx(const std::string &path);

        std::string getCurrentTrack() { return mCurrentMusicFile; }

        /**
         * The sound logic.
         * Currently used to check whether the music file can be freed after
         * a fade out, and whether new music has to be played.
         */
        void logic();

    private:
        /** Logs various info about sound device. */
        void info();

        /** Halts and frees currently playing music. */
        void haltMusic();

        bool mInstalled;

        int mSfxVolume;
        int mMusicVolume;

        /** Music file to transition to when current track finishes fading out */
        std::string mNextMusicPath;

        std::string mCurrentMusicFile;
        Music *mMusic;
};

extern Sound sound;

#endif
