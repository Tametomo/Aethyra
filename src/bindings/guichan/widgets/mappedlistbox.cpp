/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <guichan/focushandler.hpp>
#include <guichan/graphics.hpp>
#include <guichan/key.hpp>
#include <guichan/mouseinput.hpp>
#include <guichan/selectionlistener.hpp>

#include "mappedlistbox.h"

#include "../protectedfocuslistener.h"

#include "../models/mappedlistmodel.h"

#include "../sdl/sdlinput.h"

#include "../../../core/utils/dtor.h"

MappedListBox::MappedListBox(MappedListModel *listModel) :
    mWrappingEnabled(false),
    mFollowingMouse(false)
{
    setWidth(100);
    setListModel(listModel);
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

MappedListBox::~MappedListBox()
{
    if (mFocusHandler && mFocusHandler->isFocused(this))
        mFocusHandler->focusNone();

    removeFocusListener(mProtFocusListener);
    destroy(mProtFocusListener);
}

void MappedListBox::draw(gcn::Graphics* graphics)
{
    if (mListModel)
        mListModel->draw(graphics);
}

void MappedListBox::logic()
{
}

void MappedListBox::keyPressed(gcn::KeyEvent& keyEvent)
{
    if (!mListModel)
        return;

    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ENTER || key.getValue() == Key::SPACE)
    {
        setActionEventId(mListModel->getSelected());

        if (getActionEventId() != "")
            distributeActionEvent();

        keyEvent.consume();
    }
    else if (key.getValue() == Key::UP)
    {
        mListModel->selectPrevious();

        if (mWrappingEnabled && mListModel->getSelected() == "")
            mListModel->selectEnd();

        distributeValueChangedEvent();
        keyEvent.consume();
    }
    else if (key.getValue() == Key::DOWN)
    {
        mListModel->selectNext();

        if (mWrappingEnabled && mListModel->getSelected() == "")
            mListModel->selectFirst();

        distributeValueChangedEvent();
        keyEvent.consume();
    }
    else if (key.getValue() == Key::HOME)
    {
        mListModel->selectFirst();
        distributeValueChangedEvent();
        keyEvent.consume();
    }
    else if (key.getValue() == Key::END)
    {
        mListModel->selectEnd();
        distributeValueChangedEvent();
        keyEvent.consume();
    }
}

void MappedListBox::mousePressed(gcn::MouseEvent& mouseEvent)
{
    if (!mListModel)
        return;

    if (mouseEvent.getButton() == gcn::MouseEvent::LEFT)
    {
        mListModel->setSelectedByY(mouseEvent.getY());
        setActionEventId(mListModel->getSelected());

        if (getActionEventId() != "")
            distributeActionEvent();
    }
}

void MappedListBox::mouseWheelMovedUp(gcn::MouseEvent& mouseEvent)
{
    if (isFocused() && mListModel)
    {
        mListModel->selectPrevious();

        if (mWrappingEnabled && mListModel->getSelected() == "")
            mListModel->selectEnd();

        distributeValueChangedEvent();
        mouseEvent.consume();
    }
}

void MappedListBox::mouseWheelMovedDown(gcn::MouseEvent& mouseEvent)
{
    if (isFocused() && mListModel)
    {
        mListModel->selectNext();

        if (mWrappingEnabled && mListModel->getSelected() == "")
            mListModel->selectFirst();

        distributeValueChangedEvent();
        mouseEvent.consume();
    }
}

void MappedListBox::mouseDragged(gcn::MouseEvent& mouseEvent)
{
    if (mouseEvent.getButton() != gcn::MouseEvent::LEFT || !mListModel)
        return;

    // Make list selection update on drag, but guard against negative y
    const int y = std::max(0, mouseEvent.getY());
    mListModel->setSelectedByY(y);
    distributeValueChangedEvent();
}

void MappedListBox::mouseMoved(gcn::MouseEvent& mouseEvent)
{
    const int y = std::max(0, mouseEvent.getY());

    if (mFollowingMouse)
    {
        mListModel->setSelectedByY(y);
        distributeValueChangedEvent();
    }
}

void MappedListBox::setListModel(MappedListModel *listModel)
{
    mListModel = listModel;
}

MappedListModel* MappedListBox::getListModel()
{
    return mListModel;
}

void MappedListBox::adjustSize()
{
    if (mListModel)
    {
        setWidth(mListModel->getWidth());
        setHeight(mListModel->getHeight());
    }
}

bool MappedListBox::isWrappingEnabled() const
{
    return mWrappingEnabled;
}

void MappedListBox::setWrappingEnabled(bool wrappingEnabled)
{
    mWrappingEnabled = wrappingEnabled;
}

bool MappedListBox::isFollowingMouse() const
{
    return mFollowingMouse;
}

void MappedListBox::setFollowingMouse(bool followingMouse)
{
    mFollowingMouse = followingMouse;
}

void MappedListBox::addSelectionListener(gcn::SelectionListener *selectionListener)
{
    mSelectionListeners.push_back(selectionListener);
}

void MappedListBox::removeSelectionListener(gcn::SelectionListener *selectionListener)
{
    mSelectionListeners.remove(selectionListener);
}

void MappedListBox::distributeValueChangedEvent()
{
    SelectionListenerIterator iter;

    for (iter = mSelectionListeners.begin(); iter != mSelectionListeners.end(); ++iter)
    {
        gcn::SelectionEvent event(this);
        (*iter)->valueChanged(event);
    }
}

