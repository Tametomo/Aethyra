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

#ifndef SKILL_MANAGER_H
#define SKILL_MANAGER_H

#include <string>
#include <vector>

struct SkillInfo
{
    std::string name;
    bool modifiable;
};

/**
 * The class that holds the skill information.
 */
namespace SkillDB
{
    /**
     * Loads the skill data from <code>skills.xml</code>.
     */
    void load();

    /**
     * Clear the skill data
     */
    void unload();

    SkillInfo* get(int id);

    bool modifiable(int id);

    int size();

    // Skill DB
    static std::vector<SkillInfo> skill_db;
};

#endif
