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

#include <guichan/widgets/label.hpp>

#include "tab.h"
#include "tabbedarea.h"

#include "../graphics.h"
#include "../palette.h"

#include "../../../core/configlistener.h"
#include "../../../core/configuration.h"
#include "../../../core/resourcemanager.h"

#include "../../../core/image/image.h"

#include "../../../core/utils/dtor.h"

int Tab::mInstances = 0;

float Tab::mAlpha = 1.0;
TabConfigListener *Tab::mConfigListener = NULL;

enum {
    TAB_STANDARD,    // 0
    TAB_HIGHLIGHTED, // 1
    TAB_SELECTED,     // 2
    TAB_UNUSED,       // 3
    TAB_COUNT        // 4 - Must be last.
};

struct TabData
{
    char const *file;
    int gridX;
    int gridY;
};

static TabData const data[TAB_COUNT] = {
    { "graphics/gui/tab.png", 0, 0 },
    { "graphics/gui/tab.png", 9, 4 },
    { "graphics/gui/tabselected.png", 16, 19 },
    { "graphics/gui/tab.png", 25, 23 }
};

ImageRect Tab::tabImg[TAB_COUNT];
Image* Tab::mCloseButton = NULL;

class TabConfigListener : public ConfigListener
{
    public:
        TabConfigListener(Tab *rg):
            mTab(rg)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "guialpha")
            {
                mTab->mAlpha = config.getValue("guialpha", 0.8);

                for (int a = 0; a < 9; a++)
                {
                    mTab->tabImg[TAB_SELECTED].grid[a]->setAlpha(mTab->mAlpha);
                    mTab->tabImg[TAB_STANDARD].grid[a]->setAlpha(mTab->mAlpha);
                }
            }
        }
    private:
        Tab *mTab;
};

Tab::Tab(const std::string &caption, bool closeable) :
    gcn::BasicContainer(),
    mTabColor(&guiPalette->getColor(Palette::TEXT)),
    mCloseable(closeable),
    mHighlighted(false),
    mHasMouse(false),
    mTabbedArea(NULL)
{
    setFocusable(false);
    setFrameSize(0);
    mHighlighted = false;
    mLabel = new Label(caption);
    mLabel->setPosition(4, 4);
    add(mLabel);
    addMouseListener(this);

    if (mInstances == 0)
    {
        mAlpha = config.getValue("guialpha", 0.8);

        // Load the skin
        ResourceManager *resman = ResourceManager::getInstance();
        Image *tab[TAB_COUNT];

        int a, x, y, mode;

        for (mode = 0; mode < TAB_COUNT; mode++)
        {
            tab[mode] = resman->getImage(data[mode].file);
            a = 0;
            for (y = 0; y < 3; y++)
            {
                for (x = 0; x < 3; x++)
                {
                    tabImg[mode].grid[a] = tab[mode]->getSubImage(
                            data[x].gridX, data[y].gridY,
                            data[x + 1].gridX - data[x].gridX + 1,
                            data[y + 1].gridY - data[y].gridY + 1);
                    tabImg[mode].grid[a]->setAlpha(mAlpha);
                    a++;
                }
            }
            tab[mode]->decRef();
        }
        mCloseButton = resman->getImage("graphics/gui/close_button.png");

        mConfigListener = new TabConfigListener(this);
        config.addListener("guialpha", mConfigListener);
    }
    adjustSize();
    mInstances++;
}

Tab::~Tab()
{
    if (mTabbedArea)
        mTabbedArea->removeTab(this);

    remove(mLabel);
    destroy(mLabel);

    mInstances--;

    if (mInstances == 0)
    {
        config.removeListener("guialpha", mConfigListener);
        destroy(mConfigListener);

        for (int mode = 0; mode < TAB_COUNT; mode++)
            for_each(tabImg[mode].grid, tabImg[mode].grid + 9, dtor<Image*>());

        mCloseButton->decRef();
    }
}

void Tab::adjustSize()
{
    int width = mLabel->getWidth() + 8;
    int height = mLabel->getHeight() + 8;

    if (mCloseable && mCloseButton)
    {
        width = width + mCloseButton->getWidth() - 4;
        height = height > mCloseButton->getHeight() ? height :
                 mCloseButton->getHeight();
    }

    setSize(width, height);

    if (mTabbedArea != NULL)
        mTabbedArea->adjustTabPositions();
}

void Tab::draw(gcn::Graphics *graphics)
{
    int mode = TAB_STANDARD;

    // check which type of tab to draw
    if (mTabbedArea)
    {
        if (mTabbedArea->isTabSelected(this))
        {
            mode = TAB_SELECTED;
            // if tab is selected, it doesnt need to highlight activity
            mLabel->setForegroundColor(*mTabColor);
            mHighlighted = false;
        }
        else if (mHighlighted)
        {
            mode = TAB_HIGHLIGHTED;
            mLabel->setForegroundColor(guiPalette->getColor(Palette::TAB_HIGHLIGHT));
        }
        else
            mLabel->setForegroundColor(*mTabColor);
    }

    // draw tab
    static_cast<Graphics*>(graphics)->drawImageRect(0, 0, getWidth(),
                                                    getHeight(), tabImg[mode]);

    if (mTabbedArea->isFocused() && mode == TAB_SELECTED)
    {
        graphics->setColor(guiPalette->getColor(Palette::HIGHLIGHT,
                                               (int) (mAlpha * 255.0f)));
        graphics->fillRectangle(gcn::Rectangle(4, 4, getWidth() - 8,
                                               getHeight() - 8));
    }

    // draw close button
    if (isCloseable())
    {
        Graphics *g = static_cast<Graphics*>(graphics);
        g->drawImage(mCloseButton, getWidth() - mCloseButton->getWidth() - 2, 2);
    }

    // draw label
    drawChildren(graphics);
}

void Tab::setCaption(const std::string& caption)
{
    mLabel->setCaption(caption);
    mLabel->adjustSize();
    adjustSize();
}

void Tab::setCloseable(bool close)
{
    mCloseable = close;
    adjustSize();
}

void Tab::fontChanged()
{
    gcn::BasicContainer::fontChanged();

    mLabel->adjustSize();
    adjustSize();
}

void Tab::close()
{
    if (mTabbedArea)
        mTabbedArea->removeTab(this);
}

void Tab::mousePressed(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        const int x = event.getX();
        const int y = event.getY();

        // Handle close button
        if (mCloseable)
        {
            gcn::Rectangle closeButtonRect(getWidth() - mCloseButton->getWidth() - 2,
                2, mCloseButton->getWidth(), mCloseButton->getHeight());

            if (closeButtonRect.isPointInRect(x, y))
                close();
        }
    }
}
