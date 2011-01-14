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

#include <sstream>

#include <guichan/focushandler.hpp>

#include "chat.h"
#include "viewport.h"

#include "../beingmanager.h"
#include "../party.h"

#include "../db/iteminfo.h"
#include "../db/itemdb.h"

#include "../handlers/itemlinkhandler.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../../main.h"

#include "../../bindings/guichan/gui.h"
#include "../../bindings/guichan/layout.h"

#include "../../bindings/guichan/dialogs/textinputdialog.h"

#include "../../bindings/guichan/widgets/chatinput.h"
#include "../../bindings/guichan/widgets/imagebutton.h"
#include "../../bindings/guichan/widgets/proxywidget.h"
#include "../../bindings/guichan/widgets/richtextbox.h"
#include "../../bindings/guichan/widgets/scrollarea.h"
#include "../../bindings/guichan/widgets/tooltip.h"

#include "../../core/configuration.h"
#include "../../core/recorder.h"

#include "../../core/map/sprite/localplayer.h"

#include "../../core/utils/dtor.h"
#include "../../core/utils/gettext.h"
#include "../../core/utils/lockedarray.h"
#include "../../core/utils/stringutils.h"

ChatWindow::ChatWindow():
    Window("", false, NULL, "graphics/gui/gui.xml", true),
    mTmpVisible(false),
    mAutoScroll(false)
{
    setWindowName("Chat");

    setResizable(true);
    setDefaultSize(600, 123, ImageRect::LOWER_LEFT);
    setMinWidth(150);
    setMinHeight(90);

    mItemLinkHandler = new ItemLinkHandler();

    mChatInput = new ChatInput();
    mChatInput->setActionEventId("chatinput");
    mChatInput->addActionListener(this);

    mProxy = new ProxyWidget(mChatInput, FOCUS_WHEN_HIDDEN);
    mChatInput->setProxy(mProxy);

    mRecorderInput = new TextInputDialog(_("File name to record to:"));
    mRecorderInput->addActionListener(this);

    mRecordButton = new ImageButton("graphics/gui/circle-green.png", "record",
                                    this, 3);

    mToolTip = new ToolTip();

    mTextOutput = new RichTextBox(RichTextBox::AUTO_WRAP);
    mTextOutput->setOpaque(false);
    mTextOutput->setMaxRow(config.getValue("ChatLogLength", 128));
    mTextOutput->setWidth(150); // Keep from unneccessary wraps when text is
                                // sent to this before being completely
                                // initialized.
    mTextOutput->setLinkHandler(mItemLinkHandler);

    mScrollArea = new ScrollArea(mTextOutput);
    mScrollArea->setScrollPolicy(gcn::ScrollArea::SHOW_NEVER,
                                 gcn::ScrollArea::SHOW_ALWAYS);
    mScrollArea->setScrollAmount(0, 1);
    mScrollArea->setOpaque(false);

    fontChanged();
    loadWindowState();

    // Read the party prefix
    std::string partyPrefix = config.getValue("PartyPrefix", "$");
    mPartyPrefix = (partyPrefix.empty() ? '$' : partyPrefix.at(0));
    mParty = new Party(this);

    mReturnToggles = config.getValue("ReturnToggles", "0") == "1";
    mRecorder = new Recorder(this);

    // If we closed the client last with the recorder still open, open the file
    // that was open then to resume recording.
    updateRecorder(config.getValue(getWindowName() + "Record", ""));

    // If the player had @assert on in the last session, ask the server to
    // run the @assert command for the player again. Convenience for GMs.
    if (config.getValue(player_node->getName() + "GMassert", 0))
        chatSend(player_node->getName(), "@assert");


    mColorInjection = config.getValue("ChatColorInjection", true);
}

ChatWindow::~ChatWindow()
{
    char partyPrefix[2] = ".";
    *partyPrefix = mPartyPrefix;

    config.setValue("PartyPrefix", partyPrefix);
    config.setValue("ChatColorInjection", mColorInjection);
    config.setValue("ReturnToggles", mReturnToggles ? "1" : "0");

    destroy(mRecorder);
    destroy(mRecorderInput);
    destroy(mToolTip);
    destroy(mItemLinkHandler);
    destroy(mParty);
}

