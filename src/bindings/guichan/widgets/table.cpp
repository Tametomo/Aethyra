/*
 *  Aethyra
 *  Copyright (C) 2008  The Mana World Development Team
 *
 *  This file is part of Aethyra derived from original code
 *  from Guichan.
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

#include <guichan/actionlistener.hpp>
#include <guichan/focushandler.hpp>
#include <guichan/graphics.hpp>
#include <guichan/key.hpp>

#include "table.h"

#include "../palette.h"
#include "../protectedfocuslistener.h"
#include "../skin.h"

#include "../sdl/sdlinput.h"

#include "../../../utils/dtor.h"

float Table::mAlpha = 1.0;

class TableActionListener : public gcn::ActionListener
{
    public:
        TableActionListener(Table *_table, gcn::Widget *_widget, int _row, int _column);

        virtual ~TableActionListener(void);

        virtual void action(const gcn::ActionEvent& actionEvent);

    protected:
        Table *mTable;
        int mRow;
        int mColumn;
        gcn::Widget *mWidget;
};

TableActionListener::TableActionListener(Table *table, gcn::Widget *widget, int row, int column) :
    mTable(table),
    mRow(row),
    mColumn(column),
    mWidget(widget)
{
    if (widget)
    {
        widget->addActionListener(this);
        widget->_setParent(table);
    }
}

TableActionListener::~TableActionListener(void)
{
    if (mWidget)
    {
        mWidget->removeActionListener(this);
        mWidget->_setParent(NULL);
    }
}

void TableActionListener::action(const gcn::ActionEvent& actionEvent)
{
    mTable->setSelected(mRow, mColumn);
    mTable->distributeActionEvent();
}


Table::Table(TableModel *initial_model, gcn::Color background,
                   bool opacity) :
    mLinewiseMode(false),
    mWrappingEnabled(false),
    mOpaque(opacity),
    mBackgroundColor(background),
    mModel(NULL),
    mSelectedRow(0),
    mSelectedColumn(0),
    mTopWidget(NULL)
{
    setModel(initial_model);
    setFocusable(true);

    addMouseListener(this);
    addKeyListener(this);

    mProtFocusListener = new ProtectedFocusListener();

    addFocusListener(mProtFocusListener);

    mProtFocusListener->blockKey(SDLK_LEFT);
    mProtFocusListener->blockKey(SDLK_RIGHT);
    mProtFocusListener->blockKey(SDLK_UP);
    mProtFocusListener->blockKey(SDLK_DOWN);
    mProtFocusListener->blockKey(SDLK_SPACE);
    mProtFocusListener->blockKey(SDLK_RETURN);
    mProtFocusListener->blockKey(SDLK_HOME);
    mProtFocusListener->blockKey(SDLK_END);
}

Table::~Table(void)
{
    uninstallActionListeners();
    delete mModel;

    if (mFocusHandler && mFocusHandler->isFocused(this))
        mFocusHandler->focusNone();

    removeFocusListener(mProtFocusListener);
    delete mProtFocusListener;
}

TableModel* Table::getModel(void) const
{
    return mModel;
}

void Table::setModel(TableModel *new_model)
{
    if (mModel)
    {
        uninstallActionListeners();
        mModel->removeListener(this);
    }

    mModel = new_model;
    installActionListeners();

    if (new_model)
    {
        new_model->installListener(this);
        recomputeDimensions();
    }
}

void Table::recomputeDimensions(void)
{
    int rows_nr = mModel->getRows();
    int columns_nr = mModel->getColumns();
    int width = 0;
    int height = 0;

    if (mSelectedRow >= rows_nr)
        mSelectedRow = rows_nr - 1;

    if (mSelectedColumn >= columns_nr)
        mSelectedColumn = columns_nr - 1;

    for (int i = 0; i < columns_nr; i++)
        width += getColumnWidth(i);

    height = getRowHeight() * rows_nr;

    setWidth(width);
    setHeight(height);
}

void Table::setSelected(int row, int column)
{
    mSelectedColumn = column;
    mSelectedRow = row;
}

int Table::getSelectedRow(void)
{
    return mSelectedRow;
}

int Table::getSelectedColumn(void)
{
    return mSelectedColumn;
}

void Table::setLinewiseSelection(bool linewise)
{
    mLinewiseMode = linewise;
}

int Table::getRowHeight(void)
{
    if (mModel)
        return mModel->getRowHeight() + 1; // border
    else
        return 0;
}

int Table::getColumnWidth(int i)
{
    if (mModel)
        return mModel->getColumnWidth(i) + 1; // border
    else
        return 0;
}

void Table::setSelectedRow(int selected)
{
    if (!mModel)
        mSelectedRow = -1;
    else  if (selected < 0 && !mWrappingEnabled)
        mSelectedRow = -1;
    else if (selected >= mModel->getRows() && mWrappingEnabled)
        mSelectedRow = 0;
    else if ((selected >= mModel->getRows() && !mWrappingEnabled) || 
             (selected < 0 && mWrappingEnabled))
        mSelectedRow = mModel->getRows() - 1;
    else
        mSelectedRow = selected;
}

void Table::setSelectedColumn(int selected)
{
    if (!mModel)
        mSelectedColumn = -1;
    else if ((selected >= mModel->getColumns() && mWrappingEnabled) ||
             (selected < 0 && !mWrappingEnabled))
        mSelectedColumn = 0;
    else if ((selected >= mModel->getColumns() && !mWrappingEnabled) || 
             (selected < 0 && mWrappingEnabled))
        mSelectedColumn = mModel->getColumns() - 1;
    else
        mSelectedColumn = selected;
}

void Table::uninstallActionListeners(void)
{
    delete_all(action_listeners);
    action_listeners.clear();
}

void Table::installActionListeners(void)
{
    if (!mModel)
        return;

    int rows = mModel->getRows();
    int columns = mModel->getColumns();

    for (int row = 0; row < rows; ++row)
    {
        for (int column = 0; column < columns; ++column)
        {
            gcn::Widget *widget = mModel->getElementAt(row, column);
            action_listeners.push_back(new TableActionListener(this, widget,
                                                                  row, column));
        }
    }

    _setFocusHandler(_getFocusHandler()); // propagate focus handler to widgets
}

// -- widget ops
void Table::draw(gcn::Graphics* graphics)
{
    if (!mModel)
        return;

    if (Skin::getAlpha() != mAlpha)
        mAlpha = Skin::getAlpha();

    if (mOpaque)
    {
        graphics->setColor(guiPalette->getColor(Palette::BACKGROUND,
                (int)(mAlpha * 255.0f)));
        graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
    }

    // First, determine how many rows we need to draw, and where we should start.
    int first_row = -(getY() / getRowHeight());

    if (first_row < 0)
        first_row = 0;

    int rows_nr = 1 + (getHeight() / getRowHeight()); // May overestimate by one.

    int max_rows_nr = mModel->getRows() - first_row; // clip if neccessary:
    if (max_rows_nr < rows_nr)
        rows_nr = max_rows_nr;
 
    // Now determine the first and last column
    // Take the easy way out; these are usually bounded and all visible.
    int first_column = 0;
    int last_column = mModel->getColumns() - 1;

    // Set up everything for drawing
    int height = getRowHeight();
    int y_offset = first_row * height;

    for (int r = first_row; r < first_row + rows_nr; ++r)
    {
        int x_offset = 0;

        for (int c = first_column; c <= last_column; ++c)
        {
            gcn::Widget *widget = mModel->getElementAt(r, c);
            int width = getColumnWidth(c);
            if (widget)
            {
                gcn::Rectangle bounds(x_offset, y_offset, width, height);

                if (widget == mTopWidget)
                {
                    bounds.height = widget->getHeight();
                    bounds.width = widget->getWidth();
                }

                widget->setDimension(bounds);

                graphics->setColor(guiPalette->getColor(Palette::HIGHLIGHT,
                                                       (int)(mAlpha * 255.0f)));

                if (mLinewiseMode && r == mSelectedRow && c == 0)
                {
                    graphics->fillRectangle(gcn::Rectangle(0, y_offset,
                                                           getWidth(), height));
                }
                else if (!mLinewiseMode &&
                          c == mSelectedColumn && r == mSelectedRow)
                {
                    graphics->fillRectangle(gcn::Rectangle(x_offset, y_offset,
                                                           width, height));
                }

                graphics->pushClipArea(bounds);
                widget->draw(graphics);
                graphics->popClipArea();

            }

            x_offset += width;
        }

        y_offset += height;
    }

    if (isFocused())
    {
        graphics->setColor(guiPalette->getColor(Palette::HIGHLIGHT,
                                               (int)(mAlpha * 255.0f)));
        graphics->drawLine(0, 0, getWidth() - 1, 0);
        graphics->drawLine(0, 1, 0, getHeight() - 1);
        graphics->drawLine(getWidth() - 1, 1, getWidth() - 1, getHeight() - 1);
        graphics->drawLine(0, getHeight() - 1, getWidth() - 2, getHeight() - 1);
    }

    if (mTopWidget)
    {
        gcn::Rectangle bounds = mTopWidget->getDimension();
        graphics->pushClipArea(bounds);
        mTopWidget->draw(graphics);
        graphics->popClipArea();
    }
}

void Table::moveToTop(gcn::Widget *widget)
{
    gcn::Widget::moveToTop(widget);
    mTopWidget = widget;
}

void Table::moveToBottom(gcn::Widget *widget)
{
    gcn::Widget::moveToBottom(widget);
    if (widget == mTopWidget)
        mTopWidget = NULL;
}

gcn::Rectangle Table::getChildrenArea(void)
{
    return gcn::Rectangle(0, 0, getWidth(), getHeight());
}

// -- KeyListener notifications
void Table::keyPressed(gcn::KeyEvent& keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ENTER || key.getValue() == Key::SPACE)
        distributeActionEvent();
    else if (key.getValue() == Key::UP)
        setSelectedRow(mSelectedRow - 1);
    else if (key.getValue() == Key::DOWN)
        setSelectedRow(mSelectedRow + 1);
    else if (key.getValue() == Key::LEFT)
        setSelectedColumn(mSelectedColumn - 1);
    else if (key.getValue() == Key::RIGHT)
        setSelectedColumn(mSelectedColumn + 1);
    else if (key.getValue() == Key::HOME)
    {
        setSelectedRow(0);
        setSelectedColumn(0);
    }
    else if (key.getValue() == Key::END)
    {
        setSelectedRow(mModel->getRows() - 1);
        setSelectedColumn(mModel->getColumns() - 1);
    }
    else
        return;

    keyEvent.consume();
}

// -- MouseListener notifications
void Table::mousePressed(gcn::MouseEvent& mouseEvent)
{
    if (mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
        int row = getRowForY(mouseEvent.getY());
        int column = getColumnForX(mouseEvent.getX());

        if (row > -1 && column > -1 &&
            row < mModel->getRows() && column < mModel->getColumns())
        {
            mSelectedColumn = column;
            mSelectedRow = row;
        }

        distributeActionEvent();
    }
}

void Table::mouseWheelMovedUp(gcn::MouseEvent& mouseEvent)
{
    if (isFocused())
    {
        if (getSelectedRow() > 0 || (getSelectedRow() == 0 && mWrappingEnabled))
            setSelectedRow(getSelectedRow() - 1);

        mouseEvent.consume();
    }
}

void Table::mouseWheelMovedDown(gcn::MouseEvent& mouseEvent)
{
    if (isFocused())
    {
        setSelectedRow(getSelectedRow() + 1);
        mouseEvent.consume();
    }
}

void Table::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() != gcn::MouseEvent::LEFT)
        return;

    // Make table selection update on drag
    const int x = std::max(0, event.getX());
    const int y = std::max(0, event.getY());

    setSelectedRow(getRowForY(y));
    setSelectedColumn(getColumnForX(x));
}

// -- TableModelListener notifications
void Table::modelUpdated(bool completed)
{
    if (completed)
    {
        recomputeDimensions();
        installActionListeners();
    }
    else
    { // before the update?
        mTopWidget = NULL; // No longer valid in general
        uninstallActionListeners();
    }
}

gcn::Widget* Table::getWidgetAt(int x, int y)
{
    int row = getRowForY(y);
    int column = getColumnForX(x);

    if (mTopWidget && mTopWidget->getDimension().isPointInRect(x, y))
        return mTopWidget;

    if (row > -1 && column > -1)
    {
        gcn::Widget *w = mModel->getElementAt(row, column);
        if (w && w->isFocusable())
            return w;
        else
            return NULL; // Grab the event locally
    }
    else
        return NULL;
}

int Table::getRowForY(int y)
{
   int row = -1;

   if (getRowHeight() > 0)
       row = y / getRowHeight();

   if (row < 0 || row >= mModel->getRows())
       return -1;
   else
       return row;
}

int Table::getColumnForX(int x)
{
    int column;
    int delta = 0;

    for (column = 0; column < mModel->getColumns(); column++)
    {
        delta += getColumnWidth(column);
        if (x <= delta)
            break;
    }

    return (column < 0 || column >= mModel->getColumns()) ? -1 : column;
}

void Table::_setFocusHandler(gcn::FocusHandler* focusHandler)
{
    gcn::Widget::_setFocusHandler(focusHandler);

    if (mModel)
    {
        for (int r = 0; r < mModel->getRows(); ++r)
        {
            for (int c = 0; c < mModel->getColumns(); ++c)
            {
                gcn::Widget *w = mModel->getElementAt(r, c);
                if (w)
                    w->_setFocusHandler(focusHandler);
            }
        }
    }
}
