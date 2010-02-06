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

#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <memory>
#include <string>

#include "../core/utils/lockedarray.h"

/**
 * All possible client states
 */
enum State {
    ERROR_STATE,
    START_STATE,
    LOGIN_STATE,
    REGISTER_STATE,
    ACCOUNT_STATE,
    CHAR_SERVER_STATE,
    UPDATE_STATE,
    LOADDATA_STATE,
    CHAR_CONNECT_STATE,
    CHAR_SELECT_STATE,
    CONNECTING_STATE,
    GAME_STATE,
    QUIT_STATE,
    LOGOUT_STATE,
    EXIT_STATE
};

// Used to keep track of the map path to use before the viewport is ready
extern std::string map_path;

class CharServerHandler;
class LocalPlayer;
class LoginHandler;
class MapLoginHandler;
class Options;
class SERVER_INFO;

/**
 * Manages state transitions within the program, including initializing all
 * preconditions for the state to transition to.
 *
 * TODO: While the current state system works well for what it's intended for,
 *       it'd be nice in the future to strictly enforce state transitions by
 *       disallowing impossible transition states (e.g. transitioning to the
 *       game state without ever logging into a server, since the client needs
 *       to get the data for the game before being able to load it). However,
 *       stricter transition protection would likely be more overhead than it's
 *       worth.
 */
class StateManager
{
    public:
        StateManager();
        ~StateManager();

        /**
         * Returns an enumerated state which represents the current state that
         * the program is currently in.
         */
        State getState() const { return mState; }

        /**
         * Changes the program's current state to the supplied enumerated state,
         * as well as performing all required state initializations.
         */
        void setState(const State state);

        void nextState();

        /**
         * Handles all repetitive state logic.
         */
        void logic();

        /**
         * Alerts the user when an error has occured, and then changes the
         * program's state to the specified, new state.
         */
        void handleException(const std::string &mes, State returnState);

        /**
         * Whether the program is in a game session or not.
         */
        bool isInGame() { return (mState == GAME_STATE); }

        /**
         * Whether the program is trying to close or not.
         */
        bool isExiting() { return (mState == EXIT_STATE); }

        void promptForQuit();

    private:
        State mState;
        State mNextState;

        LockedArray<LocalPlayer*> mCharInfo;

        std::string mError;

        const std::auto_ptr<CharServerHandler> mCharServerHandler;
        const std::auto_ptr<LoginHandler> mLoginHandler;
        const std::auto_ptr<MapLoginHandler> mMapLoginHandler;
};

extern StateManager *stateManager;

#endif

