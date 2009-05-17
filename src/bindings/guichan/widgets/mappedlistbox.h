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

#ifndef MAPPEDLISTBOX_H
#define MAPPEDLISTBOX_H

#include <list>

#include <guichan/keylistener.hpp>
#include <guichan/mouselistener.hpp>
#include <guichan/widget.hpp>

#include "../models/mappedlistmodel.h"

/**
 * An implementation of a mapped list box whose purpose is rather similar to the
 * GUIChan ListBox's purpose, but have completely different model types for
 * displaying their data. This is done to allow for the MappedListBox to have
 * more flexibility over how its data is displayed and handled, but it comes at
 * the cost of simplicity for model implementation.
 *
 * Use this list box version when you want direct control over how your list
 * will be displayed (including custom row heights), when you want to have
 * seperators within your list box, if you want to directly model action
 * events to list elements, or if you want to draw your list in a nonstandard
 * way. Otherwise, the GUIChan version is likely more appropriate.
 */
class MappedListBox :  public gcn::Widget, public gcn::MouseListener,
                       public gcn::KeyListener
{
    public:
        friend class gcn::SelectionListener;

        /**
         * Constructor.
         */
        MappedListBox();

        /**
         * Constructor.
         *
         * @param listModel the list model to use.
         */
        MappedListBox(MappedListModel *listModel);

        /**
         * Destructor.
         */
        virtual ~MappedListBox() { }

        /**
         * Sets the list model to use.
         *
         * @param listModel the list model to use.
         * @see getListModel
         */
        void setListModel(MappedListModel *listModel);

        /**
         * Gets the list model used.
         *
         * @return the list model used.
         * @see setListModel
         */
        MappedListModel *getListModel();

        /**
         * Adjusts the size of the list box to fit it's list model.
         */
        void adjustSize();

        /**
         * Checks whether the list box wraps when selecting items with a keyboard.
         *
         * Wrapping means that the selection of items will be wrapped. That is, if 
         * the first item is selected and up is pressed, the last item will get 
         * selected. If the last item is selected and down is pressed, the first item 
         * will get selected.
         *
         * @return true if wrapping is enabled, false otherwise.
         * @see setWrappingEnabled
         */
        bool isWrappingEnabled() const;

        /**
         * Sets the list box to wrap or not when selecting items with a keyboard.
         *
         * Wrapping means that the selection of items will be wrapped. That is, if 
         * the first item is selected and up is pressed, the last item will get 
         * selected. If the last item is selected and down is pressed, the first item 
         * will get selected.
         * 
         * @see isWrappingEnabled
         */
        void setWrappingEnabled(bool wrappingEnabled);

        /**
         * Checks whether the list changes selection based on mouse position
         *
         * @return true if the selection should follow the mouse, false otherwise.
         * @see setFollowingMouse
         */
        bool isFollowingMouse() const;

        /**
         * Sets the list box to change the selection based on mouse position
         * 
         * @see isFollowingMouse
         */
        void setFollowingMouse(bool followMouse);

        /**
         * Adds a selection listener to the list box. When the selection
         * changes an event will be sent to all selection listeners of the
         * list box.
         *
         * @param selectionListener The selection listener to add.
         */
        void addSelectionListener(gcn::SelectionListener* selectionListener);

        /**
         * Removes a selection listener from the list box.
         *
         * @param selectionListener The selection listener to remove.
         */
        void removeSelectionListener(gcn::SelectionListener* selectionListener);

        // Inherited from Widget

        virtual void draw(gcn::Graphics* graphics);

        virtual void logic();

        // Inherited from KeyListener

        virtual void keyPressed(gcn::KeyEvent& keyEvent);

        // Inherited from MouseListener

        virtual void mousePressed(gcn::MouseEvent& mouseEvent);

        virtual void mouseWheelMovedUp(gcn::MouseEvent& mouseEvent);

        virtual void mouseWheelMovedDown(gcn::MouseEvent& mouseEvent);
        
        virtual void mouseDragged(gcn::MouseEvent& mouseEvent);

        virtual void mouseMoved(gcn::MouseEvent& mouseEvent);

    protected:
        /**
         * Distributes a value changed event to all selection listeners
         * of the mapped list box.
         */
        void distributeValueChangedEvent();

        /**
         * The mapped list model to use.
         */
        MappedListModel *mListModel;

        /**
         * True if wrapping is enabled, false otherwise.
         */
        bool mWrappingEnabled;

        /**
         * True if the selection should follow the mouse, false otherwise.
         */
        bool mFollowingMouse;

        /**
         * Typdef.
         */ 
        typedef std::list<gcn::SelectionListener*> SelectionListenerList;
        
        /**
         * The selection listeners of the list box.
         */
        SelectionListenerList mSelectionListeners;

        /**
         * Typedef.
         */
        typedef SelectionListenerList::iterator SelectionListenerIterator;
};

#endif
