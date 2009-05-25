/*
 *  Aethyra
 *  Copyright (C) 2008, The Legend of Mazzeroth Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Legend of Mazzeroth.
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

#include <guichan/exception.hpp>
#include <guichan/font.hpp>

#include <guichan/widgets/label.hpp>

#include "speechbubble.h"
#include "viewport.h"

#include "../configlistener.h"
#include "../configuration.h"

#include "../bindings/guichan/gui.h"
#include "../bindings/guichan/graphics.h"
#include "../bindings/guichan/skin.h"

#include "../bindings/guichan/handlers/wordtextwraphandler.h"

#include "../bindings/guichan/widgets/textbox.h"

class SpeechBubbleConfigListener : public ConfigListener
{
    public:
        SpeechBubbleConfigListener(SpeechBubble *speech):
            mSpeechBubble(speech)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "fontSize")
            {
                mSpeechBubble->mCaption->setFont(gui->getBoldFont());
                mSpeechBubble->mCaption->adjustSize();
                mSpeechBubble->setText(mSpeechBubble->mText);
                mSpeechBubble->adjustSize();
            }
        }
    private:
        SpeechBubble *mSpeechBubble;
};

SpeechBubble::SpeechBubble():
    mText("")
{
    setPopupName("Speech");
    setDefaultSkinPath("graphics/gui/speechbubble.xml");

    if (!viewport)
        throw GCN_EXCEPTION("SpeechBubble::SpeechBubble(): no viewport set");

    if (!skinLoader)
        skinLoader = new SkinLoader();

    setPadding(3);

    // Add this speech bubble to the viewport
    viewport->add(this);

    // SpeechBubbles are invisible by default
    setVisible(false);

    setContentSize(140, 46);

    mConfigListener = new SpeechBubbleConfigListener(this);
    config.addListener("fontSize", mConfigListener);

    mCaption = new gcn::Label("");
    mCaption->setFont(gui->getBoldFont());

    mSpeechBox = new TextBox(new WordTextWrapHandler());
    mSpeechBox->setEditable(false);
    mSpeechBox->setOpaque(false);
    mSpeechBox->setTextColor(&guiPalette->getColor(Palette::CHAT));

    add(mCaption);
    add(mSpeechBox);

    loadPopupConfiguration();
    setMinWidth(mSkin->getMinWidth());
    setMinHeight(mSkin->getMinHeight());

    requestMoveToBottom();
}

SpeechBubble::~SpeechBubble()
{
    config.removeListener("fontSize", mConfigListener);
    delete mConfigListener;
}

const std::string &SpeechBubble::getCaption()
{
    return mCaption->getCaption();
}

void SpeechBubble::setCaption(const std::string &name, const gcn::Color *color)
{
    mCaption->setFont(gui->getBoldFont());
    mCaption->setCaption(name);
    mCaption->adjustSize();
    mCaption->setForegroundColor(*color);
}

void SpeechBubble::setText(std::string text)
{
    mText = text;

    int width = mCaption->getWidth() + 2 * getPadding();
    mSpeechBox->setTextColor(&guiPalette->getColor(Palette::TEXT));
    mSpeechBox->setTextWrapped(text, 130 > width ? 130 : width);
}

void SpeechBubble::adjustSize()
{
    int width = mCaption->getWidth() + 2 * getPadding();
    const int speechWidth = mSpeechBox->getMinWidth() + 2 * getPadding();

    const int fontHeight = getFont()->getHeight();
    const int nameHeight = getCaption() != "" ? mCaption->getHeight() + 
                           (getPadding() / 2) : 0;
    const int numRows = mSpeechBox->getNumberOfRows();
    const int height = numRows * fontHeight + nameHeight + getPadding();

    if (width < speechWidth)
        width = speechWidth;

    width += 2 * getPadding();

    setContentSize(width, height);

    const int xPos = (getWidth() - width) / 2;
    const int yPos = (getHeight() - height) / 2 + nameHeight;

    mCaption->setPosition(xPos, getPadding());
    mSpeechBox->setPosition(xPos, yPos);
}

void SpeechBubble::scheduleDelete()
{
    viewport->scheduleDelete(this);
}

