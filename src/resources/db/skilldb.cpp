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
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "skilldb.h"

#include "../../log.h"

#include "../../utils/xml.h"

#define SKILLS_FILE "skills.xml"

static SkillInfo fakeSkillInfo = {"???", false };
static bool mLoaded = false;

void SkillDB::load()
{
    if (mLoaded)
        return;

    const SkillInfo emptySkillInfo = { "", false };

    const XML::Document doc(SKILLS_FILE);
    const xmlNodePtr root = doc.rootNode();

    if (!root || !xmlStrEqual(root->name, BAD_CAST "skills"))
    {
        logger->log("Error loading skills file: %s", SKILLS_FILE);
        skill_db.resize(2, emptySkillInfo);
        skill_db[1].name = "Basic";
        skill_db[1].modifiable = true;
        return;
    }

    for_each_xml_child_node(node, root)
    {
        if (xmlStrEqual(node->name, BAD_CAST "skill"))
        {
            const int index = atoi(XML::getProperty(node, "id", "-1").c_str());
            const std::string name = XML::getProperty(node, "name", "");
            const bool modifiable = !atoi(XML::getProperty(node, "fixed", "0").c_str());

            if (index >= 0)
            {
                skill_db.resize(index + 1, emptySkillInfo);
                skill_db[index].name = name;
                skill_db[index].modifiable = modifiable;
            }
        }
    }

    mLoaded = true;
}

void SkillDB::unload()
{
    logger->log("Unloading skill database...");

    skill_db.clear();
    mLoaded = false;
}

const SkillInfo* SkillDB::get(const int &id)
{
    return (id >= 0 && id < size() ? &skill_db[id] : &fakeSkillInfo);
}

bool SkillDB::modifiable(const int &id)
{
    return (id >= 0 && id < size() ? &skill_db[id].modifiable : false);
}

int SkillDB::size()
{
    return skill_db.size();
}
