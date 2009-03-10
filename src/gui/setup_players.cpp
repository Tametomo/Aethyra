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

#include <string>
#include <vector>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "checkbox.h"
#include "listbox.h"
#include "ok_dialog.h"
#include "scrollarea.h"
#include "setup_players.h"
#include "table.h"

#include "widgets/dropdown.h"
#include "widgets/layouthelper.h"

#include "../configuration.h"
#include "../log.h"

#include "../utils/gettext.h"

#define COLUMNS_NR 2 // name plus listbox
#define NAME_COLUMN 0
#define RELATION_CHOICE_COLUMN 1

#define ROW_HEIGHT 12
// The following column widths really shouldn't be hardcoded but should scale with the size of the widget... except
// that, right now, the widget doesn't exactly scale either.
#define NAME_COLUMN_WIDTH 230
#define RELATION_CHOICE_COLUMN_WIDTH 80

#define WIDGET_AT(row, column) (((row) * COLUMNS_NR) + column)

static const char *table_titles[COLUMNS_NR] =
{
    N_("Name"),
    N_("Relation")
};

static const char *RELATION_NAMES[PlayerRelation::RELATIONS_NR] =
{
    N_("Neutral"),
    N_("Friend"),
    N_("Disregarded"),
    N_("Ignored")
};

class PlayerRelationListModel : public gcn::ListModel
{
public:
    virtual ~PlayerRelationListModel(void) { }

    virtual int getNumberOfElements(void)
    {
        return PlayerRelation::RELATIONS_NR;
    }

    virtual std::string getElementAt(int i)
    {
        if (i >= getNumberOfElements() || i < 0)
            return "";
        return gettext(RELATION_NAMES[i]);
    }
};

class PlayerTableModel : public TableModel
{
public:
    PlayerTableModel(void) :
        mPlayers(NULL)
    {
        playerRelationsUpdated();
    }

    virtual ~PlayerTableModel(void)
    {
        freeWidgets();
        if (mPlayers)
            delete mPlayers;
    }

    virtual int getRows(void)
    {
        return mPlayers->size();
    }

    virtual int getColumns(void)
    {
        return COLUMNS_NR;
    }

    virtual int getRowHeight(void)
    {
        return ROW_HEIGHT;
    }

    virtual int getColumnWidth(int index)
    {
        if (index == NAME_COLUMN)
            return NAME_COLUMN_WIDTH;
        else
            return RELATION_CHOICE_COLUMN_WIDTH;
    }

    virtual void playerRelationsUpdated(void)
    {
        signalBeforeUpdate();

        freeWidgets();
        std::vector<std::string> *player_names = player_relations.getPlayers();
        if (mPlayers)
            delete mPlayers;
        mPlayers = player_names;

        // set up widgets
        for (unsigned int r = 0; r < player_names->size(); ++r)
        {
            std::string name = (*player_names)[r];
            gcn::Widget *widget = new gcn::Label(name);
            mWidgets.push_back(widget);
            gcn::ListModel *playerRelation = new PlayerRelationListModel();

            gcn::DropDown *choicebox = new DropDown(playerRelation,
                                                    new ScrollArea(),
                                                    new ListBox(playerRelation),
                                                    false);
            choicebox->setSelected(player_relations.getRelation(name));
            mWidgets.push_back(choicebox);
        }

        signalAfterUpdate();
    }

    virtual void updateModelInRow(int row)
    {
        gcn::DropDown *choicebox = dynamic_cast<gcn::DropDown *>(
                                   getElementAt(row, RELATION_CHOICE_COLUMN));
        player_relations.setRelation(getPlayerAt(row),
                                   static_cast<PlayerRelation::relation>(
                                   choicebox->getSelected()));
    }


    virtual gcn::Widget *getElementAt(int row, int column)
    {
        return mWidgets[WIDGET_AT(row, column)];
    }

    virtual void freeWidgets(void)
    {
        if (mPlayers)
            delete mPlayers;
        mPlayers = NULL;

        for (std::vector<gcn::Widget *>::const_iterator it = mWidgets.begin();
             it != mWidgets.end(); it++)
        {
            delete *it;
        }

        mWidgets.clear();
    }

    std::string getPlayerAt(int index)
    {
        return (*mPlayers)[index];
    }

protected:
    std::vector<std::string> *mPlayers;
    std::vector<gcn::Widget *> mWidgets;
};

/**
 * Class for choosing one of the various `what to do when ignoring a player' options
 */
class IgnoreChoicesListModel : public gcn::ListModel
{
public:
    virtual ~IgnoreChoicesListModel(void) { }

    virtual int getNumberOfElements(void)
    {
        return player_relations.getPlayerIgnoreStrategies()->size();
    }

    virtual std::string getElementAt(int i)
    {
        if (i >= getNumberOfElements())
            return _("???");

        return (*player_relations.getPlayerIgnoreStrategies())[i]->mDescription;
    }
};

#define ACTION_DELETE "delete"
#define ACTION_TABLE "table"
#define ACTION_STRATEGY "strategy"

