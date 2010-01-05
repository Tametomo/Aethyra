/*
 *  Aethyra
 *  Copyright (C) 2004-2005  The Mana World Development Team
 *  Copyright (C) 2008  Aethyra Development Team
 *
 *  This file is part of Aethyra derived from original code
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

#include <guichan/font.hpp>

#include "minimap.h"

#include "../beingmanager.h"

#include "../../bindings/guichan/graphics.h"
#include "../../bindings/guichan/palette.h"
#include "../../bindings/guichan/skin.h"

#include "../../core/configuration.h"
#include "../../core/resourcemanager.h"

#include "../../core/map/map.h"

#include "../../core/image/image.h"

#include "../../core/map/sprite/being.h"
#include "../../core/map/sprite/localplayer.h"
#include "../../core/map/sprite/player.h"

#include "../../core/utils/gettext.h"

bool Minimap::mShow = true;
int Minimap::mUserWidth = 100;
int Minimap::mUserHeight = 100;

Minimap::Minimap():
    Window(_("Map")),
    mMapImage(NULL),
    mWidthProportion(0.5),
    mHeightProportion(0.5)
{
    setWindowName("MiniMap");
    setResizable(true);
    setCloseButton(true);

    mShow = config.getValue(getWindowName() + "Show", true);
    mUserWidth = config.getValue(getWindowName() + "UserWidth", 100);
    mUserHeight = config.getValue(getWindowName() + "UserHeight", 100);

    setDefaultSize(100, 100, ImageRect::UPPER_LEFT, 5, 25);

    loadWindowState();
}

Minimap::~Minimap()
{
    if (mMapImage)
        mMapImage->decRef();

    config.setValue(getWindowName() + "Show", mShow);
    config.setValue(getWindowName() + "UserWidth", mUserWidth);
    config.setValue(getWindowName() + "UserHeight", mUserHeight);
}

void Minimap::fontChanged()
{
    Window::fontChanged();

    if (mMapImage)
    {
        const int paddingX = 2 * getPadding();
        const int paddingY = getTitleBarHeight() + getPadding();
        const int titleWidth = getFont()->getWidth(getCaption()) +
                               mSkin->getCloseImage()->getWidth() + 4 * 
                               getPadding();
        const int mapWidth = mMapImage->getWidth() < 100 ?
                             mMapImage->getWidth() + paddingX : 100;
        const int mapHeight = mMapImage->getHeight() < 100 ?
                              mMapImage->getHeight() + paddingY : 100;

        setMinWidth(mapWidth > titleWidth ? mapWidth : titleWidth);
        setMinHeight(mapHeight);

        setMaxWidth(mMapImage->getWidth() > titleWidth ?
                    mMapImage->getWidth() + paddingX : titleWidth);
        setMaxHeight(mMapImage->getHeight() + paddingY);

        setContentSize(mUserWidth - paddingX, mUserHeight - paddingY);
        saveRelativeLocation(getX(), getY());

        setVisible(mShow);
    }
}

void Minimap::setMap(Map *map)
{
    // Remove the old image if there is one.
    if (mMapImage)
    {
        mMapImage->decRef();
        mMapImage = NULL;
    }

    // Set the title for the Minimap
    if (map)
    {
        ResourceManager *resman = ResourceManager::getInstance();
        mMapImage = resman->getImage(map->getProperty("minimap"));

        if (mMapImage)
        {
            const std::string caption = map->getName();

            mWidthProportion = (float) mMapImage->getWidth() / map->getWidth();
            mHeightProportion = (float) mMapImage->getHeight() / map->getHeight();
            setCaption(caption.empty() ? _("Map") : caption);

            fontChanged();
        }
    }

    if (!mMapImage)
        setVisible(false);
}

void Minimap::toggle()
{
    mShow = !mShow;
    setVisible(mShow && mMapImage);
}

void Minimap::close()
{
    mShow = false;
    Window::close();
}

void Minimap::hide()
{
    mShow = mOldVisibility;
    Window::hide();
}

void Minimap::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);

    if (!mMapImage)
        return;

    const gcn::Rectangle a = getChildrenArea();

    graphics->pushClipArea(a);

    int mapOriginX = 0;
    int mapOriginY = 0;

    if (mMapImage->getWidth() > a.width || mMapImage->getHeight() > a.height)
    {
        mapOriginX = (int) (((a.width) / 2) - (player_node->mX *
                      mWidthProportion));
        mapOriginY = (int) (((a.height) / 2) - (player_node->mY *
                      mHeightProportion));

        const int minOriginX = a.width - mMapImage->getWidth();
        const int minOriginY = a.height - mMapImage->getHeight();

        if (mapOriginX < minOriginX)
            mapOriginX = minOriginX;
        if (mapOriginY < minOriginY)
            mapOriginY = minOriginY;
        if (mapOriginX > 0)
            mapOriginX = 0;
        if (mapOriginY > 0)
            mapOriginY = 0;
    }

    static_cast<Graphics*>(graphics)->drawImage(mMapImage, mapOriginX,
                                                mapOriginY);

    const Beings &beings = beingManager->getAll();

    for (Beings::const_iterator bi = beings.begin(), bi_end = beings.end();
         bi != bi_end; ++bi)
    {
        const Being *being = (*bi);
        const int x = (int) (being->mX * mWidthProportion) + mapOriginX;
        const int y = (int) (being->mY * mHeightProportion) + mapOriginY;

        if ((x > getWidth() || x < 0) || (y > getHeight() || y < 0))
            continue;

        int dotSize = 2;

        switch (being->getType())
        {
            case Being::PLAYER:
                {
                    Palette::ColorType type = Palette::PC;

                    if (being == player_node)
                    {
                        type = Palette::SELF;
                        dotSize = 3;
                    }

                    if (static_cast<const Player*>(being)->isGM())
                        type = Palette::GM_NAME;

                    graphics->setColor(guiPalette->getColor(type));
                    break;
                }

            case Being::MONSTER:
                graphics->setColor(guiPalette->getColor(Palette::MONSTER));
                break;

            case Being::NPC:
                graphics->setColor(guiPalette->getColor(Palette::NPC));
                break;

            default:
                continue;
        }

        const int offsetHeight = (int) ((dotSize - 1) * mHeightProportion);
        const int offsetWidth = (int) ((dotSize - 1) * mWidthProportion);

        graphics->fillRectangle(gcn::Rectangle(x - offsetWidth, y - offsetHeight,
                                               dotSize, dotSize));
    }

    graphics->popClipArea();
}

void Minimap::mouseReleased(gcn::MouseEvent &event)
{
    mUserWidth = getWidth();
    mUserHeight = getHeight();
    Window::mouseReleased(event);
}