void ChatWindow::logic()
{
    Window::logic();

    if (mAutoScroll)
    {
        mAutoScroll = false;
        mScrollArea->setVerticalScrollAmount(mScrollArea->getVerticalMaxScroll());
    }
}

void ChatWindow::fontChanged()
{
    Window::fontChanged();

    if (mWidgets.size() > 0)
        clear();

    mProxy->setHeight(mRecordButton->getHeight());

    place(0, 0, mScrollArea, 5, 5).setPadding(0);
    place(0, 5, mChatInput, 3).setPadding(1);
    place(3, 5, mProxy).setPadding(0);
    place(4, 5, mRecordButton).setPadding(1);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);
    layout.setMargin(2);

    restoreFocus();
}

void ChatWindow::chatLog(std::string line, Palette::ColorType own,
                         bool ignoreRecord)
{
    // Trim whitespace
    trim(line);

    if (line.empty())
        return;

    std::string nick = "";
    std::string text = line;

    std::string::size_type pos = line.find(" : ");
    if (pos != std::string::npos)
    {
        nick = line.substr(0, pos);
        text = line.substr(pos + 3);
    }
    else
    {
        // Fix the owner of welcome message.
        if (line.substr(0, 7) == "Welcome")
            own = Palette::SERVER;
    }

    // *implements actions in a backwards compatible way*
    if (own == Palette::PLAYER && text.at(0) == '*' &&
        text.at(text.length() - 1) == '*')
    {
        text[0] = ' ';
        text.erase(text.length() - 1);
        own = Palette::IS;
    }

    std::string lineColor = guiPalette->getColorMarkup(own);
    switch (own)
    {
        case Palette::GM:
            if (nick.empty())
                nick = std::string(_("Global announcement: "));
            else
            {
                nick = strprintf(_("Global announcement from %s: "),
                                     nick.c_str());
                lineColor = "##1"; // Equiv. to RichTextBox::RED
            }
            break;
        case Palette::CHAT:
        case Palette::PLAYER:
        case Palette::PARTY:
            nick += ": ";
            break;
        case Palette::SERVER:
            nick = _("Server:");
            nick += " ";
            text = line;
            break;
        case Palette::WHISPER:
            nick = strprintf(_("%s whispers: "), nick.c_str());
            break;
        case Palette::IS:
            nick += "";
            break;
        case Palette::LOGGER:
        default:
            text = line;
            break;
    }

    if (nick == ": ")
    {
        nick = "";
        lineColor = guiPalette->getColorMarkup(Palette::SERVER);
    }

    // Remove any special formatting that players might do to their nicknames.
    mTextOutput->sanitizeText(nick);
    mTextOutput->sanitizeText(text, mColorInjection ? 3 : 2);

    // If we are allowing for color injection, ensure we don't print out empty
    // lines in the chat window.
    if (mColorInjection)
    {
        std::string sanitizedText = text;
        std::string::size_type start = sanitizedText.find("##");
        std::string::size_type end = start;
        while (end != std::string::npos && sanitizedText.size() > start + 2)
        {
            while (end < sanitizedText.size() && sanitizedText[end] == '#')
                end++;

            if (sanitizedText[start + 2] != '#')
                sanitizedText.erase(start, 3);

            start = sanitizedText.find("##", end + 1);
            end = start;
        }
        trim(sanitizedText);

        if (sanitizedText.empty())
            return;
    }

    if (nick.empty() && text.substr(0, 17) == "Visible GM status")
        player_node->setGM();

    // Get the current system time
    time_t t;
    time(&t);

    // Format the time string properly
    std::ostringstream timeStr;
    timeStr << "[" << ((((t / 60) / 60) % 24 < 10) ? "0" : "")
            << (int) (((t / 60) / 60) % 24)
            << ":" << (((t / 60) % 60 < 10) ? "0" : "")
            << (int) ((t / 60) % 60) << "] ";

    // Check for item link
    std::string::size_type start = text.find('[');
    while (start != std::string::npos && text[start+1] != '@')
    {
        std::string::size_type end = text.find(']', start);
        if (start+1 != end && end != std::string::npos)
        {
            // Catch multiple embeds and ignore them
            // so it doesn't crash the client.
            while ((text.find('[', start + 1) != std::string::npos) &&
                   (text.find('[', start + 1) < end))
            {
                start = text.find('[', start + 1);
            }

            std::string temp = text.substr(start+1, end - start - 1);

            trim(temp);

            for (unsigned int i = 0; i < temp.size(); i++)
            {
                temp[i] = (char) tolower(temp[i]);
            }

            const ItemInfo itemInfo = ItemDB::get(temp);
            if (itemInfo.getName() != _("Unknown item"))
            {
                text.insert(end, "@@");
                text.insert(start+1, "|");
                text.insert(start+1, toString(itemInfo.getId()));
                text.insert(start+1, "@@");
            }
        }
        start = text.find('[', start + 1);
    }

    line = lineColor + timeStr.str() + nick + text;

    // We look if the Vertical Scroll Bar is set at the max before adding a
    // row, in order to check if we should scroll on the next logic loop.
    mAutoScroll = (mScrollArea->getVerticalScrollAmount() ==
                   mScrollArea->getVerticalMaxScroll());

    mTextOutput->addRow(line);
    mRecorder->record(line.substr(3));
}

