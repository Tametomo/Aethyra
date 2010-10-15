/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *  Copyright (C) 2008  Aethyra Development Team
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

#ifndef CHAT_H
#define CHAT_H

#include <string>

#include "../../bindings/guichan/widgets/window.h"

class ChatInput;
class ImageButton;
class ItemLinkHandler;
class Party;
class ProxyWidget;
class Recorder;
class RichTextBox;
class ScrollArea;
class TextInputDialog;
class ToolTip;

#define BY_GM         0   // those should be self-explanatory =)
#define BY_PLAYER     1
#define BY_OTHER      2
#define BY_SERVER     3
#define BY_PARTY      4

#define ACT_WHISPER   5   // getting whispered at
#define ACT_IS        6   // equivalent to "/me" on IRC

#define BY_LOGGER     7

#define DEFAULT_CHAT_WINDOW_SCROLL 7 // 1 means `1/8th of the window size'.

/**
 * The chat window.
 *
 * \ingroup Interface
 */
class ChatWindow : public Window
{
    public:
        /**
         * Constructor.
         */
        ChatWindow();

        /**
         * Destructor: used to write back values to the config file
         */
        ~ChatWindow();

        /**
         * Readjust the chat window's current scroll as needed.
         */
        virtual void logic();

        /**
         * Adds a line of text to our message list. Parameters:
         *
         * @param line Text message.
         * @parem own  Type of message (usually the owner-type).
         */
        void chatLog(std::string line, int own, bool ignoreRecord = false);

        /**
         * Performs action.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Request focus for typing chat message.
         */
        void requestChatFocus();

        /**
         * Updates the recorder state.
         */
        void updateRecorder(const std::string &mes);

        /**
         * Determines whether to send a command or an ordinary message, then
         * contructs packets & sends them.
         *
         * @param nick The character's name to display in front.
         * @param msg  The message text which is to be send.
         *
         * NOTE:
         * The nickname is required by the server, if not specified
         * the message may not be sent unless a command was intended
         * which requires another packet to be constructed! you can
         * achieve this by putting a slash ("/") infront of the
         * message followed by the command name and the message.
         * of course all slash-commands need implemented handler-
         * routines. ;-)
         * remember, a line starting with "@" is not a command that needs
         * to be parsed rather is sent using the normal chat-packet.
         *
         * EXAMPLE:
         * // for an global announcement   /- command
         * chatlog.chat_send("", "/announce Hello to all logged in users!");
         * // for simple message by a user /- message
         * chatlog.chat_send("Zaeiru", "Hello to all users on the screen!");
         */
        void chatSend(const std::string &nick, std::string msg);

        /** Shows a tooltip over the record button */
        void mouseMoved(gcn::MouseEvent &event);

        /** Add the given text to the chat input */
        void addInputText(const std::string &text);

        /** Called to add item to chat */
        void addItemText(const std::string &item);

        /** Used to reset mTmpVisible */
        void widgetShown(const gcn::Event& event);
        void close();

       /**
        * Scrolls the chat window
        *
        * @param amount direction and amount to scroll.  Negative numbers scroll
        * up, positive numbers scroll down.  The absolute amount indicates the
        * amount of 1/8ths of chat window real estate that should be scrolled.
        */
        void scroll(int amount);

        /**
         * party implements the partying chat commands
         *
         * @param command is the party command to perform
         * @param msg is the remainder of the message
         */
        void party(const std::string &command, const std::string &msg);

        /**
         * help implements the /help command
         *
         * @param msg1 is the command that the player needs help on
         * @param msg2 is the sub-command relating to the command
         */
        void help(const std::string &msg1, const std::string &msg2);

        void fontChanged();

    private:
        bool mTmpVisible;
        bool mColorInjection; // Whether or not to allow the user to inject in
                              // color tags.

        void whisper(const std::string &nick, std::string msg);

        /** One item in the chat log */
        struct CHATLOG
        {
            std::string nick;
            std::string text;
            int own;
        };

        ChatInput *mChatInput;      /**< Input box for typing chat messages */
        RichTextBox *mTextOutput;   /**< Text box for displaying chat history */
        ScrollArea *mScrollArea;    /**< Scroll area around text output */
        ItemLinkHandler *mItemLinkHandler; /** Used for showing item popup on
                                               clicking links **/
        TextInputDialog *mRecorderInput; /**< Dialog for getting a string for the
                                              recorder */
        ImageButton *mRecordButton; /**< Button used for recording */
        ProxyWidget *mProxy;        /**< Proxy used to get focus for the chat
                                         input field when tabbing */

        ToolTip *mToolTip;          /**< A tooltip to let the user know what
                                         pressing the record image button will
                                         do during its various states. */

        Recorder *mRecorder;        /**< Recording class */
        char mPartyPrefix;          /**< Messages beginning with the prefix are
                                         sent to the party */
        bool mReturnToggles;        /**< Marks whether <Return> toggles the chat
                                         log or not */
        bool mAutoScroll;           /**< Whether or not the chat window should
                                         autoscroll*/
        Party *mParty;
};
extern ChatWindow *chatWindow;

#endif
