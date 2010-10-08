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

#include <cassert>
#include <iostream>
#include <zlib.h>

#include "map.h"
#include "mapreader.h"
#include "tileset.h"

#include "../log.h"
#include "../resourcemanager.h"

#include "../image/animation.h"
#include "../image/image.h"

#include "../utils/base64.h"
#include "../utils/dtor.h"
#include "../utils/stringutils.h"
#include "../utils/xml.h"

#include "../../bindings/zlib/memorytools.h"

const unsigned int DEFAULT_TILE_WIDTH = 32;
const unsigned int DEFAULT_TILE_HEIGHT = 32;

Map *MapReader::readMap(const std::string &filename)
{
    logger->log("Attempting to read map %s", filename.c_str());
    // Load the file through resource manager
    ResourceManager *resman = ResourceManager::getInstance();
    int fileSize;
    void *buffer = resman->loadFile(filename, fileSize);
    Map *map = NULL;

    if (buffer == NULL)
    {
        logger->log("Map file not found (%s)", filename.c_str());
        return NULL;
    }

    unsigned char *inflated;
    unsigned int inflatedSize;

    if (filename.find(".gz", filename.length() - 3) != std::string::npos)
    {
        // Inflate the gzipped map data
        inflatedSize = inflateMemory((unsigned char*) buffer, fileSize,
                                     inflated);
        free(buffer);

        if (inflated == NULL)
        {
            logger->log("Could not decompress map file (%s)",
                    filename.c_str());
            return NULL;
        }
    }
    else
    {
        inflated = (unsigned char*) buffer;
        inflatedSize = fileSize;
    }

    XML::Document doc((char*) inflated, inflatedSize);
    free(inflated);

    xmlNodePtr node = doc.rootNode();

    // Parse the inflated map data
    if (node)
    {
        if (!xmlStrEqual(node->name, BAD_CAST "map"))
            logger->log("Error: Not a map file (%s)!", filename.c_str());
        else
            map = readMap(node, filename);
    }
    else
        logger->log("Error while parsing map file (%s)!", filename.c_str());

    if (map)
        map->setProperty("_filename", filename);

    return map;
}

Map *MapReader::readMap(const xmlNodePtr &node, const std::string &path)
{
    // Take the filename off the path
    const std::string pathDir = path.substr(0, path.rfind("/") + 1);

    const int w = XML::getProperty(node, "width", 0);
    const int h = XML::getProperty(node, "height", 0);
    const int tilew = XML::getProperty(node, "tilewidth", DEFAULT_TILE_WIDTH);
    const int tileh = XML::getProperty(node, "tileheight", DEFAULT_TILE_HEIGHT);
    Map *map = new Map(w, h, tilew, tileh);

    for_each_xml_child_node(childNode, node)
    {
        if (xmlStrEqual(childNode->name, BAD_CAST "tileset"))
        {
            Tileset *tileset = readTileset(childNode, pathDir, map);

            if (tileset)
                map->addTileset(tileset);
        }
        else if (xmlStrEqual(childNode->name, BAD_CAST "layer"))
            readLayer(childNode, map);
        else if (xmlStrEqual(childNode->name, BAD_CAST "properties"))
            readProperties(childNode, map);
        else if (xmlStrEqual(childNode->name, BAD_CAST "objectgroup"))
        {
            // The object group offset is applied to each object individually
            const int tileOffsetX = XML::getProperty(childNode, "x", 0);
            const int tileOffsetY = XML::getProperty(childNode, "y", 0);
            const int offsetX = tileOffsetX * tilew;
            const int offsetY = tileOffsetY * tileh;

            for_each_xml_child_node(objectNode, childNode)
            {
                if (xmlStrEqual(objectNode->name, BAD_CAST "object"))
                {
                    const std::string objType =
                        XML::getProperty(objectNode, "type", "");

                    if (objType == "WARP" || objType == "NPC" ||
                        objType == "SCRIPT" || objType == "SPAWN")
                    {
                        // Silently skip server-side objects.
                        continue;
                    }

                    const std::string objName =
                        XML::getProperty(objectNode, "name", "");
                    const int objX = XML::getProperty(objectNode, "x", 0);
                    const int objY = XML::getProperty(objectNode, "y", 0);

                    logger->log("- Loading object name: %s type: %s at %d:%d",
                                objName.c_str(), objType.c_str(),
                                objX, objY);

                    if (objType == "PARTICLE_EFFECT")
                    {
                        if (objName.empty())
                        {
                            logger->log("   Warning: No particle file given");
                            continue;
                        }

                        map->addParticleEffect(objName,
                                               objX + offsetX,
                                               objY + offsetY);
                    }
                    else
                        logger->log("   Warning: Unknown object type");
                }
            }
        }
    }

    map->initializeAmbientLayers();

    return map;
}

