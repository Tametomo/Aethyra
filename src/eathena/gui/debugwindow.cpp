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

#include "debugwindow.h"
#include "viewport.h"

#include "../../bindings/guichan/gui.h"
#include "../../bindings/guichan/layout.h"

#include "../../bindings/guichan/widgets/label.h"

#include "../../bindings/sdl/sound.h"

#include "../../core/image/particle/particle.h"

#include "../../core/map/map.h"

#include "../../core/utils/gettext.h"
#include "../../core/utils/stringutils.h"

DebugWindow::DebugWindow():
    Window(_("Debug"))
{
    setWindowName("Debug");
    saveVisibility(false);

    setResizable(true);
    setCloseButton(true);
    setDefaultSize(400, 100, ImageRect::CENTER);

    mFPSLabel = new Label(strprintf(_("%d FPS"), 0));
    mMusicFileLabel = new Label(strprintf(_("Music: %s"), ""));
    mMapLabel = new Label(strprintf(_("Map: %s"), ""));
    mMiniMapLabel = new Label(strprintf(_("Minimap: %s"), ""));
    mTileMouseLabel = new Label(strprintf(_("Cursor: (%d, %d)"), 0, 0));
    mParticleCountLabel = new Label(strprintf(_("Particle count: %d"), 0));

    fontChanged();
    loadWindowState();
}

void DebugWindow::fontChanged()
{
    Window::fontChanged();

    mTileMouseLabel->setCaption(strprintf(_("Cursor: (%d, %d)"), 999, 999));
    mParticleCountLabel = new Label(strprintf(_("Particle count: %d"), 99999));

    if (mWidgets.size() > 0)
        clear();

    place(0, 0, mFPSLabel, 3);
    place(3, 0, mTileMouseLabel);
    place(0, 1, mMusicFileLabel, 3);
    place(3, 1, mParticleCountLabel);
    place(0, 2, mMapLabel, 4);
    place(0, 3, mMiniMapLabel, 4);

    restoreFocus();
}

void DebugWindow::widgetShown(const gcn::Event& event)
{
    Window::widgetShown(event);

    if (!viewport)
    {
        mTileMouseLabel->setCaption("");
        mParticleCountLabel->setCaption("");
        mMapLabel->setCaption("");
        mMiniMapLabel->setCaption("");
    }
}

void DebugWindow::logic()
{
    if (!isVisible())
        return;

    mFPSLabel->setCaption(strprintf(_("%d FPS"), fps));
    mMusicFileLabel->setCaption(strprintf(_("Music: %s"),
                                          sound.getCurrentTrack().c_str()));

    if (!viewport)
        return;

    Map *currentMap = viewport->getMap();

    // Get the current mouse position
    const int mouseTileX = (gui->getMouseX() + viewport->getCameraX()) /
                            currentMap->getTileWidth();
    const int mouseTileY = (gui->getMouseY() + viewport->getCameraY()) /
                            currentMap->getTileHeight();

    mTileMouseLabel->setCaption(strprintf(_("Cursor: (%d, %d)"), mouseTileX,
                                            mouseTileY));

    if (currentMap)
    {
        mMiniMapLabel->setCaption(strprintf(_("Minimap: %s"),
                                            currentMap->getProperty("minimap").c_str()));
        mMapLabel->setCaption(strprintf(_("Map: %s"),
                                        viewport->getMapPath().c_str()));
    }

    mParticleCountLabel->setCaption(strprintf(_("Particle count: %d"),
                                                 Particle::particleCount));
}