void ChatWindow::action(const gcn::ActionEvent & event)
{
    Window::action(event);

    if (event.getId() == "chatinput")
    {
        std::string message = mChatInput->getText();

        if (!message.empty())
        {
            mChatInput->pushToHistory(message);

            // Send the message to the server
            chatSend(player_node->getName(), message);

            // Clear the text from the chat input
            mChatInput->setText("");
        }

        if (message.empty() || !mReturnToggles)
        {
            // Remove focus and hide input
            mFocusHandler->focusNone();

            // If the chatWindow is shown up because you want to send a message
            // It should hide now
            if (mTmpVisible)
                setVisible(false);
        }
    }
    else if (event.getId() == "record")
    {
        if (!mRecorder->isRecording())
            mRecorderInput->setVisible(!mRecorderInput->isVisible());
        else
            updateRecorder("");
    }
    else if (event.getId() == "ok")
        updateRecorder(mRecorderInput->getValue());
    else if (event.getId() == "cancel")
    {
        mRecorderInput->reset();
        mRecorderInput->setVisible(false);
    }
}

void ChatWindow::requestChatFocus()
{
    // Make sure chatWindow is visible
    if (!isVisible())
    {
        setVisible(true);

        /*
         * This is used to hide chatWindow after sending the message. There is
         * a trick here, because setVisible will set mTmpVisible to false, you
         * have to put this sentence *after* setVisible, not before it
         */
        mTmpVisible = true;
    }

    // Give focus to the chat input
    mChatInput->setVisible(true);
    mChatInput->requestFocus();
}

void ChatWindow::updateRecorder(const std::string &mes)
{
    mRecorder->changeRecordingStatus(mes);
    mRecordButton->changeImage(strprintf("graphics/gui/%s",
                              (mRecorder->isRecording() ? "circle-red.png" :
                               "circle-green.png")));
    mRecorderInput->setVisible(false);

    const std::string tip = (mRecorder->isRecording() ? _("Stop Recording") :
                                                        _("Start Recording"));

    mToolTip->setText(tip);
    mToolTip->adjustSize();
}

void ChatWindow::whisper(const std::string &nick, std::string msg)
{
    std::string recvnick = "";

    if (msg.substr(0, 1) == "\"")
    {
        const std::string::size_type pos = msg.find('"', 1);
        if (pos != std::string::npos)
        {
            recvnick = msg.substr(1, pos - 1);
            msg.erase(0, pos + 2);
        }
    }
    else
    {
        const std::string::size_type pos = msg.find(" ");
        if (pos != std::string::npos)
        {
            recvnick = msg.substr(0, pos);
            msg.erase(0, pos + 1);
        }
    }

    trim(msg);

    std::string playerName = player_node->getName();
    std::string tempNick = recvnick;

    toLower(playerName);
    toLower(tempNick);

    if (tempNick.compare(playerName) == 0 || msg.empty())
        return;

    MessageOut outMsg(CMSG_CHAT_WHISPER);
    outMsg.writeInt16(msg.length() + 28);
    outMsg.writeString(recvnick, 24);
    outMsg.writeString(msg, msg.length());

    chatLog(strprintf(_("Whispering to %s: %s"), recvnick.c_str(), msg.c_str()),
                        Palette::PLAYER);
}

