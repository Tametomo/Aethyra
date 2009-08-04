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

#include "beingmanager.h"
#include "flooritemmanager.h"
#include "game.h"
#include "maploader.h"

#include "gui/minimap.h"
#include "gui/viewport.h"

#include "../bindings/guichan/gui.h"

#include "../bindings/guichan/dialogs/okdialog.h"

#include "../bindings/sdl/keyboardconfig.h"
#include "../bindings/sdl/sound.h"

#include "../core/log.h"
#include "../core/resourcemanager.h"

#include "../core/image/particle/particle.h"

#include "../core/map/map.h"
#include "../core/map/mapreader.h"

#include "../core/map/sprite/localplayer.h"

#include "../core/utils/gettext.h"
#include "../core/utils/stringutils.h"

MapLoader::MapLoader():
    mCurrentMap(0)
{
}

MapLoader::~MapLoader()
{
    delete mCurrentMap;
}

bool MapLoader::changeMap(const std::string &mapPath)
{
    // Clean up floor items, beings and particles
    floorItemManager->clear();
    beingManager->clear();

    // Close the popup menu on map change so that invalid options can't be
    // executed.
    viewport->closePopupMenu();

    // Unset the map of the player so that its particles are cleared before
    // being deleted in the next step
    if (player_node)
        player_node->setMap(NULL);

    particleEngine->clear();

    mMapName = mapPath;

    // Store full map path in global var
    map_path = "maps/" + mapPath + ".tmx";
    ResourceManager *resman = ResourceManager::getInstance();
    if (!resman->exists(map_path))
        map_path += ".gz";

    // Attempt to load the new map
    Map *newMap = MapReader::readMap(map_path);

    if (!newMap)
    {
        logger->log("Error while loading %s", map_path.c_str());
        new OkDialog(_("Could not load map"),
                     strprintf(_("Error while loading %s"), map_path.c_str()));
    }

    // Notify the minimap and beingManager about the map change
    minimap->setMap(newMap);
    beingManager->setMap(newMap);
    particleEngine->setMap(newMap);
    viewport->setMap(newMap);
    keyboard.refreshActiveKeys();

    // Initialize map-based particle effects
    if (newMap)
        newMap->initializeParticleEffects(particleEngine);

    // Start playing new music file when necessary
    std::string oldMusic = mCurrentMap ? mCurrentMap->getMusicFile() : "";
    std::string newMusic = newMap ? newMap->getMusicFile() : "";

    if (newMusic != oldMusic)
        sound.playMusic(newMusic);

    if (mCurrentMap)
        delete mCurrentMap;

    mCurrentMap = newMap;

    return true;
}

