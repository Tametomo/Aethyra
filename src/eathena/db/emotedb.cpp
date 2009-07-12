/*
 *  Emote database
 *  Copyright 2009 Aethyra Development Team
 *
 *  This file is part of Aethyra.
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
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "emotedb.h"

#include "../../core/log.h"

#include "../../core/image/sprite/animatedsprite.h"

#include "../../core/utils/xml.h"

namespace
{
    EmoteInfos mEmoteInfos;
    bool mLoaded = false;
    int mLastEmote = 0;
}

void EmoteDB::load()
{
    if (mLoaded)
        return;

    mLastEmote = 0;

    logger->log("Initializing emote database...");

    const XML::Document doc("emotes.xml");
    const xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "emotes"))
    {
        logger->log("Emote Database: Error while loading emotes.xml!");
        return;
    }

    //iterate <emote>s
    for_each_xml_child_node(emoteNode, rootNode)
    {
        if (!xmlStrEqual(emoteNode->name, BAD_CAST "emote"))
            continue;

        const int id = XML::getProperty(emoteNode, "id", -1);

        if (id == -1)
        {
            logger->log("Emote Database: Emote with missing ID in emotes.xml!");
            continue;
        }

        EmoteInfo *currentInfo = new EmoteInfo();

        for_each_xml_child_node(spriteNode, emoteNode)
        {
            if (xmlStrEqual(spriteNode->name, BAD_CAST "sprite"))
            {
                EmoteSprite *currentSprite = new EmoteSprite();
                std::string file = "graphics/sprites/" + (std::string)
                            (const char*) spriteNode->xmlChildrenNode->content;
                currentSprite->sprite = AnimatedSprite::load(file,
                                XML::getProperty(spriteNode, "variant", 0));
                currentInfo->sprites.push_back(currentSprite);
            }
            else if (xmlStrEqual(spriteNode->name, BAD_CAST "particlefx"))
            {
                std::string particlefx = (const char*) spriteNode->xmlChildrenNode->content;
                currentInfo->particles.push_back(particlefx);
            }
        }
        mEmoteInfos[id] = currentInfo;

        if (id > mLastEmote)
            mLastEmote = id;
    }

    mLoaded = true;
}

void EmoteDB::unload()
{
    for (EmoteInfosIterator i = mEmoteInfos.begin(); i != mEmoteInfos.end(); i++)
    {
        while (!i->second->sprites.empty())
        {
            delete i->second->sprites.front()->sprite;
            delete i->second->sprites.front();
            i->second->sprites.pop_front();
        }
        delete i->second;
    }

    mEmoteInfos.clear();

    mLoaded = false;
}

const EmoteInfo *EmoteDB::get(const int &id)
{
    EmoteInfosIterator i = mEmoteInfos.find(id);

    if (i == mEmoteInfos.end())
    {
        logger->log("EmoteDB: Warning, unknown emote ID %d requested", id);
        return NULL;
    }
    else
        return i->second;
}

const AnimatedSprite* EmoteDB::getAnimation(const int &id)
{
    const EmoteInfo *info = get(id);
    return info->sprites.front()->sprite;
}

const int& EmoteDB::getLast()
{
    return mLastEmote;
}
