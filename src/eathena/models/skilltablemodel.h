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

#ifndef SKILL_TABLE_MODEL_H
#define SKILL_TABLE_MODEL_H

#include "../../bindings/guichan/models/statictablemodel.h"

class SkillDialog;

class SkillTableModel : public StaticTableModel
{
    public:
        SkillTableModel(SkillDialog *dialog);

        virtual int getRows(void) { return mEntriesNr; }

        virtual int getColumnWidth(int index) { return (index == 0 ? 160 : 35); }

        virtual int getRowHeight(void) { return 12; }

        virtual void update(void);

    private:
        SkillDialog *mDialog;
        int mEntriesNr;

        static void initSkillinfo(void);
};

#endif /* !defined(SKILL_TABLE_MODEL_H) */