void MapReader::readProperties(const xmlNodePtr &node, Properties *props)
{
    for_each_xml_child_node(childNode, node)
    {
        if (!xmlStrEqual(childNode->name, BAD_CAST "property"))
            continue;

        // Example: <property name="name" value="value"/>
        const std::string name = XML::getProperty(childNode, "name", "");
        const std::string value = XML::getProperty(childNode, "value", "");

        if (!name.empty() && !value.empty())
            props->setProperty(name, value);
    }
}

static void setTile(Map *map, MapLayer *layer, const int x,
                    const int y, const int gid)
{
    const Tileset* set = map->getTilesetWithGid(gid);
    if (layer)
    {
        // Set regular tile on a layer
        Image* img = set ? set->get(gid - set->getFirstGid()) : 0;
        layer->setTile(x, y, img);
    }
    else
    {
        // Set collision tile
        map->setWalk(x, y, (!set || (gid - set->getFirstGid() == 0)));
    }
}

void MapReader::readLayer(const xmlNodePtr &node, Map *map)
{
    // Layers are not necessarily the same size as the map
    const int w = XML::getProperty(node, "width", map->getWidth());
    const int h = XML::getProperty(node, "height", map->getHeight());
    const int tw = XML::getProperty(node, "tilewidth", map->getTileWidth());
    const int th = XML::getProperty(node, "tileheight", map->getTileHeight());
    const int offsetX = XML::getProperty(node, "x", 0);
    const int offsetY = XML::getProperty(node, "y", 0);
    std::string name = XML::getProperty(node, "name", "");
    name = toLower(name);

    const bool isFringeLayer = (name.substr(0,6) == "fringe");
    const bool isCollisionLayer = (name.substr(0,9) == "collision");
    const bool isVisible = XML::getProperty(node, "visible", 1);

    MapLayer *layer = NULL;

    if (!isCollisionLayer)
    {
        layer = new MapLayer(offsetX, offsetY, w, h, tw, th, isFringeLayer,
                             isVisible);
        map->addLayer(layer);
    }

    logger->log("- Loading layer \"%s\"", name.c_str());
    int x = 0;
    int y = 0;

    // Load the tile data
    for_each_xml_child_node(childNode, node)
    {
        if (!xmlStrEqual(childNode->name, BAD_CAST "data"))
            continue;

        const std::string encoding =
            XML::getProperty(childNode, "encoding", "");
        const std::string compression =
            XML::getProperty(childNode, "compression", "");

        if (encoding == "base64")
        {
            if (!compression.empty() && compression != "gzip")
            {
                logger->log("Warning: only gzip layer compression supported!");
                return;
            }

            // Read base64 encoded map file
            xmlNodePtr dataChild = childNode->xmlChildrenNode;
            if (!dataChild)
                continue;
 
            const int len = strlen((const char*)dataChild->content) + 1;
            unsigned char *charData = new unsigned char[len + 1];
            const char *charStart = (const char*)dataChild->content;
            unsigned char *charIndex = charData;
 
            while (*charStart)
            {
                if (*charStart != ' ' && *charStart != '\t' &&
                    *charStart != '\n')
                {
                    *charIndex = *charStart;
                    charIndex++;
                }
                charStart++;
            }
            *charIndex = '\0';

            int binLen;
            unsigned char *binData =
                php3_base64_decode(charData, strlen((char*) charData), &binLen);

            delete[] charData;

            if (binData)
            {
                if (compression == "gzip")
                {
                    // Inflate the gzipped layer data
                    unsigned char *inflated;
                    unsigned int inflatedSize = inflateMemory(binData, binLen,
                                                              inflated);

                    free(binData);
                    binData = inflated;
                    binLen = inflatedSize;

                    if (!inflated)
                    {
                        logger->log("Error: Could not decompress layer!");
                        return;
                    }
                }

                for (int i = 0; i < binLen - 3; i += 4)
                 {
                    const int gid = binData[i] |
                                    binData[i + 1] << 8 |
                                    binData[i + 2] << 16 |
                                    binData[i + 3] << 24;

                    setTile(map, layer, x, y, gid);

                    TileAnimation* ani = map->getAnimationForGid(gid);
                    if (ani)
                        ani->addAffectedTile(layer, x + y * w);

                    x++;
                    if (x == w)
                    {
                        x = 0; y++;

                        // When we're done, don't crash on too much data
                        if (y == h)
                            break;
                    }
                }
                free(binData);
            }
        }
        else
        {
             // Read plain XML map file
            for_each_xml_child_node(childNode2, childNode)
            {
                if (!xmlStrEqual(childNode2->name, BAD_CAST "tile"))
                    continue;

                const int gid = XML::getProperty(childNode2, "gid", -1);
                setTile(map, layer, x, y, gid);

                x++;
                if (x == w)
                {
                    x = 0; y++;
                    if (y >= h)
                        break;
                }
            }
        }
 
        if (y < h || x)
            std::cerr << "TOO SMALL!\n";
 
        // There can be only one data element
        break;
    }
}

