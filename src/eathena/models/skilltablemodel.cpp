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

#include "skilltablemodel.h"

#include "../db/skilldb.h"

#include "../gui/skill.h"

#include "../../bindings/guichan/widgets/label.h"

#include "../../core/utils/stringutils.h"

SkillTableModel::SkillTableModel(SkillDialog *dialog) :
    StaticTableModel(0, 3)
{
    mEntriesNr = 0;
    mDialog = dialog;
    update();
}

void SkillTableModel::update(void)
{
    mEntriesNr = mDialog->getSkills().size();
    resize();

    for (int i = 0; i < mEntriesNr; i++)
    {
        SKILL *skill = mDialog->getSkills()[i];
        SkillInfo const *info;
        std::string tmp;

        info = SkillDB::get(skill->id);

        tmp = strprintf("%c%s", info->modifiable? ' ' : '*', info->name.c_str());
        gcn::Label *name_label = new Label(tmp);

        tmp = strprintf("Lv:%i", skill->lv);
        gcn::Label *lv_label = new Label(tmp);

        tmp = strprintf("Sp:%i", skill->sp);
        gcn::Label *sp_label = new Label(tmp);

        set(i, 0, name_label);
        set(i, 1, lv_label);
        set(i, 2, sp_label);
    }
}

