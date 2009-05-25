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

#include "skill.h"

#include "../log.h"

#include "../bindings/guichan/layout.h"

#include "../bindings/guichan/models/skilltablemodel.h"

#include "../bindings/guichan/widgets/button.h"
#include "../bindings/guichan/widgets/label.h"
#include "../bindings/guichan/widgets/listbox.h"
#include "../bindings/guichan/widgets/scrollarea.h"
#include "../bindings/guichan/widgets/table.h"
#include "../bindings/guichan/widgets/windowcontainer.h"

#include "../resources/db/skilldb.h"

#include "../resources/sprite/localplayer.h"

#include "../utils/dtor.h"
#include "../utils/gettext.h"
#include "../utils/stringutils.h"

SkillDialog::SkillDialog():
    Window(_("Skills"))
{
    mTableModel = new SkillTableModel(this);
    mTable = new Table(mTableModel);
    mTable->setOpaque(false);
    mTable->setLinewiseSelection(true);
    mTable->setWrappingEnabled(true);
    mTable->setActionEventId("skill");
    mTable->addActionListener(this);

    setWindowName("Skills");
    setCloseButton(true);
    setDefaultSize(255, 260, ImageRect::CENTER);

    setMinHeight(50 + mTableModel->getHeight());
    setMinWidth(200);

    ScrollArea *skillScrollArea = new ScrollArea(mTable);
    mPointsLabel = new Label(strprintf(_("Skill points: %d"), 0));
    mIncButton = new Button(_("Up"), _("inc"), this);
    mUseButton = new Button(_("Use"), _("use"), this);
    mUseButton->setEnabled(false);

    skillScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    place(0, 0, skillScrollArea, 5).setPadding(3);
    place(0, 1, mPointsLabel, 4);
    place(3, 2, mIncButton);
    place(4, 2, mUseButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    setLocationRelativeTo(getParent());
    loadWindowState();
}

SkillDialog::~SkillDialog()
{
    delete_all(mSkillList);
}

void SkillDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "inc")
    {
        // Increment skill
        int selectedSkill = mTable->getSelectedRow();
        if (selectedSkill >= 0)
            player_node->raiseSkill(mSkillList[selectedSkill]->id);
    }
    else if (event.getId() == "skill" && mTable->getSelectedRow() > -1)
    {
        SKILL *skill = mSkillList[mTable->getSelectedRow()];
        SkillInfo const *info = SkillDB::get(skill->id);

        mIncButton->setEnabled(player_node->mSkillPoint > 0 &&
                               info->modifiable);
    }
    else if (event.getId() == "close")
        close();
}

void SkillDialog::update()
{
    mPointsLabel->setCaption(strprintf(_("Skill points: %d"),
                                       player_node->mSkillPoint));

    int selectedSkill = mTable->getSelectedRow();

    if (selectedSkill >= 0)
    {
        int skillId = mSkillList[selectedSkill]->id;
        bool modifiable = SkillDB::modifiable(skillId);

        mIncButton->setEnabled(modifiable && player_node->mSkillPoint > 0);
    }
    else
        mIncButton->setEnabled(false);

    mTableModel->update();
    setMinHeight(50 + mTableModel->getHeight());
}

int SkillDialog::getNumberOfElements()
{
    return mSkillList.size();
}

bool SkillDialog::hasSkill(int id)
{
    for (unsigned int i = 0; i < mSkillList.size(); i++)
    {
        if (mSkillList[i]->id == id)
            return true;
    }
    return false;
}

void SkillDialog::addSkill(int id, int lvl, int mp)
{
    SKILL *tmp = new SKILL();
    tmp->id = id;
    tmp->lv = lvl;
    tmp->sp = mp;
    mSkillList.push_back(tmp);
}

void SkillDialog::setSkill(int id, int lvl, int mp)
{
    for (unsigned int i = 0; i < mSkillList.size(); i++)
    {
        if (mSkillList[i]->id == id)
        {
            mSkillList[i]->lv = lvl;
            mSkillList[i]->sp = mp;
        }
    }
}

void SkillDialog::cleanList()
{
    for_each(mSkillList.begin(), mSkillList.end(), make_dtor(mSkillList));
    mSkillList.clear();
}

void SkillDialog::requestFocus()
{
    mTable->requestFocus();
    mTable->setSelectedRow(0);
}