Tileset *MapReader::readTileset(xmlNodePtr node, const std::string &path,
                                Map *map)
{
    int firstGid = XML::getProperty(node, "firstgid", 0);
    XML::Document* doc = NULL;
    Tileset *set = NULL;

    if (xmlHasProp(node, BAD_CAST "source"))
    {
        std::string filename = XML::getProperty(node, "source", "");
        while (filename.substr(0, 3) == "../")
               filename.erase(0, 3);  // Remove "../"
        doc = new XML::Document(filename);
        node = doc->rootNode();
        firstGid += XML::getProperty(node, "firstgid", 0);
    }

    const int tw = XML::getProperty(node, "tilewidth", map->getTileWidth());
    const int th = XML::getProperty(node, "tileheight", map->getTileHeight());

    for_each_xml_child_node(childNode, node)
    {
        if (xmlStrEqual(childNode->name, BAD_CAST "image"))
        {
            const std::string source = XML::getProperty(childNode, "source", "");

            if (!source.empty())
            {
                std::string sourceStr = source;
                sourceStr.erase(0, 3);  // Remove "../"

                ResourceManager *resman = ResourceManager::getInstance();
                Image* tilebmp = resman->getImage(sourceStr);

                if (tilebmp)
                {
                    set = new Tileset(tilebmp, tw, th, firstGid);
                    tilebmp->decRef();
                }
                else
                    logger->log("Warning: Failed to load tileset (%s)",
                                source.c_str());
            }
        }
        else if (xmlStrEqual(childNode->name, BAD_CAST "tile"))
        {
            for_each_xml_child_node(tileNode, childNode)
            {
                if (!xmlStrEqual(tileNode->name, BAD_CAST "properties")) continue;

                int tileGID = firstGid + XML::getProperty(childNode, "id", 0);

                // read tile properties to a map for simpler handling
                std::map<std::string, int> tileProperties;
                for_each_xml_child_node(propertyNode, tileNode)
                {
                    if (!xmlStrEqual(propertyNode->name, BAD_CAST "property"))
                        continue;

                    std::string name = XML::getProperty(propertyNode, "name", "");
                    int value = XML::getProperty(propertyNode, "value", 0);
                    tileProperties[name] = value;
                    logger->log("Tile Prop of %d \"%s\" = \"%d\"", tileGID, name.c_str(), value);
                }

                // create animation
                if (!set)
                    continue;

                Animation *ani = new Animation();
                for (int i = 0; ;i++)
                {
                    std::map<std::string, int>::iterator iFrame, iDelay;
                    iFrame = tileProperties.find("animation-frame" + toString(i));
                    iDelay = tileProperties.find("animation-delay" + toString(i));

                    if (iFrame != tileProperties.end() && iDelay != tileProperties.end())
                        ani->addFrame(set->get(iFrame->second), iDelay->second, 0, 0);
                    else
                        break;
                }

                if (ani->getLength() > 0)
                {
                    map->addAnimation(tileGID, new TileAnimation(ani));
                    logger->log("Animation length: %d", ani->getLength());
                }
                else
                    destroy(ani);
            }
        }
    }

    destroy(doc);

    return set;
}