void ChatWindow::chatSend(const std::string &nick, std::string msg)
{
    /* Some messages are managed client side, while others
     * require server handling by proper packet. Probably
     * those if elses should be replaced by protocol calls */

    trim(msg);

    if (msg.compare("") == 0)
        return;

    // Send party message
    if (msg.at(0) == mPartyPrefix)
    {
        msg.erase(0, 1);
        std::size_t length = msg.length() + 1;

        if (length == 0)
        {
            chatLog(_("Trying to send a blank party message."));
            return;
        }

        MessageOut outMsg(CMSG_PARTY_MESSAGE);
        outMsg.writeInt16(length + 4);
        outMsg.writeString(msg, length);
        return;
    }

    // Prepare ordinary message
    if (msg.substr(0, 1) != "/")
    {
        msg = nick + " : " + msg;

        MessageOut outMsg(CMSG_CHAT_MESSAGE);
        // Added + 1 in order to let eAthena parse admin commands correctly
        outMsg.writeInt16(msg.length() + 4 + 1);
        outMsg.writeString(msg, msg.length() + 1);
        return;
    }

    msg.erase(0, 1);
    trim(msg);

    std::size_t space = msg.find(" ");
    std::string command = msg.substr(0, space);

    if (space == std::string::npos)
        msg = "";
    else
    {
        msg = msg.substr(space);
        trim(msg);
    }

    if (command == "announce")
    {
        MessageOut outMsg(0x0099);
        outMsg.writeInt16(msg.length() + 4);
        outMsg.writeString(msg, msg.length());
    }
    else if (command == "help")
    {
        trim(msg);
        std::size_t space = msg.find(" ");
        std::string msg1;

        if (space == std::string::npos)
            msg1 = "";
        else
        {
            msg1 = msg.substr(space + 1, msg.length());
            msg = msg.substr(0, space);
        }

        if (!msg.empty() && msg.at(0) == '/')
            msg.erase(0, 1);

        trim(msg1);
        help(msg, msg1);
    }
    else if (command == "where")
    {
        // Display the current map, X, and Y
        std::ostringstream where;
        where << viewport->getMapPath() << " " << player_node->mX << ","
              << player_node->mY;
        chatLog(where.str());
    }
    else if (command == "who")
        MessageOut outMsg(0x00c1);
    else if (command == "clear")
        mTextOutput->clearRows();
    else if (command == "whisper" || command == "msg" || command == "w")
        whisper(nick, msg);
    else if (command == "record")
        updateRecorder(msg);
    else if (command == "toggle")
    {
        if (msg.empty())
        {
            chatLog(mReturnToggles ? _("Return toggles chat.") :
                                     _("Message closes chat."));
            return;
        }

        msg = msg.substr(0, 1);

        if (msg == "1" || msg == "y" || msg == "Y" || msg == "t" || msg == "T")
        {
            chatLog(_("Return now toggles chat."));
            mReturnToggles = true;
            return;
        }
        else if (msg == "0" || msg == "n" || msg == "N" ||
                 msg == "f" || msg == "F")
        {
            chatLog(_("Message now closes chat."));
            mReturnToggles = false;
            return;
        }
        else
            chatLog(_("Options to /toggle are \"yes\", \"no\", \"true\", "
                      "\"false\", \"1\", \"0\"."));
    }
    else if (command == "party")
    {
        if (msg.empty())
        {
            chatLog(_("Unknown party command... Type \"/help\" party for more "
                      "information."));
            return;
        }

        const std::string::size_type space = msg.find(" ");
        std::string rest = (space == std::string::npos ? ""
                            : msg.substr(space + 1, msg.length()));

        if (!rest.empty())
        {
            msg = msg.substr(0, space);
            trim(msg);
        }

        party(msg, rest);
        return;
    }
    else if (command == "present")
    {
        const Beings &beings = beingManager->getAll();
        std::string response = "";
        unsigned short playercount = 0;
        char cpc[25];

        for (Beings::const_iterator bi = beings.begin(), be = beings.end();
             bi != be; ++bi)
        {
            if ((*bi)->getType() == Being::PLAYER)
            {
                if (!response.empty())
                    response += ", ";

                response += (*bi)->getName();
                ++playercount;
            }
        }

        sprintf(cpc, _("%u players are present."), playercount);
        if (mRecorder->isRecording())
        {
            // Get the current system time
            time_t t;
            time(&t);

            // Format the time string properly
            std::stringstream timeStr;
            timeStr << "[" << ((((t / 60) / 60) % 24 < 10) ? "0" : "")
                    << (int) (((t / 60) / 60) % 24) << ":"
                    << (((t / 60) % 60 < 10) ? "0" : "")
                    << (int) ((t / 60) % 60) << "] ";

            mRecorder->record(timeStr.str() + strprintf(_("Present: %s; %s"),
                             response.c_str(), cpc));
            chatLog(_("Attendance written to record log."), Palette::SERVER,
                    true);
        }
        else
        {
            chatLog(_("Present: ") + response + "; " + cpc);
        }
    }
    else if (command == "me")
    {
        std::stringstream actionStr;
        actionStr << "*" << msg << "*";
        chatSend(player_node->getName(), actionStr.str());
    }
    else if (command == "version" || command == "v")
    {
#ifdef PACKAGE_VERSION
        std::string version = PACKAGE_VERSION;
#else
        std::string version = "not defined.";
#endif
        chatLog(strprintf(_("Aethyra - Version %s"), version.c_str()));
    }
    else
    {
        chatLog(_("Unknown command."));
    }
}

