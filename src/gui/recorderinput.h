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

#ifndef RECORDER_INPUT_H
#define RECORDER_INPUT_H

#include <guichan/actionlistener.hpp>

#include "../bindings/guichan/widgets/window.h"

/**
 * The npc integer input dialog.
 *
 * \ingroup Interface
 */
class RecorderInput : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        RecorderInput(std::string caption = "");

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Returns the current value.
         */
        std::string getValue();

        /**
         * Resets the textfield for input.
         */
        void reset();

        /**
         * Chnages the current value.
         *
         * @param value The new value
         */
        void setValue(const std::string &value);

        /**
         * Requests the textfield to take focus for input.
         */
        void requestFocus();

        /**
         * Overridden close() method which allows the text input dialog to clean
         * up after itself on close or lost visibility.
         */
        void close();

    private:
        gcn::TextField *mValueField;
        gcn::Label *mCaption;
        gcn::Button *mOkButton;
        gcn::Button *mCancelButton;
};

#endif