Setup_Players::Setup_Players():
    mPlayerTableTitleModel(new StaticTableModel(1, COLUMNS_NR)),
    mPlayerTableModel(new PlayerTableModel()),
    mPlayerTable(new GuiTable(mPlayerTableModel)),
    mPlayerTitleTable(new GuiTable(mPlayerTableTitleModel)),
    mPlayerScrollArea(new ScrollArea(mPlayerTable)),
    mPersistIgnores(new CheckBox(_("Save player list"),
                player_relations.getPersistIgnores())),
    mDefaultTrading(new CheckBox(_("Allow trading"),
                player_relations.getDefault() & PlayerRelation::TRADE)),
    mDefaultWhisper(new CheckBox(_("Allow whispers"),
                player_relations.getDefault() & PlayerRelation::WHISPER)),
    mDeleteButton(new Button(_("Delete"), ACTION_DELETE, this))
{
    setOpaque(false);
    mPlayerTable->setOpaque(false);

    mPlayerTableTitleModel->fixColumnWidth(NAME_COLUMN, NAME_COLUMN_WIDTH);
    mPlayerTableTitleModel->fixColumnWidth(RELATION_CHOICE_COLUMN,
                                           RELATION_CHOICE_COLUMN_WIDTH);
    mPlayerTitleTable->setBackgroundColor(gcn::Color(0xbf, 0xbf, 0xbf));

    gcn::ListModel *ignoreChoices = new IgnoreChoicesListModel();
    mIgnoreActionChoicesBox = new DropDown(ignoreChoices, new ScrollArea(),
                                           new ListBox(ignoreChoices), false);

    for (int i = 0; i < COLUMNS_NR; i++)
    {
        mPlayerTableTitleModel->set(0, i,
                new gcn::Label(gettext(table_titles[i])));
    }

    mPlayerTitleTable->setLinewiseSelection(true);

    mPlayerScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mPlayerTable->setActionEventId(ACTION_TABLE);
    mPlayerTable->setLinewiseSelection(true);
    mPlayerTable->addActionListener(this);

    gcn::Label *ignore_action_label = new gcn::Label(_("When ignoring:"));

    mIgnoreActionChoicesBox->setActionEventId(ACTION_STRATEGY);
    mIgnoreActionChoicesBox->addActionListener(this);

    int ignore_strategy_index = 0; // safe default

    if (player_relations.getPlayerIgnoreStrategy())
    {
        ignore_strategy_index = player_relations.getPlayerIgnoreStrategyIndex(
            player_relations.getPlayerIgnoreStrategy()->mShortName);
        if (ignore_strategy_index < 0)
            ignore_strategy_index = 0;
    }
    mIgnoreActionChoicesBox->setSelected(ignore_strategy_index);
    mIgnoreActionChoicesBox->adjustHeight();

    reset();

    // Do the layout
    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, mPlayerTitleTable, 4);
    place(0, 1, mPlayerScrollArea, 4, 4).setPadding(2);
    place(0, 5, mDeleteButton);
    place(2, 5, ignore_action_label);
    place(2, 6, mIgnoreActionChoicesBox, 2).setPadding(2);
    place(2, 7, mPersistIgnores);
    place(2, 8, mDefaultTrading);
    place(2, 9, mDefaultWhisper);

    player_relations.addListener(this);

    setDimension(gcn::Rectangle(0, 0, 325, 280));
}

Setup_Players::~Setup_Players(void)
{
    player_relations.removeListener(this);
}


void Setup_Players::reset()
{
    // We now have to search through the list of ignore choices to find the
    // current selection. We could use an index into the table of config
    // options in player_relations instead of strategies to sidestep this.
    int selection = 0;
    for (unsigned int i = 0;
                      i < player_relations.getPlayerIgnoreStrategies()->size();
                      ++i)
        if ((*player_relations.getPlayerIgnoreStrategies())[i] ==
            player_relations.getPlayerIgnoreStrategy())
        {

            selection = i;
            break;
        }

    mIgnoreActionChoicesBox->setSelected(selection);
}

void Setup_Players::apply()
{
    player_relations.setPersistIgnores(mPersistIgnores->isSelected());
    player_relations.store();

    unsigned int old_default_relations = player_relations.getDefault() &
                                         ~(PlayerRelation::TRADE |
                                           PlayerRelation::WHISPER);
    player_relations.setDefault(old_default_relations
                                | (mDefaultTrading->isSelected() ?
                                       PlayerRelation::TRADE : 0)
                                | (mDefaultWhisper->isSelected() ?
                                       PlayerRelation::WHISPER : 0));
}

void Setup_Players::cancel()
{
}

void Setup_Players::action(const gcn::ActionEvent &event)
{
    if (event.getId() == ACTION_TABLE)
    {
        // temporarily eliminate ourselves: we are fully aware of this change,
        // so there is no need for asynchronous updates.  (In fact, thouse
        // might destroy the widet that triggered them, which would be rather
        // embarrassing.)
        player_relations.removeListener(this);

        int row = mPlayerTable->getSelectedRow();
        if (row >= 0)
            mPlayerTableModel->updateModelInRow(row);

        player_relations.addListener(this);

    }
    else if (event.getId() == ACTION_DELETE)
    {
        int player_index = mPlayerTable->getSelectedRow();

        if (player_index < 0)
            return;

        std::string name = mPlayerTableModel->getPlayerAt(player_index);

        player_relations.removePlayer(name);

    }
    else if (event.getId() == ACTION_STRATEGY)
    {
        PlayerIgnoreStrategy *s =
            (*player_relations.getPlayerIgnoreStrategies())[
                mIgnoreActionChoicesBox->getSelected()];

        player_relations.setPlayerIgnoreStrategy(s);
    }
}

void Setup_Players::updatedPlayer(const std::string &name)
{
    mPlayerTableModel->playerRelationsUpdated();
    mDefaultTrading->setSelected(
            player_relations.getDefault() & PlayerRelation::TRADE);
    mDefaultWhisper->setSelected(
            player_relations.getDefault() & PlayerRelation::WHISPER);
}