void ChatWindow::scroll(int amount)
{
    if (!isVisible())
        return;

    int range = mScrollArea->getHeight() / 8 * amount;
    gcn::Rectangle scr;
    scr.y = mScrollArea->getVerticalScrollAmount() + range;
    scr.height = abs(range);
    mTextOutput->showPart(scr);
}

// Show Tooltip
void ChatWindow::mouseMoved(gcn::MouseEvent &event)
{
    Window::mouseMoved(event);

    if (event.getSource() == mRecordButton)
        mToolTip->view(gui->getMouseX(), gui->getMouseY());
    else
        mToolTip->setVisible(false);
}

void ChatWindow::addInputText(const std::string &text)
{
    const int caretPos = mChatInput->getCaretPosition();
    const std::string inputText = mChatInput->getText();

    std::ostringstream ss;
    ss << inputText.substr(0, caretPos) << text << " ";
    ss << inputText.substr(caretPos);

    mChatInput->setText(ss.str());
    mChatInput->setCaretPosition(caretPos + text.length() + 1);
    requestChatFocus();
}

void ChatWindow::addItemText(const std::string &item)
{
    std::ostringstream text;
    text << "[" << item << "]";
    addInputText(text.str());
}

void ChatWindow::widgetShown(const gcn::Event& event)
{
    Window::widgetShown(event);

    /*
     * For whatever reason, if setVisible is called, the mTmpVisible effect
     * should be disabled.
     */

    mTmpVisible = false;
}

void ChatWindow::close()
{
    Window::close();
    mTmpVisible = false;
}

void ChatWindow::party(const std::string &command, const std::string &rest)
{
    if (command == "prefix")
    {
        if (rest.empty())
        {
            char temp[2] = ".";
            *temp = mPartyPrefix;
            chatLog(strprintf(_("The current party prefix is %s."),
                                std::string(temp).c_str()));
        }
        else if (rest.length() != 1)
            chatLog(_("Party prefix must be one character long."));
        else
        {
            if (rest == "/")
                chatLog(_("Cannot use a '/' as the prefix."));
            else
            {
                mPartyPrefix = rest.at(0);
                chatLog(strprintf(_("Changing prefix to %s."), rest.c_str()));
            }
        }
    }
    else
        mParty->respond(command, rest);
}

