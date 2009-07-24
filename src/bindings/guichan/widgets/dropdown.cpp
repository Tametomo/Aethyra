/*
 *  Aethyra
 *  Copyright (C) 2008  The Mana World Development Team
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

#include <algorithm>

#include <guichan/focushandler.hpp>

#include "dropdown.h"
#include "listbox.h"
#include "scrollarea.h"

#include "../graphics.h"
#include "../palette.h"
#include "../protectedfocuslistener.h"

#include "../sdl/sdlinput.h"

#include "../../../core/configlistener.h"
#include "../../../core/configuration.h"
#include "../../../core/resourcemanager.h"

#include "../../../core/image/image.h"

#include "../../../core/utils/dtor.h"

int DropDown::instances = 0;
Image *DropDown::buttons[2][2];
ImageRect DropDown::skin;
float DropDown::mAlpha = 1.0;
DropDownConfigListener *DropDown::mConfigListener = NULL;

class DropDownConfigListener : public ConfigListener
{
    public:
        DropDownConfigListener(DropDown *dd):
            mDropDown(dd)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "guialpha")
            {
                mDropDown->mAlpha = config.getValue("guialpha", 0.8);

                mDropDown->buttons[0][0]->setAlpha(mDropDown->mAlpha);
                mDropDown->buttons[0][1]->setAlpha(mDropDown->mAlpha);
                mDropDown->buttons[1][0]->setAlpha(mDropDown->mAlpha);
                mDropDown->buttons[1][1]->setAlpha(mDropDown->mAlpha);

                for (int a = 0; a < 9; a++)
                    mDropDown->skin.grid[a]->setAlpha(mDropDown->mAlpha);
            }
        }
    private:
        DropDown *mDropDown;
};

DropDown::DropDown(gcn::ListModel *listModel):
    gcn::DropDown::DropDown(listModel, new ScrollArea, new ListBox(listModel))
{
    setFrameSize(2);

    // Initialize graphics
    if (instances == 0)
    {
        mAlpha = config.getValue("guialpha", 0.8);

        // Load the background skin
        ResourceManager *resman = ResourceManager::getInstance();

        // Get the button skin
        buttons[1][0] =
            resman->getImage("graphics/gui/vscroll_up_default.png");
        buttons[0][0] =
            resman->getImage("graphics/gui/vscroll_down_default.png");
        buttons[1][1] =
            resman->getImage("graphics/gui/vscroll_up_pressed.png");
        buttons[0][1] =
            resman->getImage("graphics/gui/vscroll_down_pressed.png");

        buttons[0][0]->setAlpha(mAlpha);
        buttons[0][1]->setAlpha(mAlpha);
        buttons[1][0]->setAlpha(mAlpha);
        buttons[1][1]->setAlpha(mAlpha);

        // get the border skin
        Image *boxBorder = resman->getImage("graphics/gui/deepbox.png");
        int gridx[4] = {0, 3, 28, 31};
        int gridy[4] = {0, 3, 28, 31};
        int a = 0, x, y;

        for (y = 0; y < 3; y++)
        {
            for (x = 0; x < 3; x++)
            {
                skin.grid[a] = boxBorder->getSubImage(gridx[x], gridy[y],
                                                      gridx[x + 1] -
                                                      gridx[x] + 1,
                                                      gridy[y + 1] -
                                                      gridy[y] + 1);
                skin.grid[a]->setAlpha(mAlpha);
                a++;
            }
        }

        boxBorder->decRef();

        mConfigListener = new DropDownConfigListener(this);
        config.addListener("guialpha", mConfigListener);
    }

    mProtFocusListener = new ProtectedFocusListener();

    addFocusListener(mProtFocusListener);

    mProtFocusListener->blockKey(SDLK_LEFT);
    mProtFocusListener->blockKey(SDLK_RIGHT);
    mProtFocusListener->blockKey(SDLK_UP);
    mProtFocusListener->blockKey(SDLK_DOWN);
    mProtFocusListener->blockKey(SDLK_SPACE);
    mProtFocusListener->blockKey(SDLK_RETURN);
    mProtFocusListener->blockKey(SDLK_HOME);
    mProtFocusListener->blockKey(SDLK_END);

    instances++;
}

DropDown::~DropDown()
{
    instances--;
    // Free images memory
    if (instances == 0)
    {
        config.removeListener("guialpha", mConfigListener);
        delete mConfigListener;

        buttons[0][0]->decRef();
        buttons[0][1]->decRef();
        buttons[1][0]->decRef();
        buttons[1][1]->decRef();

        for_each(skin.grid, skin.grid + 9, dtor<Image*>());
    }

    delete mScrollArea;

    if (mFocusHandler && mFocusHandler->isFocused(this))
        mFocusHandler->focusNone();

    removeFocusListener(mProtFocusListener);
    delete mProtFocusListener;
}

void DropDown::draw(gcn::Graphics* graphics)
{
    int h;

    if (mDroppedDown)
        h = mFoldedUpHeight;
    else
        h = getHeight();

    const int alpha = (int) (mAlpha * 255.0f);
    gcn::Color faceColor = getBaseColor();
    faceColor.a = alpha;
    const gcn::Color* highlightColor = &guiPalette->getColor(Palette::HIGHLIGHT,
            alpha);
    gcn::Color shadowColor = faceColor - 0x303030;
    shadowColor.a = alpha;

    if (mListBox->getListModel() && mListBox->getSelected() >= 0)
    {
        graphics->setFont(getFont());
        graphics->setColor(guiPalette->getColor(Palette::TEXT, 255));
        graphics->drawText(mListBox->getListModel()->getElementAt(mListBox->getSelected()), 1, 0);
    }

    if (isFocused())
    {
        graphics->setColor(*highlightColor);
        graphics->drawRectangle(gcn::Rectangle(0, 0, getWidth() - h, h));
    }

    drawButton(graphics);

    if (mDroppedDown)
    {
        drawChildren(graphics);

        // Draw two lines separating the ListBox with selected
        // element view.
        graphics->setColor(*highlightColor);
        graphics->drawLine(0, h, getWidth(), h);
        graphics->setColor(shadowColor);
        graphics->drawLine(0, h + 1, getWidth(), h + 1);
    }
}

void DropDown::drawFrame(gcn::Graphics *graphics)
{
    const int bs = getFrameSize();
    const int w = getWidth() + bs * 2;
    const int h = getHeight() + bs * 2;

    static_cast<Graphics*>(graphics)->drawImageRect(0, 0, w, h, skin);
}

void DropDown::drawButton(gcn::Graphics *graphics)
{
    int height = mDroppedDown ? mFoldedUpHeight : getHeight();

    static_cast<Graphics*>(graphics)->
        drawImage(buttons[mDroppedDown][mPushed], getWidth() - height + 2, 1);
}

// -- KeyListener notifications
void DropDown::keyPressed(gcn::KeyEvent& keyEvent)
{
    if (keyEvent.isConsumed())
        return;

    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ENTER || key.getValue() == Key::SPACE)
        dropDown();
    else if (key.getValue() == Key::UP)
        setSelected(getSelected() - 1);
    else if (key.getValue() == Key::DOWN)
        setSelected(getSelected() + 1);
    else if (key.getValue() == Key::HOME)
        setSelected(0);
    else if (key.getValue() == Key::END)
        setSelected(mListBox->getListModel()->getNumberOfElements() - 1);
    else
        return;

    keyEvent.consume();
}

void DropDown::focusLost(const gcn::Event& event)
{
    gcn::DropDown::focusLost(event);
    releaseModalMouseInputFocus();
}

void DropDown::mousePressed(gcn::MouseEvent& mouseEvent)
{
    gcn::DropDown::mousePressed(mouseEvent);

    if (0 <= mouseEvent.getY() && mouseEvent.getY() < getHeight() &&
        mouseEvent.getX() >= 0 && mouseEvent.getX() < getWidth() &&
        mouseEvent.getButton() == gcn::MouseEvent::LEFT && mDroppedDown &&
        mouseEvent.getSource() == mListBox)
    {
        mPushed = false;
        foldUp();
        releaseModalMouseInputFocus();
        distributeActionEvent();
    }
}

void DropDown::mouseWheelMovedUp(gcn::MouseEvent& mouseEvent)
{
    if (!mDroppedDown)
    {
        setSelected(getSelected() - 1);
        mouseEvent.consume();
    }
}

void DropDown::mouseWheelMovedDown(gcn::MouseEvent& mouseEvent)
{
    if (!mDroppedDown)
    {
        setSelected(getSelected() + 1);
        mouseEvent.consume();
    }
}
