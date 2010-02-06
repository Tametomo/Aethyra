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

#ifndef NETWORK_
#define NETWORK_

#include <map>
#include <SDL_net.h>
#include <SDL_thread.h>
#include <string>

#include "../../core/utils/mutex.h"

/**
 * Protocol version, reported to the eAthena char and mapserver who can adjust
 * the protocol accordingly.
 */
#define CLIENT_PROTOCOL_VERSION      1

class MessageHandler;
class MessageIn;

class Network
{
    public:
        // ERROR replaced by NET_ERROR because already defined in Windows
        enum NetState {
            IDLE,
            CONNECTED,
            CONNECTING,
            DATA,
            NET_ERROR
        };

        friend int networkThread(void *data);
        friend class MessageOut;

        Network();

        ~Network();

        bool connect(const std::string &address, short port);

        void disconnect();

        void registerHandler(MessageHandler *handler);

        void unregisterHandler(MessageHandler *handler);

        void clearHandlers();

        NetState getState() const { return mState; }

        const std::string& getError() const { return mError; }

        bool isConnected() const { return mState == CONNECTED; }

        int getInSize() const { return mInSize; }

        void skip(int len);

        bool messageReady();

        MessageIn getNextMessage();

        void dispatchMessages();

        void flush();

        void clearError();

        void interrupt() { mState = NET_ERROR; }

    private:
        void setError(const std::string &error);

        void fatal(const std::string &error);

        Uint16 readWord(int pos);

        bool realConnect();

        void receive();

        TCPsocket mSocket;

        std::string mAddress;
        short mPort;

        char *mInBuffer, *mOutBuffer;
        unsigned int mInSize, mOutSize;

        unsigned int mToSkip;

        NetState mState;
        std::string mError;

        SDL_Thread *mWorkerThread;
        Mutex mMutex;

        typedef std::map<Uint16, MessageHandler*> MessageHandlers;
        typedef MessageHandlers::iterator MessageHandlerIterator;
        MessageHandlers mMessageHandlers;
};

extern Network *network;

#endif