void ChatWindow::help(const std::string &msg1, const std::string &msg2)
{
    chatLog(_("-- Help --"));
    if (msg1.empty())
    {
        chatLog(_("/announce: Global announcement (GM only)."));
        chatLog(_("/clear: Clears this window."));
        chatLog(_("/help: Display this help."));
        chatLog(_("/me <message>: Tell something about yourself."));
        chatLog(_("/msg <nick> <message>: Alternate form for /whisper."));
        chatLog(_("/party <command> <params>: Party commands."));
        chatLog(_("/present: Get list of players present."));
        chatLog(_("/record <filename>: Start recording the chat to an "
                  "external file."));
        chatLog(_("/toggle: Determine whether <return> toggles the chat log."));
        chatLog(_("/version: Displays the client version"));
        chatLog(_("/w <nick> <message>: Short form for /whisper."));
        chatLog(_("/where: Display map name."));
        chatLog(_("/whisper <nick> <message>: Sends a private <message> "
                  "to <nick>."));
        chatLog(_("/who: Display number of online users."));
        chatLog(_("For more information, type /help <command>."));
    }
    else if (msg1 == "announce")
    {
        chatLog(_("Command: /announce <msg>."));
        chatLog(_("*** only available to a GM ***"));
        chatLog(_("This command sends the message <msg> to "
                  "all players currently online."));
    }
    else if (msg1 == "clear")
    {
        chatLog(_("Command: /clear"));
        chatLog(_("This command clears the chat log of previous chat."));
    }
    else if (msg1 == "help")
    {
        chatLog(_("Command: /help"));
        chatLog(_("This command displays a list of all commands available."));
        chatLog(_("Command: /help <command>"));
        chatLog(_("This command displays help on <command>."));
    }
    else if (msg1 == "me")
    {
        chatLog(_("Command: /me <msg>"));
        chatLog(_("This command tell others you are (doing) <msg>."));
    }
    else if (msg1 == "party")
    {
        mParty->help(msg2);
    }
    else if (msg1 == "present")
    {
        chatLog(_("Command: /present"));
        chatLog(_("This command gets a list of players within hearing and "
                  "sends it to either the record log if recording, or the chat "
                  "log otherwise."));
    }
    else if (msg1 == "record")
    {
        chatLog(_("Command: /record <filename>"));
        chatLog(_("This command starts recording the chat log to the file "
                  "<filename>."));
        chatLog(_("Command: /record"));
        chatLog(_("This command finishes a recording session."));
    }
    else if (msg1 == "toggle")
    {
        chatLog(_("Command: /toggle <state>"));
        chatLog(_("This command sets whether the return key should toggle the "
                  "chat log, or whether the chat log turns off automatically."));
        chatLog(_("<state> can be one of \"1\", \"yes\", \"true\" to "
                  "turn the toggle on, or \"0\", \"no\", \"false\" to turn the "
                  "toggle off."));
        chatLog(_("Command: /toggle"));
        chatLog(_("This command displays the return toggle status."));
    }
    else if (msg1 == "version" || msg1 == "v")
    {
        chatLog(_("Command: /version"));
        chatLog(_("This command displays the current client version you are "
                  "using."));
    }
    else if (msg1 == "where")
    {
        chatLog(_("Command: /where"));
        chatLog(_("This command displays the name of the current map."));
    }
    else if (msg1 == "whisper" || msg1 == "msg" || msg1 == "w")
    {
        chatLog(_("Command: /msg <nick> <msg>"));
        chatLog(_("Command: /whisper <nick> <msg>"));
        chatLog(_("Command: /w <nick> <msg>"));
        chatLog(_("This command sends the message <msg> to <nick>."));
        chatLog(_("If the <nick> has spaces in it, enclose it in "
                  "double quotes (\")."));
    }
    else if (msg1 == "who")
    {
        chatLog(_("Command: /who"));
        chatLog(_("This command displays the number of players currently "
                  "online."));
    }
    else
    {
        chatLog(_("Unknown command."));
        chatLog(_("Type /help for a list of commands."));
    }
}
